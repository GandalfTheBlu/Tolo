#include "lexer.h"
#include "common.h"

namespace Tolo
{
	Lexer::Lexer() :
		isInsideWhile(false)
	{
		coreFunctions.insert("less");
		coreFunctions.insert("greater");
		coreFunctions.insert("equal");
		coreFunctions.insert("print");
		coreFunctions.insert("add");
		coreFunctions.insert("sub");
		coreFunctions.insert("mul");
		coreFunctions.insert("div");
	}

	LexNode* Lexer::GetReturnNode(const std::vector<Token>& tokens, size_t& i)
	{
		LexNode* p_ret = new LexNode(LexNode::Type::Return, tokens[i]);

		size_t nextIndex = i + 1;
		LexNode* p_exp = GetNextNode(tokens, nextIndex);
		if (p_exp->IsValueExpression())
		{
			p_ret->children.push_back(p_exp);
			i = nextIndex;
		}
		else
			i++;

		return p_ret;
	}

	LexNode* Lexer::GetIfNode(const std::vector<Token>& tokens, size_t& i)
	{
		const Token& token = tokens[i];
		LexNode* p_if = new LexNode(LexNode::Type::IfSingle, token);

		Affirm(
			++i < tokens.size() && tokens[i].type == Token::Type::StartPar,
			"missing '(' at line %i",
			token.line
		);

		LexNode* p_cond = GetNextNode(tokens, ++i);

		Affirm(
			p_cond->IsValueExpression(),
			"expected value expression at line %i",
			p_cond->token.line
		);

		p_if->children.push_back(p_cond);

		Affirm(
			i < tokens.size() && tokens[i].type == Token::Type::EndPar,
			"missing ')' at line %i",
			token.line
		);
		i++;

		Affirm(
			i < tokens.size() && tokens[i].type == Token::Type::StartCurly,
			"missing '{' at line %i",
			token.line
		);
		i++;

		bool foundEndCurly = false;
		while (i < tokens.size())
		{
			LexNode* p_node = GetNextNode(tokens, i);

			if (p_node->type == LexNode::Type::EndCurly)
			{
				delete p_node;
				foundEndCurly = true;
				break;
			}

			Affirm(
				p_node->IsValidExpressionInScope(),
				"invalid expression '%s' at line %i",
				p_node->token.text.c_str(), p_node->token.line
			);

			p_if->children.push_back(p_node);
		}

		Affirm(
			foundEndCurly,
			"missing '}' after line %i",
			token.line
		);

		if (i < tokens.size() && tokens[i].type == Token::Type::Name && tokens[i].text == "else")
		{
			p_if->type = LexNode::Type::IfChain;

			if (i + 1 < tokens.size() && tokens[i + 1].type == Token::Type::Name && tokens[i + 1].text == "if")
			{
				LexNode* p_elseIf = GetIfNode(tokens, ++i);
				// convert if node to else if node of the correct type (single or chained)
				if (p_elseIf->type == LexNode::Type::IfSingle)
					p_elseIf->type = LexNode::Type::ElseIfSingle;
				else
					p_elseIf->type = LexNode::Type::ElseIfChain;

				p_if->children.push_back(p_elseIf);
			}
			else
				p_if->children.push_back(GetElseNode(tokens, i));
		}

		return p_if;
	}

	LexNode* Lexer::GetElseNode(const std::vector<Token>& tokens, size_t& i)
	{
		const Token& token = tokens[i];
		LexNode* p_else = new LexNode(LexNode::Type::Else, token);

		Affirm(
			++i < tokens.size() && tokens[i].type == Token::Type::StartCurly,
			"missing '{' at line %i",
			token.line
		);
		i++;

		bool foundEndCurly = false;
		while (i < tokens.size())
		{
			LexNode* p_node = GetNextNode(tokens, i);

			if (p_node->type == LexNode::Type::EndCurly)
			{
				delete p_node;
				foundEndCurly = true;
				break;
			}

			Affirm(
				p_node->IsValidExpressionInScope(),
				"invalid expression '%s' at line %i",
				p_node->token.text.c_str(), p_node->token.line
			);

			p_else->children.push_back(p_node);
		}

		Affirm(
			foundEndCurly,
			"missing '}' after line %i",
			token.line
		);

		return p_else;
	}

	LexNode* Lexer::GetWhileNode(const std::vector<Token>& tokens, size_t& i)
	{
		const Token& token = tokens[i];
		LexNode* p_while = new LexNode(LexNode::Type::While, token);

		Affirm(
			++i < tokens.size() && tokens[i].type == Token::Type::StartPar,
			"missing '(' at line %i",
			token.line
		);

		LexNode* p_cond = GetNextNode(tokens, ++i);

		Affirm(
			p_cond->IsValueExpression(),
			"expected value expression at line %i",
			p_cond->token.line
		);

		p_while->children.push_back(p_cond);

		Affirm(
			i < tokens.size() && tokens[i].type == Token::Type::EndPar,
			"missing ')' at line %i",
			token.line
		);
		i++;

		Affirm(
			i < tokens.size() && tokens[i].type == Token::Type::StartCurly,
			"missing '{' at line %i",
			token.line
		);
		i++;

		bool foundEndCurly = false;
		bool wasInsideWhile = isInsideWhile;
		isInsideWhile = true;
		while (i < tokens.size())
		{
			LexNode* p_node = GetNextNode(tokens, i);

			if (p_node->type == LexNode::Type::EndCurly)
			{
				delete p_node;
				foundEndCurly = true;
				break;
			}

			Affirm(
				p_node->IsValidExpressionInScope(),
				"invalid expression '%s' at line %i",
				p_node->token.text.c_str(), p_node->token.line
			);

			p_while->children.push_back(p_node);
		}
		isInsideWhile = wasInsideWhile;

		Affirm(
			foundEndCurly,
			"missing '}' after line %i",
			token.line
		);

		return p_while;
	}

	LexNode* Lexer::GetFunctionCallNode(const std::vector<Token>& tokens, size_t& i)
	{
		const Token& token = tokens[i];
		bool isCoreFunction = coreFunctions.count(token.text) != 0;
		LexNode* p_funcCall = new LexNode(isCoreFunction ? LexNode::Type::CoreFunctionCall : LexNode::Type::UserFunctionCall, token);

		Affirm(
			++i < tokens.size() && tokens[i].type == Token::Type::StartPar,
			"missing '(' at line %i",
			token.line
		);
		i++;

		bool foundEndPar = false;
		while (i < tokens.size())
		{
			LexNode* p_node = GetNextNode(tokens, i);

			if (p_node->token.type == Token::Type::EndPar)
			{
				foundEndPar = true;
				delete p_node;
				break;
			}

			Affirm(
				p_node->IsValueExpression(),
				"expected value expression at line %i",
				p_node->token.line
			);

			p_funcCall->children.push_back(p_node);
		}

		Affirm(
			foundEndPar,
			"missing ')' after line %i",
			token.line
		);

		return p_funcCall;
	}

	LexNode* Lexer::GetVariableWriteNode(const std::vector<Token>& tokens, size_t& i)
	{
		const Token& token = tokens[i];
		LexNode* p_varWrite = new LexNode(LexNode::Type::VariableWrite, token);

		Affirm(
			++i < tokens.size() && tokens[i].type == Token::Type::EqualSign,
			"expected '=' at line %i",
			token.line
		);
		i++;


		LexNode* p_exp = GetNextNode(tokens, i);

		Affirm(
			p_exp->IsValueExpression(),
			"expected value expression at line %i",
			token.line
		);

		p_varWrite->children.push_back(p_exp);
		return p_varWrite;
	}

	LexNode* Lexer::GetVariableLoadNode(const std::vector<Token>& tokens, size_t& i)
	{
		const Token& token = tokens[i];
		i++;
		return new LexNode(LexNode::Type::VariableLoad, token);
	}

	LexNode* Lexer::GetVariableDefinitionNode(const std::vector<Token>& tokens, size_t& i)
	{
		const Token& token = tokens[i];
		LexNode* p_varDef = new LexNode(LexNode::Type::VariableDefinition, token);

		Affirm(
			i + 3 < tokens.size() && tokens[i + 1].type == Token::Type::Name && tokens[i + 2].type == Token::Type::EqualSign,
			"expected variable definition at line %i",
			token.line
		);

		p_varDef->children.push_back(new LexNode(LexNode::Type::Identifier, tokens[i + 1]));
		i += 3;

		LexNode* p_exp = GetNextNode(tokens, i);

		Affirm(
			p_exp->IsValueExpression(),
			"expected value expression at line %i",
			token.line
		);

		p_varDef->children.push_back(p_exp);

		return p_varDef;
	}

	LexNode* Lexer::GetFunctionDefinitionNode(const std::vector<Token>& tokens, size_t& i)
	{
		const Token& token = tokens[i];
		LexNode* p_funcDef = new LexNode(LexNode::Type::FunctionDefinition, token);

		Affirm(
			i + 5 < tokens.size() && tokens[i + 1].type == Token::Type::Name && tokens[i + 2].type == Token::Type::StartPar,
			"expected variable definition at line %i",
			token.line
		);

		p_funcDef->children.push_back(new LexNode(LexNode::Type::Identifier, tokens[i + 1]));
		i += 3;

		bool foundEndPar = false;
		while (i < tokens.size())
		{
			if (tokens[i].type == Token::Type::EndPar)
			{
				i++;
				foundEndPar = true;
				break;
			}

			Affirm(
				tokens[i].type == Token::Type::Name,
				"expected identifier at line %i",
				tokens[i].line
			);

			p_funcDef->children.push_back(new LexNode(LexNode::Type::Identifier, tokens[i]));
			i++;
		}

		Affirm(
			foundEndPar,
			"missing ')' after line %i",
			token.line
		);

		Affirm(
			i < tokens.size() && tokens[i].type == Token::Type::StartCurly,
			"missing '{' at line %i",
			token.line
		);
		i++;

		bool foundEndCurly = false;
		while (i < tokens.size())
		{
			LexNode* p_node = GetNextNode(tokens, i);

			if (p_node->type == LexNode::Type::EndCurly)
			{
				delete p_node;
				foundEndCurly = true;
				break;
			}

			Affirm(
				p_node->IsValidExpressionInScope(),
				"invalid expression '%s' at line %i",
				p_node->token.text.c_str(), p_node->token.line
			);

			p_funcDef->children.push_back(p_node);
		}

		Affirm(
			foundEndCurly,
			"missing '}' after line %i",
			token.line
		);

		return p_funcDef;
	}

	LexNode* Lexer::GetNextNode(const std::vector<Token>& tokens, size_t& i)
	{
		const Token& token = tokens[i];

		if (token.type == Token::Type::EndCurly)
		{
			i++;
			return new LexNode(LexNode::Type::EndCurly, token);
		}
		if (token.type == Token::Type::EndPar)
		{
			i++;
			return new LexNode(LexNode::Type::EndPar, token);
		}
		if (token.type == Token::Type::ConstChar || token.type == Token::Type::ConstInt || token.type == Token::Type::ConstFloat)
		{
			i++;
			return new LexNode(LexNode::Type::LiteralConstant, token);
		}
		if (token.type == Token::Type::Name)
		{
			if (token.text == "break")
			{
				Affirm(isInsideWhile, "cannot use keyword 'break' when not inside a 'while' body");
				return new LexNode(LexNode::Type::Break, tokens[i++]);
			}
			if (token.text == "continue")
			{
				Affirm(isInsideWhile, "cannot use keyword 'break' when not inside a 'while' body");
				return new LexNode(LexNode::Type::Continue, tokens[i++]);
			}
			if (token.text == "return")
				return GetReturnNode(tokens, i);
			if (token.text == "if")
				return GetIfNode(tokens, i);
			if (token.text == "while")
				return GetWhileNode(tokens, i);
			if (i + 1 < tokens.size() && tokens[i + 1].type == Token::Type::StartPar)
				return GetFunctionCallNode(tokens, i);
			if (i + 1 < tokens.size() && tokens[i + 1].type == Token::Type::EqualSign)
				return GetVariableWriteNode(tokens, i);
			if (i + 2 < tokens.size() && tokens[i + 1].type == Token::Type::Name && tokens[i + 2].type == Token::Type::EqualSign)
				return GetVariableDefinitionNode(tokens, i);
			else if (i + 2 < tokens.size() && tokens[i + 1].type == Token::Type::Name && tokens[i + 2].type == Token::Type::StartPar)
				return GetFunctionDefinitionNode(tokens, i);
			else
				return GetVariableLoadNode(tokens, i);
		}
		else
			Affirm(false, "unexpected token '%s' at line %i", token.text.c_str(), token.line);
	}

	void Lexer::Lex(const std::vector<Token>& tokens, std::vector<LexNode*>& lexNodes)
	{
		for (size_t i = 0; i < tokens.size();)
			lexNodes.push_back(GetNextNode(tokens, i));
	}
}