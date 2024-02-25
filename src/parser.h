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

		VariableInfo(const VariableInfo& rhs);

		VariableInfo& operator=(const VariableInfo& rhs);
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

	struct NativeFunctionInfo
	{
		std::string returnTypeName;
		Ptr p_functionPtr;
		std::vector<std::string> parameterTypeNames;

		NativeFunctionInfo();
	};

	struct StructInfo
	{
		std::map<std::string, VariableInfo> propNameToVarInfo;
		std::vector<std::string> propNames;

		StructInfo();
	};


	struct Parser
	{
		typedef std::vector<OpCode> DataTypeOperators;
		typedef std::map<std::string, FunctionInfo> DataTypeOperatorFunctions;
		typedef std::map<std::string, NativeFunctionInfo> DataTypeNativeOpFuncs;

		std::map<std::string, Int> typeNameToSize;
		std::map<std::string, FunctionInfo> userFunctions;
		std::map<std::string, NativeFunctionInfo> nativeFunctions;
		FunctionInfo* currentFunction;
		std::string currentExpectedReturnType;
		std::map<std::string, DataTypeOperators> typeNameOperators;
		std::map<std::string, DataTypeOperatorFunctions> typeNameToOpFuncs;
		std::map<std::string, DataTypeNativeOpFuncs> typeNameToNativeOpFuncs;
		std::map<std::string, StructInfo> typeNameToStructInfo;

		Parser();

		bool HasBody(LexNode* p_lexNode, Int& outBodyStartIndex, Int& outBodyEndIndex);

		void FlattenNode(LexNode* p_lexNode, std::vector<LexNode*>& outNodes);

		Expression* ParseLiteralConstant(LexNode* p_lexNode);

		Expression* ParseVariableLoad(LexNode* p_lexNode);

		Expression* ParseVariableWrite(LexNode* p_lexNode);

		Expression* ParsePropertyLoad(LexNode* p_lexNode);

		Expression* ParsePropertyWrite(LexNode* p_lexNode);

		Expression* ParseReturn(LexNode* p_lexNode);

		Expression* ParseIfSingle(LexNode* p_lexNode);

		Expression* ParseIfChain(LexNode* p_lexNode);

		Expression* ParseElseIfSingle(LexNode* p_lexNode);

		Expression* ParseElseIfChain(LexNode* p_lexNode);

		Expression* ParseElse(LexNode* p_lexNode);

		Expression* ParseWhile(LexNode* p_lexNode);

		Expression* ParseBinaryMathOp(LexNode* p_lexNode);

		Expression* ParseBinaryCompareOp(LexNode* p_lexNode);

		Expression* ParseBinaryOp(LexNode* p_lexNode);

		Expression* ParseUnaryNegate(LexNode* p_lexNode);

		Expression* ParseUnaryNot(LexNode* p_lexNode);

		Expression* ParseUnaryOp(LexNode* p_lexNode);

		Expression* ParseStructInitialization(LexNode* p_lexNode);

		Expression* ParseUserFunctionCall(LexNode* p_lexNode);

		Expression* ParseNativeFunctionCall(LexNode* p_lexNode);

		Expression* ParseVariableDefinition(LexNode* p_lexNode);

		Expression* ParseFunctionDefinition(LexNode* p_lexNode);

		Expression* ParseOperatorDefinition(LexNode* p_lexNode);

		Expression* ParseStructDefinition(LexNode* p_lexNode);

		Expression* ParseNextExpression(LexNode* p_lexNode);

		void Parse(std::vector<LexNode*>& lexNodes, std::vector<Expression*>& expressions);
	};
}