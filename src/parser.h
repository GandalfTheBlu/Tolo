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
		std::map<std::string, VariableInfo> memberNameToVarInfo;
		std::vector<std::string> memberNames;

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
		FunctionInfo* p_currentFunction;
		std::string currentExpectedReturnType;
		std::map<std::string, DataTypeOperators> typeNameOperators;
		std::map<std::string, DataTypeOperatorFunctions> typeNameToOpFuncs;
		std::map<std::string, DataTypeNativeOpFuncs> typeNameToNativeOpFuncs;
		std::map<std::string, StructInfo> typeNameToStructInfo;

		using SharedExp = std::shared_ptr<Expression>;
		using SharedNode = std::shared_ptr<LexNode>;

		Parser();

		void AffirmCurrentType(const std::string& typeName, int line, bool canBeAnyValueType = true);

		bool HasBody(const SharedNode& lexNode, size_t& outContentStartIndex, size_t& outContentCount);

		void FlattenNode(const SharedNode& lexNode, std::vector<SharedNode>& outNodes);

		void Parse(const std::vector<SharedNode>& lexNodes, std::vector<SharedExp>& expressions);

		// global structures
		SharedExp PGlobalStructure(const SharedNode& lexNode);

		SharedExp PStructDefinition(const SharedNode& lexNode);

		SharedExp PFunctionDefinition(const SharedNode& lexNode);

		SharedExp POperatorDefinition(const SharedNode& lexNode);

		// statements
		SharedExp PStatement(const SharedNode& lexNode);

		SharedExp PScope(const SharedNode& lexNode);

		SharedExp PReturn(const SharedNode& lexNode);

		SharedExp PIfSingle(const SharedNode& lexNode);

		SharedExp PIfChain(const SharedNode& lexNode);

		SharedExp PElseIfSingle(const SharedNode& lexNode);

		SharedExp PElseIfChain(const SharedNode& lexNode);

		SharedExp PElse(const SharedNode& lexNode);

		SharedExp PWhile(const SharedNode& lexNode);

		// expressions
		SharedExp PExpression(const SharedNode& lexNode);

		SharedExp PBinaryOp(const SharedNode& lexNode);

		SharedExp PAssign(const SharedNode& lexNode);

		SharedExp PWritablePtr(const SharedNode& lexNode, std::string& outWriteDataType);

		SharedExp PVariablePtr(const SharedNode& lexNode, std::string& outWriteDataType);

		SharedExp PMemberAccessPtr(const SharedNode& lexNode, std::string& outWriteDataType);

		SharedExp PDereferencePtr(const SharedNode& lexNode);

		SharedExp PReadableValue(const SharedNode& lexNode);

		SharedExp PLiteralConstantValue(const SharedNode& lexNode);

		SharedExp PVariableValue(const SharedNode& lexNode);

		SharedExp PMemberAccessValue(const SharedNode& lexNode);

		SharedExp PBinaryMathOp(const SharedNode& lexNode);

		SharedExp PBinaryCompareOp(const SharedNode& lexNode);

		SharedExp PUnaryOp(const SharedNode& lexNode);

		SharedExp PReferenceValue(const SharedNode& lexNode);

		SharedExp PDereferenceValue(const SharedNode& lexNode);

		SharedExp PNegate(const SharedNode& lexNode);

		SharedExp PNot(const SharedNode& lexNode);

		SharedExp PBitInvert(const SharedNode& lexNode);

		SharedExp PFunctionCall(const SharedNode& lexNode);

		SharedExp PUserFunctionCall(const SharedNode& lexNode);

		SharedExp PNativeFunctionCall(const SharedNode& lexNode);

		SharedExp PStructInitialization(const SharedNode& lexNode);
	};
}