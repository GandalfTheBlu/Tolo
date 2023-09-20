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
			If,
			While,
			CoreFunctionCall,
			UserFunctionCall,
			VariableWrite,
			VariableLoad,
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