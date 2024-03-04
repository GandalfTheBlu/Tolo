#include "parser.h"
#include <utility>

#define ANY_VALUE_TYPE "0any"

namespace Tolo
{
	VariableInfo::VariableInfo() :
		offset(0)
	{}

	VariableInfo::VariableInfo(const std::string& _typeName, Int _offset) :
		typeName(_typeName),
		offset(_offset)
	{}

	VariableInfo::VariableInfo(const VariableInfo& rhs) :
		typeName(rhs.typeName),
		offset(rhs.offset)
	{}

	VariableInfo& VariableInfo::operator=(const VariableInfo& rhs)
	{
		typeName = rhs.typeName;
		offset = rhs.offset;
		return *this;
	}

	FunctionInfo::FunctionInfo() :
		localsSize(0),
		parametersSize(0)
	{}

	NativeFunctionInfo::NativeFunctionInfo() :
		p_functionPtr(nullptr)
	{}

	StructInfo::StructInfo()
	{}


	std::string GetFunctionHash(
		const std::string& returnTypeName,
		const std::string& funcName,
		const std::vector<std::string>& parameterTypeNames
	)
	{
		std::string hash = returnTypeName + " " + funcName + "(";

		bool first = true;
		for (const std::string& paramTypeName : parameterTypeNames)
		{
			if (!first)
				hash += ", ";

			hash += paramTypeName;

			first = false;
		}

		hash += ")";

		return hash;
	}


	Parser::Parser() :
		p_currentFunction(nullptr)
	{
		typeNameToSize["void"] = 0;
		typeNameToSize["char"] = sizeof(Char);
		typeNameToSize["int"] = sizeof(Int);
		typeNameToSize["float"] = sizeof(Float);
		typeNameToSize["ptr"] = sizeof(Ptr);

		typeNameOperators["char"] =
		{
			OpCode::Char_Add,
			OpCode::Char_Sub,
			OpCode::Char_Mul,
			OpCode::Char_Div,
			OpCode::Bit_8_And,
			OpCode::Bit_8_Or,
			OpCode::Bit_8_Xor,
			OpCode::Bit_8_LeftShift,
			OpCode::Bit_8_RightShift,
			OpCode::Char_Less,
			OpCode::Char_Greater,
			OpCode::Char_Equal,
			OpCode::Char_LessOrEqual,
			OpCode::Char_GreaterOrEqual,
			OpCode::Char_NotEqual,
			OpCode::And,
			OpCode::Or,
			OpCode::Char_Negate,
			OpCode::Bit_8_Invert
		};

		typeNameOperators["int"] =
		{
			OpCode::Int_Add,
			OpCode::Int_Sub,
			OpCode::Int_Mul,
			OpCode::Int_Div,
			OpCode::Bit_32_And,
			OpCode::Bit_32_Or,
			OpCode::Bit_32_Xor,
			OpCode::Bit_32_LeftShift,
			OpCode::Bit_32_RightShift,
			OpCode::Int_Less,
			OpCode::Int_Greater,
			OpCode::Int_Equal,
			OpCode::Int_LessOrEqual,
			OpCode::Int_GreaterOrEqual,
			OpCode::Int_NotEqual,
			OpCode::INVALID,
			OpCode::INVALID,
			OpCode::Int_Negate,
			OpCode::Bit_32_Invert
		};

		typeNameOperators["float"] =
		{
			OpCode::Float_Add,
			OpCode::Float_Sub,
			OpCode::Float_Mul,
			OpCode::Float_Div,
			OpCode::Bit_32_And,
			OpCode::Bit_32_Or,
			OpCode::Bit_32_Xor,
			OpCode::Bit_32_LeftShift,
			OpCode::Bit_32_RightShift,
			OpCode::Float_Less,
			OpCode::Float_Greater,
			OpCode::Float_Equal,
			OpCode::Float_LessOrEqual,
			OpCode::Float_GreaterOrEqual,
			OpCode::Float_NotEqual,
			OpCode::INVALID,
			OpCode::INVALID,
			OpCode::Float_Negate,
			OpCode::Bit_32_Invert
		};

		typeNameOperators["ptr"] =
		{
			OpCode::Ptr_Add,
			OpCode::Ptr_Sub,
			OpCode::INVALID,
			OpCode::INVALID,
			OpCode::INVALID,
			OpCode::INVALID,
			OpCode::INVALID,
			OpCode::INVALID,
			OpCode::INVALID,
			OpCode::Ptr_Less,
			OpCode::Ptr_Greater,
			OpCode::Ptr_Equal,
			OpCode::Ptr_LessOrEqual,
			OpCode::Ptr_GreaterOrEqual,
			OpCode::Ptr_NotEqual,
			OpCode::INVALID,
			OpCode::INVALID,
			OpCode::INVALID,
			OpCode::INVALID
		};

		currentExpectedReturnType = "void";
	}

	bool Parser::IsFunctionDefined(const std::string& hash)
	{
		return hashToUserFunctions.count(hash) != 0 || hashToNativeFunctions.count(hash) != 0;
	}

	void Parser::AffirmCurrentType(const std::string& typeName, int line, bool canBeAnyValueType)
	{
		if (canBeAnyValueType && typeName != "void" && currentExpectedReturnType == ANY_VALUE_TYPE)
			return;

		Affirm(
			currentExpectedReturnType == typeName,
			"expected expression of type '%s' but got '%s' at line %i",
			currentExpectedReturnType.c_str(), typeName.c_str(), line
		);
	}

	bool Parser::HasBody(const SharedNode& lexNode, size_t& outContentStartIndex, size_t& outContentCount)
	{
		switch (lexNode->type)
		{
		case LexNode::Type::Scope:
			outContentStartIndex = 0;
			outContentCount = lexNode->children.size();
			break;
		case LexNode::Type::Else:
			outContentStartIndex = 0;
			outContentCount = 1;
			break;
		case LexNode::Type::While:
		case LexNode::Type::IfSingle:
		case LexNode::Type::IfChain:
		case LexNode::Type::ElseIfSingle:
		case LexNode::Type::ElseIfChain:
			outContentStartIndex = 1;
			outContentCount = 1;
			break;
		default:
			return false;
		}

		return true;
	}

	void Parser::FlattenNode(const SharedNode& lexNode, std::vector<SharedNode>& outNodes)
	{
		size_t contentStart = 0;
		size_t contentCount = 0;

		if (!HasBody(lexNode, contentStart, contentCount))
		{
			outNodes.push_back(lexNode);
			return;
		}
		
		for(size_t i=0; i<contentCount; i++)
			FlattenNode(lexNode->children[contentStart + i], outNodes);
	}
	
	void Parser::Parse(const std::vector<SharedNode>& lexNodes, std::vector<SharedExp>& expressions)
	{
		for (const SharedNode& node : lexNodes)
			expressions.push_back(PGlobalStructure(node));
	}

	// global structures
	Parser::SharedExp Parser::PGlobalStructure(const SharedNode& lexNode) 
	{
		switch (lexNode->type)
		{
		case LexNode::Type::StructDefinition:
			return PStructDefinition(lexNode);
		case LexNode::Type::FunctionDefinition:
			return PFunctionDefinition(lexNode);
		case LexNode::Type::OperatorDefinition:
			return POperatorDefinition(lexNode);
		}

		return PMemberFunctionDefinition(lexNode);
	}

	Parser::SharedExp Parser::PStructDefinition(const SharedNode& lexNode) 
	{
		const std::string& structName = lexNode->token.text;

		Affirm(
			typeNameToSize.contains(structName) == 0,
			"type name '%s' at line %i is already defined",
			structName.c_str(), lexNode->token.line
		);

		std::string structPtrName = structName + "::ptr";
		ptrTypeNameToStructTypeName[structPtrName] = structName;
		typeNameToSize[structPtrName] = sizeof(Ptr);

		StructInfo& structInfo = typeNameToStructInfo[structName];

		Int propertyOffset = 0;
		for (size_t i = 0; i + 1 < lexNode->children.size(); i += 2)
		{
			const std::string& membTypeName = lexNode->children[i]->token.text;
			const std::string& membName = lexNode->children[i + 1]->token.text;

			Affirm(
				membTypeName != structName,
				"struct cannot contain itself, line %i",
				lexNode->children[i]->token.line
			);

			Affirm(
				typeNameToSize.count(membTypeName) != 0,
				"type name '%s' at line %i is not defined",
				membTypeName.c_str(), lexNode->children[i]->token.line
			);

			Affirm(
				structInfo.memberNameToVarInfo.count(membName) == 0,
				"member '%s' at line %i is already defined in struct '%s'",
				membName.c_str(), lexNode->children[i + 1]->token.line, structName.c_str()
			);

			structInfo.memberNameToVarInfo[membName] = VariableInfo(membTypeName, propertyOffset);
			structInfo.memberNames.push_back(membName);
			propertyOffset += typeNameToSize[membTypeName];
		}

		typeNameToSize[structName] = propertyOffset;

		return std::make_shared<EEmpty>();
	}

	Parser::SharedExp Parser::PFunctionDefinition(const SharedNode& lexNode) 
	{
		const std::string& returnTypeName = lexNode->token.text;

		Affirm(
			typeNameToSize.count(returnTypeName) != 0,
			"undefined type '%s' at line %i",
			returnTypeName.c_str(), lexNode->token.line
		);

		const std::string& funcName = lexNode->children[0]->token.text;
		
		FunctionInfo funcInfo;
		funcInfo.returnTypeName = returnTypeName;
		Int nextVarOffset = 0;

		// flatten content nodes into a single array to find all variable definitions
		std::vector<SharedNode> bodyContent;
		FlattenNode(lexNode->children.back(), bodyContent);

		// find all local variable definitions
		for (auto statNode : bodyContent)
		{
			if (statNode->type != LexNode::Type::BinaryOperation ||
				statNode->children[0]->type != LexNode::Type::VariableDefinition)
			{
				continue;
			}

			const SharedNode& varDefNode = statNode->children[0];

			const std::string& varTypeName = varDefNode->token.text;
			const std::string& varName = varDefNode->children[0]->token.text;

			Affirm(
				typeNameToSize.count(varTypeName) != 0,
				"undefined type '%s' at line %i",
				varTypeName.c_str(), statNode->token.line
			);

			Affirm(
				funcInfo.varNameToVarInfo.count(varName) == 0,
				"variable '%s' at line %i is already defined",
				varName.c_str(), varDefNode->token.line
			);

			Int varSize = typeNameToSize[varTypeName];
			nextVarOffset -= varSize;
			funcInfo.localsSize += varSize;
			funcInfo.varNameToVarInfo[varName] = { varTypeName, nextVarOffset };
		}

		// find all parameters
		std::vector<std::string> paramTypeNames;
		for (size_t i = 1; i + 2 < lexNode->children.size(); i += 2)
		{
			const std::string& paramTypeName = lexNode->children[i]->token.text;
			const std::string& paramName = lexNode->children[i + 1]->token.text;

			Affirm(
				typeNameToSize.count(paramTypeName) != 0,
				"undefined type '%s' at line %i",
				paramTypeName.c_str(), lexNode->children[i]->token.line
			);

			Affirm(
				funcInfo.varNameToVarInfo.count(paramName) == 0,
				"variable '%s' at line %i is already defined",
				paramName.c_str(), lexNode->children[i + 1]->token.line
			);

			paramTypeNames.push_back(paramTypeName);

			Int varSize = typeNameToSize[paramTypeName];
			nextVarOffset -= varSize;
			funcInfo.parametersSize += varSize;
			funcInfo.varNameToVarInfo[paramName] = { paramTypeName, nextVarOffset };
			funcInfo.parameterNames.push_back(paramName);
		}

		std::string funcHash = GetFunctionHash(funcInfo.returnTypeName, funcName, paramTypeNames);

		Affirm(
			!IsFunctionDefined(funcHash),
			"function '%s' at line %i is already defined",
			funcName, lexNode->token.line
		);

		hashToUserFunctions[funcHash] = funcInfo;

		p_currentFunction = &funcInfo;

		// parse body content
		auto defFuncExp = std::make_shared<EDefineFunction>(funcHash);
		defFuncExp->body.push_back(PStatement(lexNode->children.back()));

		p_currentFunction = nullptr;

		// check for final return expression
		if (funcInfo.returnTypeName == "void" &&
			(defFuncExp->body.size() == 0 || bodyContent.back()->type != LexNode::Type::Return))
		{
			// add a return statement if function has void return type and no return statement exists at end
			// of function
			defFuncExp->body.push_back(std::make_shared<EReturn>(0));
		}
		else
		{
			Affirm(
				defFuncExp->body.size() > 0 && bodyContent.back()->type == LexNode::Type::Return,
				"missing 'return' in function '%s' at line %i",
				funcName.c_str(), lexNode->token.line
			);
		}

		return defFuncExp;
	}

	Parser::SharedExp Parser::POperatorDefinition(const SharedNode& lexNode) 
	{
		const std::string& returnTypeName = lexNode->token.text;
		Affirm(
			typeNameToSize.count(returnTypeName) != 0,
			"undefined type '%s' at line %i",
			returnTypeName.c_str(), lexNode->token.line
		);

		std::string opName = lexNode->children[0]->token.text;

		FunctionInfo funcInfo;
		funcInfo.returnTypeName = returnTypeName;
		Int nextVarOffset = 0;

		// flatten content nodes into a single array to find all variable definitions
		std::vector<SharedNode> bodyContent;
		FlattenNode(lexNode->children.back(), bodyContent);

		// find all local variable definitions
		for (auto statNode : bodyContent)
		{
			if (statNode->type != LexNode::Type::BinaryOperation ||
				statNode->children[0]->type != LexNode::Type::VariableDefinition)
			{
				continue;
			}

			const SharedNode& varDefNode = statNode->children[0];

			const std::string& varTypeName = varDefNode->token.text;
			const std::string& varName = varDefNode->children[0]->token.text;

			Affirm(
				typeNameToSize.count(varTypeName) != 0,
				"undefined type '%s' at line %i",
				varTypeName.c_str(), statNode->token.line
			);

			Affirm(
				funcInfo.varNameToVarInfo.count(varName) == 0,
				"variable '%s' at line %i is already defined",
				varName.c_str(), varDefNode->token.line
			);

			Int varSize = typeNameToSize[varTypeName];
			nextVarOffset -= varSize;
			funcInfo.localsSize += varSize;
			funcInfo.varNameToVarInfo[varName] = { varTypeName, nextVarOffset };
		}

		// find all parameters and determine operand type from first parameter
		std::string operandTypeName;

		for (size_t i = 1; i + 2 < lexNode->children.size(); i += 2)
		{
			const std::string& varTypeName = lexNode->children[i]->token.text;
			const std::string& varName = lexNode->children[i + 1]->token.text;

			if (i == 1)
				operandTypeName = varTypeName;

			Affirm(
				funcInfo.varNameToVarInfo.count(varName) == 0,
				"variable '%s' at line %i is already defined",
				varName.c_str(), lexNode->children[i + 1]->token.line
			);

			Int varSize = typeNameToSize[varTypeName];
			nextVarOffset -= varSize;
			funcInfo.parametersSize += varSize;
			funcInfo.varNameToVarInfo[varName] = { varTypeName, nextVarOffset };
			funcInfo.parameterNames.push_back(varName);
		}

		if (opName == "-" && funcInfo.parameterNames.size() == 1)
			opName = "negate";

		DataTypeFunctions& opFunctions = typeNameToOpFuncs[operandTypeName];
		Affirm(
			opFunctions.count(opName) == 0 &&
			(typeNameToNativeOpFuncs.count(operandTypeName) == 0 || 
			typeNameToNativeOpFuncs[operandTypeName].count(opName) == 0),
			"operator '%s' for type '%s' at line %i is already defined",
			opName.c_str(), operandTypeName.c_str(), lexNode->token.line
		);
		opFunctions[opName] = funcInfo;

		auto defFuncExp = std::make_shared<EDefineFunction>(operandTypeName + opName);

		p_currentFunction = &funcInfo;

		// parse body content
		defFuncExp->body.push_back(PStatement(lexNode->children.back()));

		p_currentFunction = nullptr;

		// check for final return expression
		Affirm(
			defFuncExp->body.size() > 0 && bodyContent.back()->type == LexNode::Type::Return,
			"operator function at line %i does not return a value",
			lexNode->token.line
		);

		return defFuncExp;
	}

	Parser::SharedExp Parser::PMemberFunctionDefinition(const SharedNode& lexNode)
	{
		const std::string& returnTypeName = lexNode->token.text;
		const std::string& structTypeName = lexNode->children[0]->token.text;
		const std::string& funcName = lexNode->children[1]->token.text;

		Affirm(
			typeNameToSize.count(returnTypeName) != 0,
			"undefined type '%s' at line %i",
			returnTypeName.c_str(), lexNode->token.line
		);

		Affirm(
			typeNameToStructInfo.count(structTypeName) != 0,
			"undefined struct type '%s' at line %i",
			structTypeName.c_str(), lexNode->token.line
		);

		DataTypeFunctions& structFuncs = typeNameToMemberFunctions[structTypeName];

		Affirm(
			structFuncs.count(funcName) == 0,
			"name '%s' at line %i is already a defined member function",
			funcName.c_str(), lexNode->children[1]->token.line
		);

		auto defFuncExp = std::make_shared<EDefineFunction>(structTypeName + "::" + funcName);
		FunctionInfo& funcInfo = structFuncs[funcName];
		funcInfo.returnTypeName = returnTypeName;
		Int nextVarOffset = 0;

		// flatten content nodes into a single array to find all variable definitions
		std::vector<SharedNode> bodyContent;
		FlattenNode(lexNode->children.back(), bodyContent);

		// find all local variable definitions
		for (auto statNode : bodyContent)
		{
			if (statNode->type != LexNode::Type::BinaryOperation ||
				statNode->children[0]->type != LexNode::Type::VariableDefinition)
			{
				continue;
			}

			const SharedNode& varDefNode = statNode->children[0];

			const std::string& varTypeName = varDefNode->token.text;
			const std::string& varName = varDefNode->children[0]->token.text;

			Affirm(
				typeNameToSize.count(varTypeName) != 0,
				"undefined type '%s' at line %i",
				varTypeName.c_str(), statNode->token.line
			);

			Affirm(
				funcInfo.varNameToVarInfo.count(varName) == 0,
				"variable '%s' at line %i is already defined",
				varName.c_str(), varDefNode->token.line
			);

			Int varSize = typeNameToSize[varTypeName];
			nextVarOffset -= varSize;
			funcInfo.localsSize += varSize;
			funcInfo.varNameToVarInfo[varName] = { varTypeName, nextVarOffset };
		}

		// add this ptr
		{
			Int varSize = sizeof(Ptr);
			nextVarOffset -= varSize;
			funcInfo.parametersSize += varSize;
			funcInfo.varNameToVarInfo["this"] = VariableInfo(structTypeName + "::ptr", nextVarOffset);
		}

		// find all parameters
		for (size_t i = 2; i + 2 < lexNode->children.size(); i += 2)
		{
			const std::string& varTypeName = lexNode->children[i]->token.text;
			const std::string& varName = lexNode->children[i + 1]->token.text;

			Affirm(
				typeNameToSize.count(varTypeName) != 0,
				"undefined type '%s' at line %i",
				varTypeName.c_str(), lexNode->children[i]->token.line
			);

			Affirm(
				funcInfo.varNameToVarInfo.count(varName) == 0,
				"variable '%s' at line %i is already defined",
				varName.c_str(), lexNode->children[i + 1]->token.line
			);

			Int varSize = typeNameToSize[varTypeName];
			nextVarOffset -= varSize;
			funcInfo.parametersSize += varSize;
			funcInfo.varNameToVarInfo[varName] = { varTypeName, nextVarOffset };
			funcInfo.parameterNames.push_back(varName);
		}

		p_currentFunction = &funcInfo;

		// parse body content
		defFuncExp->body.push_back(PStatement(lexNode->children.back()));

		p_currentFunction = nullptr;

		// check for final return expression
		if (funcInfo.returnTypeName == "void" &&
			(defFuncExp->body.size() == 0 || bodyContent.back()->type != LexNode::Type::Return))
		{
			// add a return statement if function has void return type and no return statement exists at end
			// of function
			defFuncExp->body.push_back(std::make_shared<EReturn>(0));
		}
		else
		{
			Affirm(
				defFuncExp->body.size() > 0 && bodyContent.back()->type == LexNode::Type::Return,
				"missing 'return' in function '%s' at line %i",
				funcName.c_str(), lexNode->token.line
			);
		}

		return defFuncExp;
	}

	// statements
	Parser::SharedExp Parser::PStatement(const SharedNode& lexNode) 
	{
		switch (lexNode->type)
		{
		case LexNode::Type::Break:
			return std::make_shared<EBreak>();
		case LexNode::Type::Continue:
			return std::make_shared<EContinue>();
		case LexNode::Type::Scope:
			return PScope(lexNode);
		case LexNode::Type::Return:
			return PReturn(lexNode);
		case LexNode::Type::IfSingle:
			return PIfSingle(lexNode);
		case LexNode::Type::IfChain:
			return PIfChain(lexNode);
		case LexNode::Type::ElseIfSingle:
			return PElseIfSingle(lexNode);
		case LexNode::Type::ElseIfChain:
			return PElseIfChain(lexNode);
		case LexNode::Type::Else:
			return PElse(lexNode);
		case LexNode::Type::While:
			return PWhile(lexNode);
		}

		return PExpression(lexNode);
	}

	Parser::SharedExp Parser::PScope(const SharedNode& lexNode)
	{
		auto scopeExp = std::make_shared<EScope>();
		
		for (const SharedNode& statNode : lexNode->children)
			scopeExp->statements.push_back(PStatement(statNode));

		return scopeExp;
	}

	Parser::SharedExp Parser::PReturn(const SharedNode& lexNode) 
	{
		auto retExp = std::make_shared<EReturn>(typeNameToSize[p_currentFunction->returnTypeName]);

		if (lexNode->children.size() == 0)
		{
			Affirm(
				p_currentFunction->returnTypeName == "void",
				"missing value expression after 'return' keyword at line %i",
				lexNode->token.line
			);
		}
		else
		{
			currentExpectedReturnType = p_currentFunction->returnTypeName;
			retExp->retValLoad = PReadableValue(lexNode->children[0]);
			currentExpectedReturnType = "void";
		}

		return retExp;
	}

	Parser::SharedExp Parser::PIfSingle(const SharedNode& lexNode) 
	{
		auto ifExp = std::make_shared<EIfSingle>();

		currentExpectedReturnType = "char";
		ifExp->conditionLoad = PReadableValue(lexNode->children[0]);
		currentExpectedReturnType = "void";

		ifExp->body.push_back(PStatement(lexNode->children[1]));

		return ifExp;
	}

	Parser::SharedExp Parser::PIfChain(const SharedNode& lexNode) 
	{
		auto ifChainExp = std::make_shared<EIfChain>();

		currentExpectedReturnType = "char";
		ifChainExp->conditionLoad = PReadableValue(lexNode->children[0]);
		currentExpectedReturnType = "void";

		ifChainExp->body.push_back(PStatement(lexNode->children[1]));
		ifChainExp->chain = PStatement(lexNode->children[2]);

		return ifChainExp;
	}

	Parser::SharedExp Parser::PElseIfSingle(const SharedNode& lexNode) 
	{
		auto elifExp = std::make_shared<EElseIfSingle>();

		currentExpectedReturnType = "char";
		elifExp->conditionLoad = PReadableValue(lexNode->children[0]);
		currentExpectedReturnType = "void";

		elifExp->body.push_back(PStatement(lexNode->children[1]));

		return elifExp;
	}

	Parser::SharedExp Parser::PElseIfChain(const SharedNode& lexNode) 
	{
		auto elifExp = std::make_shared<EElseIfChain>();

		currentExpectedReturnType = "char";
		elifExp->conditionLoad = PReadableValue(lexNode->children[0]);
		currentExpectedReturnType = "void";

		elifExp->body.push_back(PStatement(lexNode->children[1]));
		elifExp->chain = PStatement(lexNode->children[2]);

		return elifExp;
	}

	Parser::SharedExp Parser::PElse(const SharedNode& lexNode) 
	{
		auto elseExp = std::make_shared<EElse>();

		elseExp->body.push_back(PStatement(lexNode->children[0]));

		return elseExp;
	}

	Parser::SharedExp Parser::PWhile(const SharedNode& lexNode) 
	{
		auto whileExp = std::make_shared<EWhile>();

		currentExpectedReturnType = "char";
		whileExp->conditionLoad = PReadableValue(lexNode->children[0]);
		currentExpectedReturnType = "void";

		whileExp->body.push_back(PStatement(lexNode->children[1]));

		return whileExp;
	}

	// expressions
	Parser::SharedExp Parser::PExpression(const SharedNode& lexNode)
	{
		std::string unused;

		if (lexNode->type == LexNode::Type::BinaryOperation)
			return PBinaryOp(lexNode, unused);

		return PReadableValue(lexNode);
	}

	Parser::SharedExp Parser::PBinaryOp(const SharedNode& lexNode, std::string& outReadDataType)
	{
		switch (lexNode->token.type)
		{
		case Token::Type::EqualSign:
			return PAssign(lexNode);
		case Token::Type::Plus:
		case Token::Type::Minus:
		case Token::Type::Asterisk:
		case Token::Type::ForwardSlash:
		case Token::Type::Ampersand:
		case Token::Type::VerticalBar:
		case Token::Type::Caret:
		case Token::Type::DoubleLeftArrow:
		case Token::Type::DoubleRightArrow:
			return PBinaryMathOp(lexNode, outReadDataType);
		}

		return PBinaryCompareOp(lexNode, outReadDataType);
	}

	Parser::SharedExp Parser::PAssign(const SharedNode& lexNode) 
	{
		auto writeBytesExp = std::make_shared<EWriteBytesTo>();

		std::string expectedWriteType;
		writeBytesExp->writePtrLoad = PWritablePtr(lexNode->children[0], expectedWriteType);

		currentExpectedReturnType = expectedWriteType;
		std::string readType;
		writeBytesExp->dataLoad = PReadableValue(lexNode->children[1], readType);
		currentExpectedReturnType = "void";

		Int byteSize = typeNameToSize.at(readType);
		writeBytesExp->bytesSizeLoad = std::make_shared<ELoadConstInt>(byteSize);

		return writeBytesExp;
	}

	Parser::SharedExp Parser::PWritablePtr(const SharedNode& lexNode, std::string& outWriteDataType)
	{
		switch (lexNode->type)
		{
		case LexNode::Type::VariableDefinition:
			return PVariablePtr(lexNode->children[0], outWriteDataType);
		case LexNode::Type::Identifier:
			return PVariablePtr(lexNode, outWriteDataType);
		case LexNode::Type::MemberVariableAccess:
			return PMemberAccessPtr(lexNode, outWriteDataType);
		}

		if (lexNode->type == LexNode::Type::UnaryOperation && lexNode->token.type == Token::Type::Asterisk)
		{
			outWriteDataType = ANY_VALUE_TYPE;
			return PDereferencePtr(lexNode);
		}

		Affirm(
			false, 
			"left hand side of assignment was not a writable pointer at line %i", 
			lexNode->token.line
		);

		return nullptr;
	}

	Parser::SharedExp Parser::PVariablePtr(const SharedNode& lexNode, std::string& outWriteDataType)
	{
		const std::string& varName = lexNode->token.text;

		Affirm(
			p_currentFunction->varNameToVarInfo.count(varName) != 0,
			"undefined variable '%s' at line %i",
			varName.c_str(), lexNode->token.line
		);

		const VariableInfo& varInfo = p_currentFunction->varNameToVarInfo.at(varName);
		outWriteDataType = varInfo.typeName;

		return std::make_shared<ELoadVariablePtr>(varInfo.offset);
	}

	Parser::SharedExp Parser::PMemberAccessPtr(const SharedNode& lexNode, std::string& outWriteDataType)
	{
		Affirm(
			p_currentFunction->varNameToVarInfo.count(lexNode->token.text) != 0,
			"undefined variable '%s' at line %i",
			lexNode->token.text.c_str(), lexNode->token.line
		);

		const VariableInfo& varInfo = p_currentFunction->varNameToVarInfo.at(lexNode->token.text);

		auto loadMembPtrExp = std::make_shared<ELoadMulti>();
		std::string parentStructType = varInfo.typeName;

		// load variable pointer
		if (ptrTypeNameToStructTypeName.count(varInfo.typeName) != 0)
		{
			// variable is struct pointer
			parentStructType = ptrTypeNameToStructTypeName.at(varInfo.typeName);
			loadMembPtrExp->loaders.push_back(std::make_shared<ELoadVariable>(varInfo.offset, static_cast<Int>(sizeof(Ptr))));
		}
		else
		{
			Affirm(
				typeNameToStructInfo.count(varInfo.typeName) != 0,
				"type '%s' at line %i is not a struct",
				varInfo.typeName.c_str(), lexNode->token.line
			);

			// variable is struct value
			loadMembPtrExp->loaders.push_back(std::make_shared<ELoadVariablePtr>(varInfo.offset));
		}

		// traverse dot chain to reach final pointer
		for (const SharedNode& membNode : lexNode->children)
		{
			Affirm(
				typeNameToStructInfo.count(parentStructType),
				"type '%s' at line %i is not a struct",
				parentStructType.c_str(), membNode->token.line
			);

			const StructInfo& parentStructInfo = typeNameToStructInfo.at(parentStructType);

			Affirm(
				parentStructInfo.memberNameToVarInfo.count(membNode->token.text) != 0,
				"'%s' at line %i is not a member of struct '%s'",
				membNode->token.text.c_str(), membNode->token.line, parentStructType.c_str()
			);

			const VariableInfo& membInfo = parentStructInfo.memberNameToVarInfo.at(membNode->token.text);

			if (ptrTypeNameToStructTypeName.count(membInfo.typeName) != 0)
			{
				// member is struct pointer
				parentStructType = ptrTypeNameToStructTypeName.at(membInfo.typeName);

				// push member variable address
				loadMembPtrExp->loaders.push_back(std::make_shared<ELoadConstInt>(membInfo.offset));
				loadMembPtrExp->loaders.push_back(std::make_shared<EPtrAdd>());
				// push pointer stored in member variable
				loadMembPtrExp->loaders.push_back(std::make_shared<ELoadPtrFromStackTopPtr>());
			}
			else
			{
				// member is struct value
				parentStructType = membInfo.typeName;

				// push member variable address
				loadMembPtrExp->loaders.push_back(std::make_shared<ELoadConstInt>(membInfo.offset));
				loadMembPtrExp->loaders.push_back(std::make_shared<EPtrAdd>());
			}
		}

		outWriteDataType = parentStructType;

		return loadMembPtrExp;
	}

	Parser::SharedExp Parser::PDereferencePtr(const SharedNode& lexNode) 
	{
		currentExpectedReturnType = "ptr";
		return PReadableValue(lexNode->children[0]);
		currentExpectedReturnType = "void";
	}

	Parser::SharedExp Parser::PReadableValue(const SharedNode& lexNode, std::string& outReadDataType)
	{
		switch (lexNode->type)
		{
		case LexNode::Type::Parenthesis:
			return PReadableValue(lexNode->children[0], outReadDataType);
		case LexNode::Type::LiteralConstant:
			return PLiteralConstantValue(lexNode, outReadDataType);
		case LexNode::Type::Identifier:
			return PVariableValue(lexNode, outReadDataType);
		case LexNode::Type::MemberVariableAccess:
			return PMemberAccessValue(lexNode, outReadDataType);
		case LexNode::Type::BinaryOperation:
			return PBinaryOp(lexNode, outReadDataType);
		case LexNode::Type::UnaryOperation:
			return PUnaryOp(lexNode, outReadDataType);
		case LexNode::Type::FunctionCall:
			return PFunctionCall(lexNode, outReadDataType);
		case LexNode::Type::MemberFunctionCall:
			return PMemberFunctionCall(lexNode, outReadDataType);
		}

		Affirm(
			false,
			"expected value expression at line %i",
			lexNode->token.line
		);

		return nullptr;
	}

	Parser::SharedExp Parser::PReadableValue(const SharedNode& lexNode)
	{
		std::string unused;
		return PReadableValue(lexNode, unused);
	}

	Parser::SharedExp Parser::PLiteralConstantValue(const SharedNode& lexNode, std::string& outReadDataType)
	{
		if (lexNode->token.type == Token::Type::ConstChar)
		{
			AffirmCurrentType("char", lexNode->token.line);
			outReadDataType = "char";

			Char value = lexNode->token.text[0];
			return std::make_shared<ELoadConstChar>(value);
		}
		if (lexNode->token.type == Token::Type::ConstInt)
		{
			AffirmCurrentType("int", lexNode->token.line);
			outReadDataType = "int";

			Int value = std::stoi(lexNode->token.text);
			return std::make_shared<ELoadConstInt>(value);
		}
		if (lexNode->token.type == Token::Type::ConstFloat)
		{
			AffirmCurrentType("float", lexNode->token.line);
			outReadDataType = "float";

			Float value = std::stof(lexNode->token.text);
			return std::make_shared<ELoadConstFloat>(value);
		}
		if (lexNode->token.type == Token::Type::ConstString)
		{
			AffirmCurrentType("ptr", lexNode->token.line);
			outReadDataType = "ptr";

			const std::string& value = lexNode->token.text;
			return std::make_shared<ELoadConstString>(value);
		}

		return nullptr;
	}

	Parser::SharedExp Parser::PVariableValue(const SharedNode& lexNode, std::string& outReadDataType) 
	{
		const std::string& varName = lexNode->token.text;

		Affirm(
			p_currentFunction->varNameToVarInfo.count(varName) != 0,
			"undefined variable '%s' at line %i",
			varName.c_str(), lexNode->token.line
		);

		const VariableInfo& info = p_currentFunction->varNameToVarInfo[varName];

		AffirmCurrentType(info.typeName, lexNode->token.line);
		outReadDataType = info.typeName;

		return std::make_shared<ELoadVariable>(info.offset, typeNameToSize[info.typeName]);
	}

	Parser::SharedExp Parser::PMemberAccessValue(const SharedNode& lexNode, std::string& outReadDataType)
	{
		auto membPtrLoadExp = PMemberAccessPtr(lexNode, outReadDataType);
		auto membValLoadExp = std::make_shared<ELoadBytesFromPtr>(typeNameToSize.at(outReadDataType));
		membValLoadExp->ptrLoad = membPtrLoadExp;

		return membValLoadExp;
	}

	Parser::SharedExp Parser::PBinaryMathOp(const SharedNode& lexNode, std::string& outReadDataType)
	{
		static std::map<Token::Type, size_t> opTypeToOpIndex
		{
			{Token::Type::Plus, 0},
			{Token::Type::Minus, 1},
			{Token::Type::Asterisk, 2},
			{Token::Type::ForwardSlash, 3},
			{Token::Type::Ampersand, 4},
			{Token::Type::VerticalBar, 5},
			{Token::Type::Caret, 6},
			{Token::Type::DoubleLeftArrow, 7},
			{Token::Type::DoubleRightArrow, 8}
		};

		std::string readType;
		auto lhsExp = PReadableValue(lexNode->children[0], readType);

		if (currentExpectedReturnType == ANY_VALUE_TYPE)
			currentExpectedReturnType = readType;

		outReadDataType = currentExpectedReturnType;

		if (typeNameToOpFuncs.count(currentExpectedReturnType) != 0 &&
			typeNameToOpFuncs[currentExpectedReturnType].count(lexNode->token.text) != 0)
		{
			const DataTypeFunctions& opFunctions = typeNameToOpFuncs[currentExpectedReturnType];
			const std::string& opName = lexNode->token.text;

			const FunctionInfo& funcInfo = opFunctions.at(opName);

			auto callOpExp = std::make_shared<ECallFunction>(funcInfo.parametersSize, funcInfo.localsSize);
			callOpExp->argumentLoads.push_back(lhsExp);

			std::string oldRetType = currentExpectedReturnType;
			currentExpectedReturnType = ANY_VALUE_TYPE;
			callOpExp->argumentLoads.push_back(PReadableValue(lexNode->children[1]));
			currentExpectedReturnType = oldRetType;

			callOpExp->functionIpLoad = std::make_shared<ELoadConstPtrToLabel>(currentExpectedReturnType + opName);

			return callOpExp;
		}
		if (typeNameToNativeOpFuncs.count(currentExpectedReturnType) != 0 &&
			typeNameToNativeOpFuncs[currentExpectedReturnType].count(lexNode->token.text) != 0)
		{
			const DataTypeNativeFunctions& opFunctions = typeNameToNativeOpFuncs[currentExpectedReturnType];
			const std::string& opName = lexNode->token.text;

			const NativeFunctionInfo& funcInfo = opFunctions.at(opName);

			auto callNativeOpExp = std::make_shared<ECallNativeFunction>();
			callNativeOpExp->argumentLoads.push_back(lhsExp);

			std::string oldRetType = currentExpectedReturnType;
			currentExpectedReturnType = ANY_VALUE_TYPE;
			callNativeOpExp->argumentLoads.push_back(PReadableValue(lexNode->children[1]));
			currentExpectedReturnType = oldRetType;

			callNativeOpExp->functionPtrLoad = std::make_shared<ELoadConstPtr>(funcInfo.p_functionPtr);

			return callNativeOpExp;
		}

		Affirm(
			typeNameOperators.count(currentExpectedReturnType) != 0,
			"cannot perform binary math operation '%s' on operand of type '%s' at line %i",
			lexNode->token.text.c_str(), currentExpectedReturnType.c_str(), lexNode->token.line
		);

		OpCode opCode = typeNameOperators[currentExpectedReturnType][opTypeToOpIndex[lexNode->token.type]];

		Affirm(
			opCode != OpCode::INVALID,
			"cannot perform binary math operation '%s' on operand of type '%s' at line %i",
			lexNode->token.text.c_str(), currentExpectedReturnType.c_str(), lexNode->token.line
		);

		auto binMathOpExp = std::make_shared<EBinaryOp>(opCode);
		binMathOpExp->lhsLoad = lhsExp;

		std::string oldRetType = currentExpectedReturnType;
		if (currentExpectedReturnType == "ptr" ||
			lexNode->token.type == Token::Type::DoubleLeftArrow ||
			lexNode->token.type == Token::Type::DoubleRightArrow)
		{
			currentExpectedReturnType = "int";
		}

		binMathOpExp->rhsLoad = PReadableValue(lexNode->children[1]);

		currentExpectedReturnType = oldRetType;

		return binMathOpExp;
	}

	Parser::SharedExp Parser::PBinaryCompareOp(const SharedNode& lexNode, std::string& outReadDataType) 
	{
		static std::map<Token::Type, size_t> opTypeToOpIndex
		{
			{Token::Type::LeftArrow, 9},
			{Token::Type::RightArrow, 10},
			{Token::Type::DoubleEqualSign, 11},
			{Token::Type::LeftArrowEqualSign, 12},
			{Token::Type::RightArrowEqualSign, 13},
			{Token::Type::ExclamationMarkEqualSign, 14},
			{Token::Type::DoubleAmpersand, 15},
			{Token::Type::DoubleVerticalBar, 16}
		};

		AffirmCurrentType("char", lexNode->token.line);
		outReadDataType = "char";

		// determine operand data type
		currentExpectedReturnType = ANY_VALUE_TYPE;

		std::string readType;
		auto lhsExp = PReadableValue(lexNode->children[0], readType);

		currentExpectedReturnType = readType;

		if (typeNameToOpFuncs.count(currentExpectedReturnType) != 0 &&
			typeNameToOpFuncs[currentExpectedReturnType].count(lexNode->token.text) != 0)
		{
			const DataTypeFunctions& opFunctions = typeNameToOpFuncs[currentExpectedReturnType];
			const std::string& opName = lexNode->token.text;

			const FunctionInfo& funcInfo = opFunctions.at(opName);

			auto callOpExp = std::make_shared<ECallFunction>(funcInfo.parametersSize, funcInfo.localsSize);
			callOpExp->argumentLoads.push_back(lhsExp);
			callOpExp->argumentLoads.push_back(PReadableValue(lexNode->children[1]));
			callOpExp->functionIpLoad = std::make_shared<ELoadConstPtrToLabel>(currentExpectedReturnType + opName);

			currentExpectedReturnType = "char";

			return callOpExp;
		}
		if (typeNameToNativeOpFuncs.count(currentExpectedReturnType) != 0 &&
			typeNameToNativeOpFuncs[currentExpectedReturnType].count(lexNode->token.text) != 0)
		{
			const DataTypeNativeFunctions& opFunctions = typeNameToNativeOpFuncs[currentExpectedReturnType];
			const std::string& opName = lexNode->token.text;

			const NativeFunctionInfo& funcInfo = opFunctions.at(opName);

			auto callNativeOpExp = std::make_shared<ECallNativeFunction>();
			callNativeOpExp->argumentLoads.push_back(lhsExp);
			callNativeOpExp->argumentLoads.push_back(PReadableValue(lexNode->children[1]));
			callNativeOpExp->functionPtrLoad = std::make_shared<ELoadConstPtr>(funcInfo.p_functionPtr);

			currentExpectedReturnType = "char";

			return callNativeOpExp;
		}

		Affirm(
			typeNameOperators.count(currentExpectedReturnType) != 0,
			"cannot perform binary compare operation '%s' on operand of type '%s' at line %i",
			lexNode->token.text.c_str(), currentExpectedReturnType.c_str(), lexNode->token.line
		);

		OpCode opCode = typeNameOperators[currentExpectedReturnType][opTypeToOpIndex[lexNode->token.type]];

		Affirm(
			opCode != OpCode::INVALID,
			"cannot perform binary compare operation '%s' on operand of type '%s' at line %i",
			lexNode->token.text.c_str(), currentExpectedReturnType.c_str(), lexNode->token.line
		);

		auto binCompOpExp = std::make_shared<EBinaryOp>(opCode);
		binCompOpExp->lhsLoad = lhsExp;

		binCompOpExp->rhsLoad = PReadableValue(lexNode->children[1]);

		currentExpectedReturnType = "char";

		return binCompOpExp;
	}

	Parser::SharedExp Parser::PUnaryOp(const SharedNode& lexNode, std::string& outReadDataType)
	{
		switch (lexNode->token.type)
		{
		case Token::Type::Ampersand:
			return PReferenceValue(lexNode, outReadDataType);
		case Token::Type::Asterisk:
			return PDereferenceValue(lexNode, outReadDataType);
		case Token::Type::Minus:
			return PNegate(lexNode, outReadDataType);
		case Token::Type::ExclamationMark:
			return PNot(lexNode, outReadDataType);
		case Token::Type::Tilde:
			return PBitInvert(lexNode, outReadDataType);
		}

		return nullptr;
	}

	Parser::SharedExp Parser::PReferenceValue(const SharedNode& lexNode, std::string& outReadDataType)
	{
		outReadDataType = "ptr";

		if (lexNode->children[0]->type == LexNode::Type::Identifier)
		{
			std::string dataType;
			return PVariablePtr(lexNode->children[0], dataType);
		}
		if (lexNode->children[0]->type == LexNode::Type::MemberVariableAccess)
		{
			std::string dataType;
			return PMemberAccessPtr(lexNode->children[0], dataType);
		}

		Affirm(
			false,
			"cannot get the pointer of a potentially temporary value at line %i",
			lexNode->token.line
		);

		return nullptr;
	}

	Parser::SharedExp Parser::PDereferenceValue(const SharedNode& lexNode, std::string& outReadDataType)
	{
		Affirm(
			currentExpectedReturnType != ANY_VALUE_TYPE,
			"cannot dereference pointer when the expected value type is unknown at line %i",
			lexNode->token.line
		);

		outReadDataType = currentExpectedReturnType;

		auto loadBytes = std::make_shared<ELoadBytesFromPtr>(typeNameToSize[currentExpectedReturnType]);

		std::string oldType = currentExpectedReturnType;
		currentExpectedReturnType = "ptr";
		loadBytes->ptrLoad = PReadableValue(lexNode->children[0]);
		currentExpectedReturnType = oldType;

		return loadBytes;
	}

	Parser::SharedExp Parser::PNegate(const SharedNode& lexNode, std::string& outReadDataType)
	{
		const size_t opIndex = 17;
		const std::string opName = "negate";

		std::string readType;
		auto valExp = PReadableValue(lexNode->children[0], readType);

		if (currentExpectedReturnType == ANY_VALUE_TYPE)
			currentExpectedReturnType = readType;

		outReadDataType = currentExpectedReturnType;

		if (typeNameToOpFuncs.count(currentExpectedReturnType) != 0 &&
			typeNameToOpFuncs[currentExpectedReturnType].count(opName) != 0)
		{
			const DataTypeFunctions& opFunctions = typeNameToOpFuncs[currentExpectedReturnType];

			const FunctionInfo& funcInfo = opFunctions.at(opName);

			auto callOpExp = std::make_shared<ECallFunction>(funcInfo.parametersSize, funcInfo.localsSize);
			callOpExp->argumentLoads.push_back(valExp);
			callOpExp->functionIpLoad = std::make_shared<ELoadConstPtrToLabel>(currentExpectedReturnType + opName);

			return callOpExp;
		}
		if (typeNameToNativeOpFuncs.count(currentExpectedReturnType) != 0 &&
			typeNameToNativeOpFuncs[currentExpectedReturnType].count(opName) != 0)
		{
			const DataTypeNativeFunctions& opFunctions = typeNameToNativeOpFuncs[currentExpectedReturnType];

			const NativeFunctionInfo& funcInfo = opFunctions.at(opName);

			auto callNativeOpExp = std::make_shared<ECallNativeFunction>();
			callNativeOpExp->argumentLoads.push_back(valExp);
			callNativeOpExp->functionPtrLoad = std::make_shared<ELoadConstPtr>(funcInfo.p_functionPtr);

			return callNativeOpExp;
		}

		Affirm(
			typeNameOperators.count(currentExpectedReturnType) != 0,
			"cannot perform unary 'negate' on operand of type '%s' at line %i",
			lexNode->token.text.c_str(), currentExpectedReturnType.c_str(), lexNode->token.line
		);

		OpCode opCode = typeNameOperators[currentExpectedReturnType][opIndex];

		Affirm(
			opCode != OpCode::INVALID,
			"cannot perform unary 'negate' on operand of type '%s' at line %i",
			lexNode->token.text.c_str(), currentExpectedReturnType.c_str(), lexNode->token.line
		);

		auto unaryOpExp = std::make_shared<EUnaryOp>(opCode);
		unaryOpExp->valLoad = valExp;

		return unaryOpExp;
	}

	Parser::SharedExp Parser::PNot(const SharedNode& lexNode, std::string& outReadDataType)
	{
		AffirmCurrentType("char", lexNode->token.line);
		outReadDataType = "char";

		auto unaryNotExp = std::make_shared<EUnaryOp>(OpCode::Not);
		unaryNotExp->valLoad = PReadableValue(lexNode->children[0]);

		return unaryNotExp;
	}

	Parser::SharedExp Parser::PBitInvert(const SharedNode& lexNode, std::string& outReadDataType)
	{
		const size_t opIndex = 18;

		const std::string& retType = currentExpectedReturnType;
		outReadDataType = retType;

		Affirm(
			typeNameOperators.count(retType) != 0 &&
			typeNameOperators.at(retType)[opIndex] != OpCode::INVALID,
			"cannot perform unary 'bitwise invert' on operand of type '%s' at line %i",
			retType.c_str(), lexNode->token.line
		);

		OpCode opCode = typeNameOperators.at(retType)[opIndex];
		auto bitInvExp = std::make_shared<EUnaryOp>(opCode);

		bitInvExp->valLoad = PReadableValue(lexNode->children[0]);

		return bitInvExp;
	}

	Parser::SharedExp Parser::PFunctionCall(const SharedNode& lexNode, std::string& outReadDataType)
	{
		const std::string& funcName = lexNode->token.text;

		if (funcName == "sizeof")
		{
			Affirm(
				lexNode->children.size() == 1 &&
				lexNode->children[0]->type == LexNode::Type::Identifier,
				"'sizeof' at line %i expects 1 typename as argument",
				lexNode->children[0]->token.line
			);

			const std::string& typeName = lexNode->children[0]->token.text;

			Affirm(
				typeNameToSize.count(typeName) != 0,
				"'%s' at line %i is not a type name",
				typeName.c_str(), lexNode->children[0]->token.line
			);

			outReadDataType = "int";

			return std::make_shared<ELoadConstInt>(typeNameToSize.at(typeName));
		}

		if (typeNameToStructInfo.count(funcName) != 0)
			return PStructInitialization(lexNode, outReadDataType);

		if (ptrTypeNameToStructTypeName.count(funcName) != 0)
			return PStructPtrInitialization(lexNode, outReadDataType);

		return PUserOrNativeFunctionCall(lexNode, outReadDataType);
	}

	Parser::SharedExp Parser::PUserOrNativeFunctionCall(const SharedNode& lexNode, std::string& outReadDataType)
	{
		std::vector<std::string> argTypeNames;
		std::vector<SharedExp> argumentLoads;

		std::string oldRetType = currentExpectedReturnType;
		currentExpectedReturnType = ANY_VALUE_TYPE;

		for (size_t i = 0; i < lexNode->children.size(); i++)
		{
			std::string argTypeName;
			argumentLoads.push_back(PReadableValue(lexNode->children[i], argTypeName));

			argTypeNames.push_back(argTypeName);
		}

		currentExpectedReturnType = oldRetType;
		outReadDataType = currentExpectedReturnType;

		const std::string& funcName = lexNode->token.text;
		std::string funcHash = GetFunctionHash(currentExpectedReturnType, funcName, argTypeNames);

		if (hashToUserFunctions.count(funcHash) != 0)
		{
			const FunctionInfo& funcInfo = hashToUserFunctions.at(funcHash);
			auto callUserFuncExp = std::make_shared<ECallFunction>(funcInfo.parametersSize, funcInfo.localsSize);
			callUserFuncExp->argumentLoads = argumentLoads;
			callUserFuncExp->functionIpLoad = std::make_shared<ELoadConstPtrToLabel>(funcHash);

			return callUserFuncExp;
		}
		if (hashToNativeFunctions.count(funcHash) != 0)
		{
			const NativeFunctionInfo& funcInfo = hashToNativeFunctions.at(funcHash);
			auto callNativeFuncExp = std::make_shared<ECallNativeFunction>();
			callNativeFuncExp->argumentLoads = argumentLoads;
			callNativeFuncExp->functionPtrLoad = std::make_shared<ELoadConstPtr>(funcInfo.p_functionPtr);

			return callNativeFuncExp;
		}

		Affirm(
			false,
			"function signature '%s' at line %i does not match any existing functions",
			funcHash.c_str(), lexNode->token.line
		);

		return nullptr;
	}

	Parser::SharedExp Parser::PStructInitialization(const SharedNode& lexNode, std::string& outReadDataType)
	{
		const std::string& funcName = lexNode->token.text;
		AffirmCurrentType(funcName, lexNode->token.line);
		outReadDataType = funcName;

		const StructInfo& structInfo = typeNameToStructInfo[funcName];

		Affirm(
			structInfo.memberNameToVarInfo.size() == lexNode->children.size(),
			"number of arguments provided to struct initializer '%s' at line %i does not match number of properties",
			funcName.c_str(), lexNode->token.line
		);

		auto loadMultiExp = std::make_shared<ELoadMulti>();

		size_t argIndex = 0;
		std::string oldRetType = currentExpectedReturnType;

		for (const std::string& membName : structInfo.memberNames)
		{
			currentExpectedReturnType = structInfo.memberNameToVarInfo.at(membName).typeName;
			loadMultiExp->loaders.push_back(PReadableValue(lexNode->children[argIndex]));
			argIndex++;
		}

		currentExpectedReturnType = oldRetType;

		return loadMultiExp;
	}

	Parser::SharedExp Parser::PStructPtrInitialization(const SharedNode& lexNode, std::string& outReadDataType)
	{
		const std::string& funcName = lexNode->token.text;
		AffirmCurrentType(funcName, lexNode->token.line);
		outReadDataType = funcName;

		Affirm(
			lexNode->children.size() == 1,
			"expected 1 argument in struct pointer initializer '%s' at line %i",
			funcName.c_str(), lexNode->token.line
		);

		auto loadMultiExp = std::make_shared<ELoadMulti>();

		std::string oldRetType = currentExpectedReturnType;
		currentExpectedReturnType = "ptr";

		loadMultiExp->loaders.push_back(PReadableValue(lexNode->children[0]));

		currentExpectedReturnType = oldRetType;

		return loadMultiExp;
	}

	Parser::SharedExp Parser::PMemberFunctionCall(const SharedNode& lexNode, std::string& outReadDataType)
	{
		Affirm(
			p_currentFunction->varNameToVarInfo.count(lexNode->token.text) != 0,
			"undefined variable '%s' at line %i",
			lexNode->token.text.c_str(), lexNode->token.line
		);

		const VariableInfo& varInfo = p_currentFunction->varNameToVarInfo.at(lexNode->token.text);

		auto loadCallerPtrExp = std::make_shared<ELoadMulti>();
		std::string parentStructType = varInfo.typeName;

		// load variable pointer
		if (ptrTypeNameToStructTypeName.count(varInfo.typeName) != 0)
		{
			// variable is struct pointer
			parentStructType = ptrTypeNameToStructTypeName.at(varInfo.typeName);
			loadCallerPtrExp->loaders.push_back(std::make_shared<ELoadVariable>(varInfo.offset, static_cast<Int>(sizeof(Ptr))));
		}
		else
		{
			Affirm(
				typeNameToStructInfo.count(varInfo.typeName) != 0,
				"type '%s' at line %i is not a struct",
				varInfo.typeName.c_str(), lexNode->token.line
			);

			// variable is struct value
			loadCallerPtrExp->loaders.push_back(std::make_shared<ELoadVariablePtr>(varInfo.offset));
		}

		// traverse dot chain to next last to reach caller's pointer
		for (size_t i=0; i+1<lexNode->children.size(); i++)
		{
			const SharedNode& membNode = lexNode->children[i];

			Affirm(
				typeNameToStructInfo.count(parentStructType),
				"type '%s' at line %i is not a struct",
				parentStructType.c_str(), membNode->token.line
			);

			const StructInfo& parentStructInfo = typeNameToStructInfo.at(parentStructType);

			Affirm(
				parentStructInfo.memberNameToVarInfo.count(membNode->token.text) != 0,
				"'%s' at line %i is not a member of struct '%s'",
				membNode->token.text.c_str(), membNode->token.line, parentStructType.c_str()
			);

			const VariableInfo& membInfo = parentStructInfo.memberNameToVarInfo.at(membNode->token.text);

			if (ptrTypeNameToStructTypeName.count(membInfo.typeName) != 0)
			{
				// member is struct pointer
				parentStructType = ptrTypeNameToStructTypeName.at(membInfo.typeName);

				// push member variable address
				loadCallerPtrExp->loaders.push_back(std::make_shared<ELoadConstInt>(membInfo.offset));
				loadCallerPtrExp->loaders.push_back(std::make_shared<EPtrAdd>());
				// push pointer stored in member variable
				loadCallerPtrExp->loaders.push_back(std::make_shared<ELoadPtrFromStackTopPtr>());
			}
			else
			{
				// member is struct value
				parentStructType = membInfo.typeName;

				// push member variable address
				loadCallerPtrExp->loaders.push_back(std::make_shared<ELoadConstInt>(membInfo.offset));
				loadCallerPtrExp->loaders.push_back(std::make_shared<EPtrAdd>());
			}
		}

		const SharedNode& funcNode = lexNode->children.back();
		const std::string& funcName = funcNode->token.text;

		Affirm(
			typeNameToMemberFunctions.count(parentStructType) != 0 &&
			typeNameToMemberFunctions.at(parentStructType).count(funcName) != 0,
			"type '%s' at line %i does not have member function '%s'",
			parentStructType.c_str(), funcNode->token.line, funcNode->token.text.c_str()
		);

		const FunctionInfo& funcInfo = typeNameToMemberFunctions.at(parentStructType).at(funcName);

		AffirmCurrentType(funcInfo.returnTypeName, funcNode->token.line);
		outReadDataType = funcInfo.returnTypeName;

		auto callMembFuncExp = std::make_shared<ECallFunction>(funcInfo.parametersSize, funcInfo.localsSize);
		std::string funcLabel = parentStructType + "::" + funcName;
		callMembFuncExp->functionIpLoad = std::make_shared<ELoadConstPtrToLabel>(funcLabel);

		Affirm(
			funcInfo.parameterNames.size() == funcNode->children.size(),
			"argument count in member function call att line %i does not match parameter count",
			funcNode->token.line
		);

		// add "this" struct pointer
		callMembFuncExp->argumentLoads.push_back(loadCallerPtrExp);

		std::string oldRetType = currentExpectedReturnType;

		for (size_t i = 0; i < funcInfo.parameterNames.size(); i++)
		{
			const VariableInfo& varInfo = funcInfo.varNameToVarInfo.at(funcInfo.parameterNames[i]);
			currentExpectedReturnType = varInfo.typeName;

			callMembFuncExp->argumentLoads.push_back(PReadableValue(funcNode->children[i]));
		}

		currentExpectedReturnType = oldRetType;

		return callMembFuncExp;
	}
}