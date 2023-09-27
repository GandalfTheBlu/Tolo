#include "parser.h"
#include <utility>

#define ANY_VALUE_TYPE "__any__"

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
		functionPtr(0)
	{}

	StructInfo::StructInfo()
	{}

	Parser::Parser() :
		currentFunction(nullptr)
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
			OpCode::Char_Less,
			OpCode::Char_Greater,
			OpCode::Char_Equal,
			OpCode::Char_Negate
		};

		typeNameOperators["int"] =
		{
			OpCode::Int_Add,
			OpCode::Int_Sub,
			OpCode::Int_Mul,
			OpCode::Int_Div,
			OpCode::Int_Less,
			OpCode::Int_Greater,
			OpCode::Int_Equal,
			OpCode::Int_Negate
		};

		typeNameOperators["float"] =
		{
			OpCode::Float_Add,
			OpCode::Float_Sub,
			OpCode::Float_Mul,
			OpCode::Float_Div,
			OpCode::Float_Less,
			OpCode::Float_Greater,
			OpCode::Float_Equal,
			OpCode::Float_Negate
		};

		typeNameOperators["ptr"] =
		{
			OpCode::Ptr_Add,
			OpCode::Ptr_Sub,
			OpCode::INVALID,
			OpCode::INVALID,
			OpCode::INVALID,
			OpCode::INVALID,
			OpCode::Ptr_Equal,
			OpCode::INVALID
		};

		currentExpectedReturnType = "void";
	}

	bool Parser::HasBody(LexNode* p_lexNode, Int& outBodyStartIndex, Int& outBodyEndIndex)
	{
		switch (p_lexNode->type)
		{
		case LexNode::Type::While:
		case LexNode::Type::IfSingle:
		case LexNode::Type::ElseIfSingle:
			outBodyStartIndex = 1;
			outBodyEndIndex = (Int)p_lexNode->children.size() - 1;
			break;
		case LexNode::Type::IfChain:
		case LexNode::Type::ElseIfChain:
			outBodyStartIndex = 1;
			outBodyEndIndex = (Int)p_lexNode->children.size() - 2;
			break;
		case LexNode::Type::Else:
			outBodyStartIndex = 0;
			outBodyStartIndex = 0;
			break;
		default:
			return false;
		}

		return true;
	}

	void Parser::FlattenNode(LexNode* p_lexNode, std::vector<LexNode*>& outNodes)
	{
		Int bodyStartIndex = 0;
		Int bodyEndIndex = 0;
		if (!HasBody(p_lexNode, bodyStartIndex, bodyEndIndex))
		{
			outNodes.push_back(p_lexNode);
			return;
		}

		for (Int i = bodyStartIndex; i <= bodyEndIndex; i++)
		{
			LexNode* p_bodyNode = p_lexNode->children[i];
			FlattenNode(p_bodyNode, outNodes);
		}
	}

	Expression* Parser::ParseLiteralConstant(LexNode* p_lexNode)
	{
		if (p_lexNode->token.type == Token::Type::ConstChar)
		{
			Char value = p_lexNode->token.text[0];

			Affirm(
				currentExpectedReturnType == "char" || currentExpectedReturnType == ANY_VALUE_TYPE,
				"expected an expression of type '%s' but got 'char' (%c) at line %i",
				currentExpectedReturnType.c_str(), value, p_lexNode->token.line
			);

			return new ELoadConstChar(value);
		}
		if (p_lexNode->token.type == Token::Type::ConstInt)
		{
			Int value = std::stoi(p_lexNode->token.text);

			Affirm(
				currentExpectedReturnType == "int" || currentExpectedReturnType == ANY_VALUE_TYPE,
				"expected an expression of type '%s' but got 'int' (%i) at line %i",
				currentExpectedReturnType.c_str(), value, p_lexNode->token.line
			);

			return new ELoadConstInt(value);
		}
		if (p_lexNode->token.type == Token::Type::ConstFloat)
		{
			Float value = std::stof(p_lexNode->token.text);

			Affirm(
				currentExpectedReturnType == "float" || currentExpectedReturnType == ANY_VALUE_TYPE,
				"expected an expression of type '%s' but got 'float' (%f) at line %i",
				currentExpectedReturnType.c_str(), value, p_lexNode->token.line
			);

			return new ELoadConstFloat(value);
		}
	}

	Expression* Parser::ParseVariableLoad(LexNode* p_lexNode)
	{
		const std::string& varName = p_lexNode->token.text;

		Affirm(
			currentFunction->varNameToVarInfo.count(varName) != 0,
			"undefined name '%s' at line %i",
			varName.c_str(), p_lexNode->token.line
		);

		const VariableInfo& info = currentFunction->varNameToVarInfo[varName];

		Affirm(
			info.typeName == currentExpectedReturnType || currentExpectedReturnType == ANY_VALUE_TYPE,
			"expected '%s' to be of type '%s' at line %i",
			varName.c_str(), currentExpectedReturnType.c_str(), p_lexNode->token.line
		);

		return new ELoadVariable(info.offset, typeNameToSize[info.typeName], info.typeName);
	}

	Expression* Parser::ParseVariableWrite(LexNode* p_lexNode)
	{
		const std::string& varName = p_lexNode->token.text;

		Affirm(
			currentFunction->varNameToVarInfo.count(varName) != 0,
			"undefined name '%s' at line %i",
			varName.c_str(), p_lexNode->token.line
		);

		const VariableInfo& info = currentFunction->varNameToVarInfo[varName];

		EWriteBytesTo* p_write = new EWriteBytesTo();
		p_write->bytesSizeLoad = new ELoadConstInt(typeNameToSize[info.typeName]);
		p_write->writePtrLoad = new ELoadVariablePtr(info.offset);

		currentExpectedReturnType = info.typeName;

		p_write->dataLoad = ParseNextExpression(p_lexNode->children[0]);

		currentExpectedReturnType = "void";

		return p_write;
	}

	Expression* Parser::ParsePropertyLoad(LexNode* p_lexNode)
	{
		const std::string& varName = p_lexNode->token.text;

		Affirm(
			currentFunction->varNameToVarInfo.count(varName) != 0,
			"undefined name '%s' at line %i",
			varName.c_str(), p_lexNode->token.line
		);

		VariableInfo currentVarInfo = currentFunction->varNameToVarInfo[varName];
		Int propOffset = currentVarInfo.offset;

		for (size_t i = 0; i < p_lexNode->children.size(); i++)
		{
			const std::string& propName = p_lexNode->children[i]->token.text;

			Affirm(
				typeNameToStructInfo.count(currentVarInfo.typeName) != 0,
				"cannot get property '%s' at line %i because preceeding expression is not a struct",
				propName.c_str(), p_lexNode->children[i]->token.line
			);

			StructInfo& structInfo = typeNameToStructInfo[currentVarInfo.typeName];

			Affirm(structInfo.propNameToVarInfo.count(propName) != 0,
				"cannot access property '%s' at line %i because the struct '%s' does not contain it",
				propName.c_str(), p_lexNode->children[0]->token.line, currentVarInfo.typeName.c_str()
			);

			VariableInfo& propInfo = structInfo.propNameToVarInfo[propName];
			propOffset -= propInfo.offset;

			currentVarInfo = propInfo;
		}

		Affirm(
			currentVarInfo.typeName == currentExpectedReturnType || currentExpectedReturnType == ANY_VALUE_TYPE,
			"expected expression of type '%s' at line %i",
			currentExpectedReturnType.c_str(), p_lexNode->token.line
		);
		
		return new ELoadVariable(propOffset, typeNameToSize[currentVarInfo.typeName], currentVarInfo.typeName);
	}

	Expression* Parser::ParsePropertyWrite(LexNode* p_lexNode)
	{
		const std::string& varName = p_lexNode->token.text;

		Affirm(
			currentFunction->varNameToVarInfo.count(varName) != 0,
			"undefined name '%s' at line %i",
			varName.c_str(), p_lexNode->token.line
		);

		VariableInfo currentVarInfo = currentFunction->varNameToVarInfo[varName];
		Int propOffset = currentVarInfo.offset;

		for (size_t i = 0; i+1 < p_lexNode->children.size(); i++)
		{
			const std::string& propName = p_lexNode->children[i]->token.text;

			Affirm(
				typeNameToStructInfo.count(currentVarInfo.typeName) != 0,
				"cannot get property '%s' at line %i because preceeding expression is not a struct",
				propName.c_str(), p_lexNode->children[i]->token.line
			);

			StructInfo& structInfo = typeNameToStructInfo[currentVarInfo.typeName];

			Affirm(structInfo.propNameToVarInfo.count(propName) != 0,
				"cannot access property '%s' at line %i because the struct '%s' does not contain it",
				propName.c_str(), p_lexNode->children[0]->token.line, currentVarInfo.typeName.c_str()
			);

			VariableInfo& propInfo = structInfo.propNameToVarInfo[propName];
			propOffset -= propInfo.offset;

			currentVarInfo = propInfo;
		}

		EWriteBytesTo* p_write = new EWriteBytesTo();
		p_write->bytesSizeLoad = new ELoadConstInt(typeNameToSize[currentVarInfo.typeName]);
		p_write->writePtrLoad = new ELoadVariablePtr(propOffset);

		currentExpectedReturnType = currentVarInfo.typeName;

		p_write->dataLoad = ParseNextExpression(p_lexNode->children.back());

		currentExpectedReturnType = "void";

		return p_write;
	}

	Expression* Parser::ParseReturn(LexNode* p_lexNode)
	{
		EReturn* p_ret = new EReturn(typeNameToSize[currentFunction->returnTypeName]);

		if (p_lexNode->children.size() == 0)
		{
			Affirm(
				currentFunction->returnTypeName == "void",
				"missing value expression after 'return' keyword at line %i", 
				p_lexNode->token.line
			);
		}
		else
		{
			currentExpectedReturnType = currentFunction->returnTypeName;
			p_ret->retValLoad = ParseNextExpression(p_lexNode->children[0]);
			currentExpectedReturnType = "void";
		}

		return p_ret;
	}


	Expression* Parser::ParseIfSingle(LexNode* p_lexNode)
	{
		EIfSingle* p_if = new EIfSingle();

		currentExpectedReturnType = "char";
		p_if->conditionLoad = ParseNextExpression(p_lexNode->children[0]);
		currentExpectedReturnType = "void";

		for (size_t i = 1; i < p_lexNode->children.size(); i++)
		{
			p_if->body.push_back(ParseNextExpression(p_lexNode->children[i]));
		}

		return p_if;
	}

	Expression* Parser::ParseIfChain(LexNode* p_lexNode)
	{
		EIfChain* p_if = new EIfChain();

		currentExpectedReturnType = "char";
		p_if->conditionLoad = ParseNextExpression(p_lexNode->children[0]);
		currentExpectedReturnType = "void";

		for (size_t i = 1; i+1 < p_lexNode->children.size(); i++)
		{
			p_if->body.push_back(ParseNextExpression(p_lexNode->children[i]));
		}

		p_if->chain = ParseNextExpression(p_lexNode->children.back());

		return p_if;
	}

	Expression* Parser::ParseElseIfSingle(LexNode* p_lexNode)
	{
		EElseIfSingle* p_elif = new EElseIfSingle();

		currentExpectedReturnType = "char";
		p_elif->conditionLoad = ParseNextExpression(p_lexNode->children[0]);
		currentExpectedReturnType = "void";

		for (size_t i = 1; i < p_lexNode->children.size(); i++)
		{
			p_elif->body.push_back(ParseNextExpression(p_lexNode->children[i]));
		}

		return p_elif;
	}

	Expression* Parser::ParseElseIfChain(LexNode* p_lexNode)
	{
		EElseIfChain* p_elif = new EElseIfChain();

		currentExpectedReturnType = "char";
		p_elif->conditionLoad = ParseNextExpression(p_lexNode->children[0]);
		currentExpectedReturnType = "void";

		for (size_t i = 1; i + 1 < p_lexNode->children.size(); i++)
		{
			p_elif->body.push_back(ParseNextExpression(p_lexNode->children[i]));
		}

		p_elif->chain = ParseNextExpression(p_lexNode->children.back());

		return p_elif;
	}

	Expression* Parser::ParseElse(LexNode* p_lexNode)
	{
		EElse* p_else = new EElse();

		for (size_t i = 0; i < p_lexNode->children.size(); i++)
		{
			p_else->body.push_back(ParseNextExpression(p_lexNode->children[i]));
		}

		return p_else;
	}

	Expression* Parser::ParseWhile(LexNode* p_lexNode)
	{
		EWhile* p_while = new EWhile();

		currentExpectedReturnType = "char";
		p_while->conditionLoad = ParseNextExpression(p_lexNode->children[0]);
		currentExpectedReturnType = "void";

		for (size_t i = 1; i < p_lexNode->children.size(); i++)
		{
			p_while->body.push_back(ParseNextExpression(p_lexNode->children[i]));
		}

		return p_while;
	}

	Expression* Parser::ParseBinaryMathOp(LexNode* p_lexNode)
	{
		static std::map<Token::Type, size_t> opTypeToOpIndex
		{
			{Token::Type::Plus, 0},
			{Token::Type::Minus, 1},
			{Token::Type::Asterisk, 2},
			{Token::Type::ForwardSlash, 3}
		};

		// determine op code based on function and operand type
		OpCode opCode = OpCode::Char_Add;
		bool anyValueType = false;

		if (currentExpectedReturnType == ANY_VALUE_TYPE)
			anyValueType = true;
		else if (typeNameOperators.count(currentExpectedReturnType) == 0)
			Affirm(false, "expected expression of type '%s' at line %i", currentExpectedReturnType.c_str(), p_lexNode->token.line);

		Expression* p_lhs = ParseNextExpression(p_lexNode->children[0]);
		if (anyValueType)
		{
			currentExpectedReturnType = p_lhs->GetDataType();
			Affirm(
				typeNameOperators.count(currentExpectedReturnType) != 0,
				"cannot perform math operation '%s' on operand of type '%s' at line %i",
				p_lexNode->token.text.c_str(), currentExpectedReturnType.c_str(), p_lexNode->token.line
			);
		}

		opCode = typeNameOperators[currentExpectedReturnType][opTypeToOpIndex[p_lexNode->token.type]];

		Affirm(
			opCode != OpCode::INVALID,
			"cannot perform math operation '%s' on operand of type '%s' at line %i",
			p_lexNode->token.text.c_str(), currentExpectedReturnType.c_str(), p_lexNode->token.line
		);

		EBinaryOp* p_binMathOp = new EBinaryOp(opCode);
		p_binMathOp->lhsLoad = p_lhs;

		std::string oldRetType = currentExpectedReturnType;
		if (currentExpectedReturnType == "ptr")
			currentExpectedReturnType = "int";

		p_binMathOp->rhsLoad = ParseNextExpression(p_lexNode->children[1]);

		currentExpectedReturnType = oldRetType;

		return p_binMathOp;
	}

	Expression* Parser::ParseBinaryCompareOp(LexNode* p_lexNode)
	{
		static std::map<Token::Type, size_t> opTypeToOpIndex
		{
			{Token::Type::LeftArrow, 4},
			{Token::Type::RightArrow, 5}
			//{"equal", 6}
		};

		if (currentExpectedReturnType != "char" && currentExpectedReturnType != ANY_VALUE_TYPE)
			Affirm(false, "expected expression of type '%s' at line %i", currentExpectedReturnType.c_str(), p_lexNode->token.line);


		// determine operand data type
		currentExpectedReturnType = ANY_VALUE_TYPE;

		Expression* p_lhs = ParseNextExpression(p_lexNode->children[0]);

		currentExpectedReturnType = p_lhs->GetDataType();
		Affirm(
			typeNameOperators.count(currentExpectedReturnType) != 0,
			"cannot perform binary compare operation '%s' on operand of type '%s' at line %i",
			p_lexNode->token.text.c_str(), currentExpectedReturnType.c_str(), p_lexNode->token.line
		);

		OpCode opCode = typeNameOperators[currentExpectedReturnType][opTypeToOpIndex[p_lexNode->token.type]];

		Affirm(
			opCode != OpCode::INVALID,
			"cannot perform binary operator '%s' on operand of type '%s' at line %i",
			p_lexNode->token.text.c_str(), currentExpectedReturnType.c_str(), p_lexNode->token.line
		);

		EBinaryOp* p_binCompOp = new EBinaryOp(opCode);
		p_binCompOp->lhsLoad = p_lhs;

		p_binCompOp->rhsLoad = ParseNextExpression(p_lexNode->children[1]);

		currentExpectedReturnType = "char";

		return p_binCompOp;
	}

	Expression* Parser::ParseBinaryOp(LexNode* p_lexNode)
	{
		switch (p_lexNode->token.type)
		{
		case Token::Type::LeftArrow:
		case Token::Type::RightArrow:
			return ParseBinaryCompareOp(p_lexNode);
		}
		
		return ParseBinaryMathOp(p_lexNode);
	}

	Expression* Parser::ParseUnaryOp(LexNode* p_lexNode)
	{
		static std::map<Token::Type, size_t> opTypeToOpIndex
		{
			{Token::Type::Minus, 7}
		};

		// determine op code based on function and operand type
		OpCode opCode = OpCode::Char_Negate;
		bool anyValueType = false;

		if (currentExpectedReturnType == ANY_VALUE_TYPE)
			anyValueType = true;
		else if (typeNameOperators.count(currentExpectedReturnType) == 0)
			Affirm(false, "expected expression of type '%s' at line %i", currentExpectedReturnType.c_str(), p_lexNode->token.line);

		Expression* p_val = ParseNextExpression(p_lexNode->children[0]);
		if (anyValueType)
		{
			currentExpectedReturnType = p_val->GetDataType();
			Affirm(
				typeNameOperators.count(currentExpectedReturnType) != 0,
				"cannot perform math operation '%s' on operand of type '%s' at line %i",
				p_lexNode->token.text.c_str(), currentExpectedReturnType.c_str(), p_lexNode->token.line
			);
		}

		opCode = typeNameOperators[currentExpectedReturnType][opTypeToOpIndex[p_lexNode->token.type]];

		Affirm(
			opCode != OpCode::INVALID,
			"cannot perform unary operation '%s' on operand of type '%s' at line %i",
			p_lexNode->token.text.c_str(), currentExpectedReturnType.c_str(), p_lexNode->token.line
		);

		EUnaryOp* p_unaryOp = new EUnaryOp(opCode);
		p_unaryOp->valLoad = p_val;

		return p_unaryOp;
	}

	Expression* Parser::ParseUserFunctionCall(LexNode* p_lexNode)
	{
		const std::string& funcName = p_lexNode->token.text;

		// handle struct initialization
		if (typeNameToStructInfo.count(funcName) != 0)
		{
			std::string oldRetType = currentExpectedReturnType;
			Affirm(
				oldRetType == funcName || oldRetType == ANY_VALUE_TYPE,
				"expected expression of type '%s' at line %i but got '%s'",
				oldRetType.c_str(), p_lexNode->token.line, funcName.c_str()
			);

			StructInfo& structInfo = typeNameToStructInfo[funcName];
				
			Affirm(
				structInfo.propNameToVarInfo.size() == p_lexNode->children.size(),
				"number of arguments provided to struct initializer '%s' at line %i does not match number of properties",
				funcName.c_str(), p_lexNode->token.line
			);

			ELoadMulti* p_loadMulti = new ELoadMulti(funcName);

			size_t argIndex = 0;
			for (const std::string& propName : structInfo.propNames)
			{
				currentExpectedReturnType = structInfo.propNameToVarInfo[propName].typeName;
				p_loadMulti->loaders.push_back(ParseNextExpression(p_lexNode->children[argIndex]));
				argIndex++;
			}

			currentExpectedReturnType = oldRetType;

			return p_loadMulti;
		}

		Affirm(
			userFunctions.count(funcName) != 0,
			"name '%s' at line %i is not a function",
			funcName.c_str(), p_lexNode->token.line
		);

		FunctionInfo& info = userFunctions[funcName];

		ECallFunction* p_call = new ECallFunction(info.parametersSize, info.localsSize, info.returnTypeName);
		p_call->functionIpLoad = new ELoadConstPtrToLabel(funcName);

		std::string oldRetType = currentExpectedReturnType;
		Affirm(
			oldRetType == info.returnTypeName || oldRetType == ANY_VALUE_TYPE,
			"expected expression of type '%s' at line %i but got '%s'",
			oldRetType.c_str(), p_lexNode->token.line, info.returnTypeName.c_str()
		);

		Affirm(
			info.parameterNames.size() == p_lexNode->children.size(),
			"argument count in function call att line %i does not match parameter count",
			p_lexNode->token.line
		);

		for (size_t i = 0; i < info.parameterNames.size(); i++)
		{
			const VariableInfo& varInfo = info.varNameToVarInfo[info.parameterNames[i]];
			currentExpectedReturnType = varInfo.typeName;

			p_call->argumentLoads.push_back(ParseNextExpression(p_lexNode->children[i]));
		}

		currentExpectedReturnType = oldRetType;

		return p_call;
	}

	Expression* Parser::ParseNativeFunctionCall(LexNode* p_lexNode)
	{
		const std::string& funcName = p_lexNode->token.text;

		Affirm(
			nativeFunctions.count(funcName) != 0, 
			"native function '%s' at line %i is not defined", 
			funcName.c_str(), p_lexNode->token.line
		);

		NativeFunctionInfo& info = nativeFunctions[funcName];

		ECallNativeFunction* p_call = new ECallNativeFunction(info.returnTypeName);
		p_call->functionPtrLoad = new ELoadConstPtr(info.functionPtr);

		std::string oldRetType = currentExpectedReturnType;
		Affirm(
			oldRetType == info.returnTypeName || oldRetType == ANY_VALUE_TYPE,
			"expected expression of type '%s' at line %i but got '%s'",
			oldRetType.c_str(), p_lexNode->token.line, info.returnTypeName.c_str()
		);

		Affirm(
			info.parameterTypeNames.size() == p_lexNode->children.size(),
			"argument count in function call att line %i does not match parameter count",
			p_lexNode->token.line
		);

		for (size_t i = 0; i < info.parameterTypeNames.size(); i++)
		{
			currentExpectedReturnType = info.parameterTypeNames[i];
			p_call->argumentLoads.push_back(ParseNextExpression(p_lexNode->children[i]));
		}

		currentExpectedReturnType = oldRetType;

		return p_call;
	}

	Expression* Parser::ParseVariableDefinition(LexNode* p_lexNode)
	{
		const std::string& varTypeName = p_lexNode->token.text;
		const std::string& varName = p_lexNode->children[0]->token.text;

		Affirm(
			currentFunction->varNameToVarInfo.count(varName) != 0,
			"undefined name '%s' at line %i",
			varName.c_str(), p_lexNode->token.line
		);

		const VariableInfo& info = currentFunction->varNameToVarInfo[varName];

		EWriteBytesTo* p_write = new EWriteBytesTo();
		p_write->bytesSizeLoad = new ELoadConstInt(typeNameToSize[info.typeName]);
		p_write->writePtrLoad = new ELoadVariablePtr(info.offset);

		currentExpectedReturnType = info.typeName;

		p_write->dataLoad = ParseNextExpression(p_lexNode->children[1]);

		currentExpectedReturnType = "void";

		return p_write;
	}

	Expression* Parser::ParseFunctionDefinition(LexNode* p_lexNode)
	{
		const std::string& returnTypeName = p_lexNode->token.text;
		Affirm(
			typeNameToSize.count(returnTypeName) != 0,
			"undefined type '%s' at line %i",
			returnTypeName.c_str(), p_lexNode->token.line
		);

		const std::string& funcName = p_lexNode->children[0]->token.text;
		Affirm(
			userFunctions.count(funcName) == 0 && nativeFunctions.count(funcName) == 0,
			"name '%s' at line %i is already a defined function",
			funcName.c_str(), p_lexNode->children[0]->token.line
		);

		EDefineFunction* p_defFunc = new EDefineFunction(funcName);
		FunctionInfo& funcInfo = userFunctions[funcName];
		funcInfo.returnTypeName = returnTypeName;
		Int nextVarOffset = 0;

		// find body start
		size_t bodyStartIndex = 1;
		for (; bodyStartIndex < p_lexNode->children.size(); bodyStartIndex++)
		{
			if (p_lexNode->children[bodyStartIndex]->type != LexNode::Type::Identifier)
				break;
		}

		// flatten content nodes into a single array to find all variable definitions
		std::vector<LexNode*> bodyContent;
		for (size_t i = bodyStartIndex; i < p_lexNode->children.size(); i++)
		{
			FlattenNode(p_lexNode->children[i], bodyContent);
		}

		// find all local variable definitions
		for (auto p_bodyNode : bodyContent)
		{
			if (p_bodyNode->type != LexNode::Type::VariableDefinition)
				continue;

			const std::string& varTypeName = p_bodyNode->token.text;
			const std::string& varName = p_bodyNode->children[0]->token.text;

			Affirm(
				typeNameToSize.count(varTypeName) != 0,
				"undefined type '%s' at line %i",
				varTypeName.c_str(), p_bodyNode->token.line
			);

			Affirm(
				funcInfo.varNameToVarInfo.count(varName) == 0,
				"variable '%s' at line %i is already defined",
				varName.c_str(), p_bodyNode->children[0]->token.line
			);

			Int varSize = typeNameToSize[varTypeName];
			nextVarOffset += varSize;
			funcInfo.localsSize += varSize;
			funcInfo.varNameToVarInfo[varName] = { varTypeName, nextVarOffset };
		}

		// find all parameters
		for (size_t i = 1; i + 1 < bodyStartIndex; i += 2)
		{
			const std::string& varTypeName = p_lexNode->children[i]->token.text;
			const std::string& varName = p_lexNode->children[i + 1]->token.text;

			Affirm(
				typeNameToSize.count(varTypeName) != 0,
				"undefined type '%s' at line %i",
				varTypeName.c_str(), p_lexNode->children[i]->token.line
			);

			Affirm(
				funcInfo.varNameToVarInfo.count(varName) == 0,
				"variable '%s' at line %i is already defined",
				varName.c_str(), p_lexNode->children[i + 1]->token.line
			);

			Int varSize = typeNameToSize[varTypeName];
			nextVarOffset += varSize;
			funcInfo.parametersSize += varSize;
			funcInfo.varNameToVarInfo[varName] = { varTypeName, nextVarOffset };
			funcInfo.parameterNames.push_back(varName);
		}

		currentFunction = &funcInfo;

		// parse body content
		for (size_t i = bodyStartIndex; i < p_lexNode->children.size(); i++)
			p_defFunc->body.push_back(ParseNextExpression(p_lexNode->children[i]));

		currentFunction = nullptr;

		// check for final return expression
		if (p_defFunc->body.size() == 0)
		{
			Affirm(funcInfo.returnTypeName == "void",
				"missing 'return'-statement in function '%s' at line %i",
				funcName.c_str(), p_lexNode->token.line
			);

			p_defFunc->body.push_back(new EReturn(0));
		}
		else if(p_lexNode->children.back()->type != LexNode::Type::Return)
		{
			Affirm(p_defFunc->body.back()->GetDataType() == "void",
				"missing 'return'-statement in function '%s' at line %i",
				funcName.c_str(), p_lexNode->token.line
			);

			p_defFunc->body.push_back(new EReturn(0));
		}

		return p_defFunc;
	}

	Expression* Parser::ParseStructDefinition(LexNode* p_lexNode)
	{
		const std::string& structName = p_lexNode->children[0]->token.text;

		Affirm(
			typeNameToSize.contains(structName) == 0,
			"type name '%s' at line %i is already defined", 
			structName.c_str(), p_lexNode->children[0]->token.line
		);

		StructInfo& structInfo = typeNameToStructInfo[structName];

		Int propertyOffset = 0;
		for (size_t i = 1; i + 1 < p_lexNode->children.size(); i += 2)
		{
			const std::string& propTypeName = p_lexNode->children[i]->token.text;
			const std::string& propName = p_lexNode->children[i + 1]->token.text;

			Affirm(
				propTypeName != structName,
				"struct cannot contain itself, line %i",
				p_lexNode->children[i]->token.line
			);

			Affirm(
				typeNameToSize.contains(propTypeName) != 0,
				"type name '%s' at line %i is not defined",
				propTypeName.c_str(), p_lexNode->children[i]->token.line
			);

			Affirm(
				structInfo.propNameToVarInfo.count(propName) == 0,
				"property '%s' at line %i is already defined in struct '%s'",
				propName.c_str(), p_lexNode->children[i + 1]->token.line, structName.c_str()
			);

			structInfo.propNameToVarInfo[propName] = VariableInfo(propTypeName, propertyOffset);
			structInfo.propNames.push_back(propName);
			propertyOffset += typeNameToSize[propTypeName];
		}

		typeNameToSize[structName] = propertyOffset;

		return new EEmpty();
	}

	Expression* Parser::ParseNextExpression(LexNode* p_lexNode)
	{
		switch (p_lexNode->type)
		{
		case LexNode::Type::LiteralConstant:
			return ParseLiteralConstant(p_lexNode);
		case LexNode::Type::VariableLoad:
			return ParseVariableLoad(p_lexNode);
		case LexNode::Type::VariableWrite:
			return ParseVariableWrite(p_lexNode);
		case LexNode::Type::PropertyLoad:
			return ParsePropertyLoad(p_lexNode);
		case LexNode::Type::PropertyWrite:
			return ParsePropertyWrite(p_lexNode);
		case LexNode::Type::VariableDefinition:
			return ParseVariableDefinition(p_lexNode);
		case LexNode::Type::FunctionDefinition:
			return ParseFunctionDefinition(p_lexNode);
		case LexNode::Type::StructDefinition:
			return ParseStructDefinition(p_lexNode);
		case LexNode::Type::UserFunctionCall:
			return ParseUserFunctionCall(p_lexNode);
		case LexNode::Type::NativeFunctionCall:
			return ParseNativeFunctionCall(p_lexNode);
		case LexNode::Type::BinaryOperation:
			return ParseBinaryOp(p_lexNode);
		case LexNode::Type::UnaryOperation:
			return ParseUnaryOp(p_lexNode);
		case LexNode::Type::Parenthesis:
			return ParseNextExpression(p_lexNode->children[0]);
		case LexNode::Type::Return:
			return ParseReturn(p_lexNode);
		case LexNode::Type::IfSingle:
			return ParseIfSingle(p_lexNode);
		case LexNode::Type::IfChain:
			return ParseIfChain(p_lexNode);
		case LexNode::Type::ElseIfSingle:
			return ParseElseIfSingle(p_lexNode);
		case LexNode::Type::ElseIfChain:
			return ParseElseIfChain(p_lexNode);
		case LexNode::Type::Else:
			return ParseElse(p_lexNode);
		case LexNode::Type::While:
			return ParseWhile(p_lexNode);
		case LexNode::Type::Break:
			return new EBreak();
		case LexNode::Type::Continue:
			return new EContinue();
		default:
			Affirm(false, "unexpected expression '%s' at line %i", p_lexNode->token.text.c_str(), p_lexNode->token.line);
			break;
		}

		return nullptr;
	}

	void Parser::Parse(std::vector<LexNode*>& lexNodes, std::vector<Expression*>& expressions)
	{
		for (auto e : lexNodes)
			expressions.push_back(ParseNextExpression(e));
	}
}