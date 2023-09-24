#include "compiler.h"
#include "tokenizer.h"
#include "lexer.h"
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

	void CompileProgram(const std::string& codePath, Char* p_stack, const std::map<std::string, NativeFunctionInfo>& nativeFunctions, Ptr& outCodeStart, Ptr& outCodeEnd, Int& outMainRetValByteSize)
	{
		std::string code;
		ReadTextFile(codePath, code);

		std::vector<Token> tokens;
		Tokenize(code, tokens);

		Lexer lexer;
		for (auto& e : nativeFunctions)
			lexer.nativeFunctions.insert(e.first);

		std::vector<LexNode*> lexNodes;
		lexer.Lex(tokens, lexNodes);

		Parser parser;
		parser.nativeFunctions = nativeFunctions;
		std::vector<Expression*> expressions;
		parser.Parse(lexNodes, expressions);

		CodeBuilder cb(p_stack);

		Affirm(
			parser.userFunctions.count("main") != 0,
			"no 'main' function found"
		);

		FunctionInfo& mainInfo = parser.userFunctions["main"];
		Int mainParamsSize = 0;

		for (size_t i=0; i<mainInfo.parameterNames.size(); i++)
		{
			const std::string& paramTypeName = mainInfo.varNameToVarInfo[mainInfo.parameterNames[i]].typeName;
			mainParamsSize += parser.typeNameToSize[paramTypeName];
		}

		cb.codeLength += mainParamsSize;// allocate space for main arguments in bottom of stack

		outCodeStart = cb.codeLength;
		outMainRetValByteSize = parser.typeNameToSize[mainInfo.returnTypeName];

		ECallFunction mainCall(mainParamsSize, mainInfo.localsSize, mainInfo.returnTypeName);
		// tell the main function to load arguments from the beginning of the stack, where the user will write them
		mainCall.argumentLoads = {new ELoadConstBytes(mainParamsSize, 0)};
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

		outCodeEnd = cb.codeLength;
	}
}