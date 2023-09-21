#pragma once
#include "lex_node.h"
#include <set>

namespace Tolo
{
	struct Lexer
	{
		std::set<std::string> coreFunctions;
		bool isInsideWhile;

		Lexer();

		LexNode* GetReturnNode(const std::vector<Token>& tokens, size_t& i);

		LexNode* GetIfNode(const std::vector<Token>& tokens, size_t& i);
		
		LexNode* GetElseNode(const std::vector<Token>& tokens, size_t& i);

		LexNode* GetWhileNode(const std::vector<Token>& tokens, size_t& i);

		LexNode* GetFunctionCallNode(const std::vector<Token>& tokens, size_t& i);

		LexNode* GetVariableWriteNode(const std::vector<Token>& tokens, size_t& i);

		LexNode* GetVariableLoadNode(const std::vector<Token>& tokens, size_t& i);

		LexNode* GetVariableDefinitionNode(const std::vector<Token>& tokens, size_t& i);
		
		LexNode* GetFunctionDefinitionNode(const std::vector<Token>& tokens, size_t& i);

		LexNode* GetNextNode(const std::vector<Token>& tokens, size_t& i);
		
		void Lex(const std::vector<Token>& tokens, std::vector<LexNode*>& lexNodes);
	};
}