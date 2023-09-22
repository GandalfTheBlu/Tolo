#include "compiler.h"
#include "tokenizer.h"
#include "lexer.h"
#include "parser.h"
#include <fstream>
#include <sstream>

namespace Tolo
{
	void ReadTextFile(const std::string& filePath, std::string& outText)
	{
		std::ifstream file;
		file.open(filePath);

		Affirm(file.is_open(), "failed to open file '%s'", filePath.c_str());

		std::stringstream stringStream;
		stringStream << file.rdbuf();
		outText = stringStream.str();
		file.close();
	}

	void CompileProgram(const std::string& codePath, Char* p_stack, Ptr& outCodeLength, const std::string& expectedMainReturnType, const std::vector<Expression*>& mainArgumentLoaders)
	{
		std::string code;
		ReadTextFile(codePath, code);

		std::vector<Token> tokens;
		Tokenize(code, tokens);

		Lexer lexer;
		std::vector<LexNode*> lexNodes;
		lexer.Lex(tokens, lexNodes);

		Parser parser;
		std::vector<Expression*> expressions;
		parser.Parse(lexNodes, expressions);

		CodeBuilder cb(p_stack);

		Affirm(
			parser.definedFunctions.count("main") != 0,
			"no 'main' function found"
		);

		FunctionInfo& mainInfo = parser.definedFunctions["main"];
		Affirm(
			expectedMainReturnType == mainInfo.returnTypeName,
			"expected 'main'-function with return type of '%s' but got '%s'",
			mainInfo.returnTypeName.c_str(), expectedMainReturnType.c_str()
		);
		Affirm(
			mainInfo.parameterNames.size() == mainArgumentLoaders.size(),
			"'main'-function parameters do not match arguments"
		);

		Int mainParamsSize = 0;
		for (size_t i=0; i<mainInfo.parameterNames.size(); i++)
		{
			std::string argTypeName = mainArgumentLoaders[i]->GetDataType();
			const std::string& paramTypeName = mainInfo.varNameToVarInfo[mainInfo.parameterNames[i]].typeName;

			Affirm(
				argTypeName == paramTypeName, 
				"'main'-function parameter '%s' of type '%s' does not match argument of type '%s'",
				mainInfo.parameterNames[i].c_str(), paramTypeName.c_str(), argTypeName.c_str()
			);

			mainParamsSize += parser.typeNameToSize[paramTypeName];
		}

		ECallFunction mainCall(mainParamsSize, mainInfo.localsSize, mainInfo.returnTypeName);
		mainCall.argumentLoads = mainArgumentLoaders;
		mainCall.functionIpLoad = new ELoadConstPtrToLabel("main");
		mainCall.Evaluate(cb);
		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel("__program_end__");
		cb.Op(OpCode::Write_IP);

		for (auto e : expressions)
			e->Evaluate(cb);

		cb.DefineLabel("__program_end__");
		cb.RemoveLabel("__program_end__");

		for (auto e : lexNodes)
			delete e;

		for (auto e : expressions)
			delete e;

		outCodeLength = cb.codeLength;
	}
}