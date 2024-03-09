#pragma once
#include "lex_node.h"
#include <memory>

namespace Tolo
{
	struct Lexer
	{
		const std::vector<Token>* p_tokens;
		size_t tokenIndex;
		bool isInsideWhile;

		using SharedNode = std::shared_ptr<LexNode>;

		Lexer();

		void AffirmTokensLeft();

		void ConsumeCurrentToken(Token::Type affirmType);

		void ConsumeNextToken(Token::Type affirmType);
		
		const Token& CurrentToken();

		const Token& CurrentToken(Token::Type affirmType);

		const Token& NextToken(Token::Type affirmType);

		bool TryCompareCurrentToken(Token::Type compareType);

		bool TryCompareNextToken(Token::Type compareType);

		int BinaryOpPrecedence(Token::Type tokenType);

		int UnaryOpPrecedence(Token::Type tokenType);

		// root lexer
		void Lex(const std::vector<Token>& tokens, std::vector<SharedNode>& lexNodes);

		// global structures
		SharedNode LGlobalStructure();

		SharedNode LStructDefinition();

		SharedNode LFunctionDefinition();

		SharedNode LOperatorDefinition();

		SharedNode LMemberFunctionDefinition();

		SharedNode LEnumDefinition();

		// statements
		SharedNode LStatement();

		SharedNode LScope();

		SharedNode LIfStatement();

		SharedNode LElseStatement();

		SharedNode LWhileStatement();

		SharedNode LBreakStatement();

		SharedNode LContinueStatement();

		SharedNode LReturnStatement();

		// expressions
		SharedNode LExpression(int precedence);

		SharedNode LPrefix();

		SharedNode LInfix(const SharedNode& lhsNode);

		SharedNode LNameWithDoubleColon();

		SharedNode LVariableDefinition();

		SharedNode LIdentifier();

		SharedNode LMemberAccess();

		SharedNode LLiteral();

		SharedNode LUnaryOp();

		SharedNode LParenthesis();

		SharedNode LFunctionCall();
	};
}