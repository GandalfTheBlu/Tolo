#pragma once
#include "common.h"
#include "lex_node.h"
#include "expression.h"
#include <map>

namespace Tolo
{
	struct VariableInfo
	{
		std::string typeName;
		Int offset;

		VariableInfo();

		VariableInfo(const std::string& _typeName, Int _offset);
	};

	struct FunctionInfo
	{
		std::string returnTypeName;
		Int localsSize;
		Int parametersSize;

		std::map<std::string, VariableInfo> varNameToVarInfo;
		std::vector<std::string> parameterNames;

		FunctionInfo();
	};

	struct Parser
	{
		typedef std::vector<OpCode> DataTypeOperators;

		std::map<std::string, Int> typeNameToSize;
		std::map<std::string, FunctionInfo> definedFunctions;
		FunctionInfo* currentFunction;
		std::string currentExpectedReturnType;

		std::map<std::string, DataTypeOperators> typeNameOperators;

		Parser();

		Expression* ParseLiteralConstant(LexNode* p_lexNode);

		Expression* ParseVariableLoad(LexNode* p_lexNode);

		Expression* ParseVariableWrite(LexNode* p_lexNode);

		Expression* ParseReturn(LexNode* p_lexNode);

		Expression* ParseIf(LexNode* p_lexNode);

		Expression* ParseWhile(LexNode* p_lexNode);

		Expression* ParseBinaryMathOp(LexNode* p_lexNode, const std::string& funcName);

		Expression* ParseBinaryCompareOp(LexNode* p_lexNode, const std::string& funcName);

		Expression* ParseDebugPrint(LexNode* p_lexNode);

		Expression* ParseCoreFunctionCall(LexNode* p_lexNode);

		Expression* ParseUserFunctionCall(LexNode* p_lexNode);

		Expression* ParseVariableDefinition(LexNode* p_lexNode);

		Expression* ParseFunctionDefinition(LexNode* p_lexNode);

		Expression* ParseNextExpression(LexNode* p_lexNode);

		void Parse(std::vector<LexNode*>& lexNodes, std::vector<Expression*>& expressions);
	};
}