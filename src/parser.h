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

	std::string GetFunctionHash(
		const std::string& returnTypeName,
		const std::string& funcName,
		const std::vector<std::string>& parameterTypeNames
	);

	struct Parser
	{
		typedef std::vector<OpCode> DataTypeOperators;
		typedef std::map<std::string, FunctionInfo> HashToFunction;
		typedef std::map<std::string, NativeFunctionInfo> HashToNativeFunction;

		std::map<std::string, Int> typeNameToSize;
		HashToFunction hashToUserFunctions;
		HashToNativeFunction hashToNativeFunctions;
		FunctionInfo* p_currentFunction;
		std::string currentExpectedReturnType;
		std::map<std::string, DataTypeOperators> typeNameOperators;
		std::map<std::string, StructInfo> typeNameToStructInfo;
		std::map<std::string, std::string> ptrTypeNameToStructTypeName;
		std::map<std::string, Int> nameToEnumValue;

		using SharedExp = std::shared_ptr<Expression>;
		using SharedNode = std::shared_ptr<LexNode>;

		Parser();

		bool IsFunctionDefined(const std::string& hash);

		void AffirmCurrentType(const std::string& typeName, int line, bool canBeAnyValueType = true);

		bool HasBody(const SharedNode& lexNode, size_t& outContentStartIndex, size_t& outContentCount);

		void FlattenNode(const SharedNode& lexNode, std::vector<SharedNode>& outNodes);

		void Parse(const std::vector<SharedNode>& lexNodes, std::vector<SharedExp>& expressions);

		// global structures
		SharedExp PGlobalStructure(const SharedNode& lexNode);

		SharedExp PStructDefinition(const SharedNode& lexNode);

		SharedExp PFunctionDefinition(const SharedNode& lexNode);

		SharedExp POperatorDefinition(const SharedNode& lexNode);

		SharedExp PMemberFunctionDefinition(const SharedNode& lexNode);

		SharedExp PEnumDefinition(const SharedNode& lexNode);

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

		SharedExp PBinaryOp(const SharedNode& lexNode, std::string& outReadDataType);

		SharedExp PAssign(const SharedNode& lexNode);

		SharedExp PWritablePtr(const SharedNode& lexNode, std::string& outWriteDataType);

		SharedExp PVariablePtr(const SharedNode& lexNode, std::string& outWriteDataType);

		SharedExp PMemberAccessPtr(const SharedNode& lexNode, std::string& outWriteDataType);

		SharedExp PDereferencePtr(const SharedNode& lexNode);

		SharedExp PReadableValue(const SharedNode& lexNode, std::string& outReadDataType);

		SharedExp PReadableValue(const SharedNode& lexNode);

		SharedExp PLiteralConstantValue(const SharedNode& lexNode, std::string& outReadDataType);

		SharedExp PVariableValue(const SharedNode& lexNode, std::string& outReadDataType);

		SharedExp PMemberAccessValue(const SharedNode& lexNode, std::string& outReadDataType);

		SharedExp PBinaryMathOp(const SharedNode& lexNode, std::string& outReadDataType);

		SharedExp PBinaryCompareOp(const SharedNode& lexNode, std::string& outReadDataType);

		SharedExp PUnaryOp(const SharedNode& lexNode, std::string& outReadDataType);

		SharedExp PReferenceValue(const SharedNode& lexNode, std::string& outReadDataType);

		SharedExp PDereferenceValue(const SharedNode& lexNode, std::string& outReadDataType);

		SharedExp PNegate(const SharedNode& lexNode, std::string& outReadDataType);

		SharedExp PNot(const SharedNode& lexNode, std::string& outReadDataType);

		SharedExp PBitInvert(const SharedNode& lexNode, std::string& outReadDataType);

		SharedExp PFunctionCall(const SharedNode& lexNode, std::string& outReadDataType);

		SharedExp PUserOrNativeFunctionCall(const SharedNode& lexNode, std::string& outReadDataType);

		SharedExp PStructInitialization(const SharedNode& lexNode, std::string& outReadDataType);

		SharedExp PStructPtrInitialization(const SharedNode& lexNode, std::string& outReadDataType);

		SharedExp PMemberFunctionCall(const SharedNode& lexNode, std::string& outReadDataType);
	};
}