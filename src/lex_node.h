#pragma once
#include "token.h"
#include <vector>
#include <memory>

namespace Tolo
{
	struct LexNode
	{
		enum class Type
		{
			INVALID,

			// statements
			Return,
			IfSingle,// ifSingle{cond, body[...]}
			IfChain,// ifChain{cond, body[...], elseIfSingle | elseIfChain | else}
			ElseIfSingle,// elseIfSignle{cond, body[...]}
			ElseIfChain,// elseIfChain{cond, body[...], elseIfSingle | elseIfChain | else}
			Else,// else{body[...]}
			While,
			Break,
			Continue,
			Goto,
			Scope,

			// values
			LiteralConstant,
			Identifier,
			UnaryOperation,
			BinaryOperation,
			Parenthesis,
			FunctionCall,
			MemberVariableAccess,
			MemberFunctionCall,

			// definitions
			StructDefinition,
			StructDefinitionInheritance,
			VariableDefinition,
			FunctionDefinition,
			OperatorDefinition,
			MemberFunctionDefinition,
			MemberFunctionDefinitionVirtual,
			EnumDefinition
		};

		Type type;
		Token token;
		std::vector<std::shared_ptr<LexNode>> children;

		LexNode(Type _type, const Token& _token);
	};
}