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

	void Compile(const std::string& codePath, Char* p_programMemory, Ptr& outCodeLength)
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

		CodeBuilder cb(p_programMemory);

		Affirm(
			parser.definedFunctions.count("main") != 0,
			"no 'main' function found"
		);

		FunctionInfo& mainInfo = parser.definedFunctions["main"];

		Affirm(
			mainInfo.parametersSize == 0,
			"'main' function cannot take any parameters"
		);

		ECallFunction mainCall(0, mainInfo.localsSize, mainInfo.returnTypeName);
		mainCall.functionIpLoad = new ELoadConstPtrToLabel("main");
		mainCall.Evaluate(cb);
		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel("__program_end__");
		cb.Op(OpCode::Write_IP);

		for (auto e : expressions)
			e->Evaluate(cb);

		cb.DefineLabel("__program_end__");
		cb.RemoveLabel("__program__end__");

		for (auto e : lexNodes)
			delete e;

		for (auto e : expressions)
			delete e;

		outCodeLength = cb.codeLength;
	}
}