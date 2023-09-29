#pragma once
#include "lex_node.h"
#include <set>

namespace Tolo
{
	struct Lexer
	{
		std::set<std::string> nativeFunctions;
		bool isInsideWhile;

		Lexer();

		bool IsOverloadableOperator(const Token& token);

		LexNode* GetReturnNode(const std::vector<Token>& tokens, size_t& i);

		LexNode* GetIfNode(const std::vector<Token>& tokens, size_t& i);
		
		LexNode* GetElseNode(const std::vector<Token>& tokens, size_t& i);

		LexNode* GetWhileNode(const std::vector<Token>& tokens, size_t& i);

		LexNode* GetFunctionCallNode(const std::vector<Token>& tokens, size_t& i);

		LexNode* GetPropertyLoadOrWriteNode(const std::vector<Token>& tokens, size_t& i);

		LexNode* GetVariableWriteNode(const std::vector<Token>& tokens, size_t& i);

		LexNode* GetVariableLoadNode(const std::vector<Token>& tokens, size_t& i);

		LexNode* GetVariableDefinitionNode(const std::vector<Token>& tokens, size_t& i);
		
		LexNode* GetFunctionDefinitionNode(const std::vector<Token>& tokens, size_t& i);

		LexNode* GetStructDefinitionNode(const std::vector<Token>& tokens, size_t& i);

		LexNode* GetOperatorDefinitionNode(const std::vector<Token>& tokens, size_t& i);

		int GetBinaryOpPrecedence(Token::Type tokenType);

		int GetUnaryOpPrecedence(Token::Type tokenType);

		LexNode* GetPrefix(const std::vector<Token>& tokens, size_t& i);

		LexNode* GetInfix(const std::vector<Token>& tokens, size_t& i, LexNode* p_lhs);

		LexNode* GetNextNode(const std::vector<Token>& tokens, size_t& i, int precedence = 0);
		
		void Lex(const std::vector<Token>& tokens, std::vector<LexNode*>& lexNodes);
	};
}