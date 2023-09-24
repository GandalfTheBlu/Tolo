#include "lex_node.h"

namespace Tolo
{
	LexNode::LexNode(Type _type, const Token& _token) :
		type(_type),
		token(_token)
	{}

	LexNode::~LexNode()
	{
		for (auto e : children)
			delete e;
	}

	bool LexNode::IsValueExpression()
	{
		return type == Type::LiteralConstant ||
			type == Type::VariableLoad ||
			type == Type::PropertyLoad ||
			type == Type::CoreFunctionCall ||
			type == Type::NativeFunctionCall ||
			type == Type::UserFunctionCall;
	}

	bool LexNode::IsValidExpressionInScope()
	{
		return type != Type::FunctionDefinition && type != Type::Identifier && type != Type::StructDefinition;
	}
}