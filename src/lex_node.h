#pragma once
#include "token.h"
#include <vector>

namespace Tolo
{
	struct LexNode
	{
		enum class Type
		{
			Return,
			IfSingle,// ifSingle{cond, body[...]}
			IfChain,// ifChain{cond, body[...], elseIfSingle | elseIfChain | else}
			ElseIfSingle,// elseIfSignle{cond, body[...]}
			ElseIfChain,// elseIfChain{cond, body[...], elseIfSingle | elseIfChain | else}
			Else,// else{body[...]}
			While,
			Break,
			Continue,
			CoreFunctionCall,
			NativeFunctionCall,
			UserFunctionCall,
			VariableWrite,
			VariableLoad,
			PropertyLoad,
			PropertyWrite,
			StructDefinition,
			VariableDefinition,
			FunctionDefinition,
			Identifier,
			LiteralConstant,
			EndCurly,
			EndPar
		};

		Type type;
		Token token;
		std::vector<LexNode*> children;

		LexNode(Type _type, const Token& _token);

		~LexNode();

		bool IsValueExpression();

		bool IsValidExpressionInScope();
	};
}