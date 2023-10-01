#include "lexer.h"
#include "common.h"

namespace Tolo
{
	Lexer::Lexer() :
		isInsideWhile(false)
	{}

	bool Lexer::IsOverloadableOperator(const Token& token)
	{
		switch (token.type)
		{
		case Token::Type::Plus:
		case Token::Type::Minus:
		case Token::Type::Asterisk:
		case Token::Type::ForwardSlash:
		case Token::Type::LeftArrow:
		case Token::Type::RightArrow:
		case Token::Type::EqualSign:
		case Token::Type::LeftArrowEqualSign:
		case Token::Type::RightArrowEqualSign:
		case Token::Type::DoubleEqualSign:
		case Token::Type::ExclamationMarkEqualSign:
			return true;
		}

		return false;
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

		Affirm(
			i < tokens.size() && tokens[i].type == Token::Type::Semicolon,
			"missing ';' at line %i",
			tokens[i - 1].line
		);
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

		LexNode::Type callType = LexNode::Type::UserFunctionCall;

		if (nativeFunctions.count(token.text) != 0)
			callType = LexNode::Type::NativeFunctionCall;

		LexNode* p_funcCall = new LexNode(callType, token);

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

			Affirm(i < tokens.size(), "unexpected end of tokens at line %i", p_node->token.line);

			if (tokens[i].type == Token::Type::Comma)
				i++;
			else
			{
				Affirm(
					tokens[i].type == Token::Type::EndPar, 
					"unexpected token '%s' at line %i", 
					tokens[i].text.c_str(), tokens[i].line
				);
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

	LexNode* Lexer::GetPropertyLoadOrWriteNode(const std::vector<Token>& tokens, size_t& i)
	{
		Affirm(
			i + 2 < tokens.size() && tokens[i + 2].type == Token::Type::Name,
			"expected identifier at line %i", 
			tokens[i].line
		);

		LexNode* p_prop = new LexNode(LexNode::Type::PropertyLoad, tokens[i]);
		p_prop->children.push_back(new LexNode(LexNode::Type::Identifier, tokens[i + 2]));

		i += 3;

		for (; i+1 < tokens.size(); i+=2)
		{
			if (tokens[i].type == Token::Type::Dot && tokens[i + 1].type == Token::Type::Name)
				p_prop->children.push_back(new LexNode(LexNode::Type::Identifier, tokens[i + 1]));
			else
				break;
		}

		if (i < tokens.size() && tokens[i].type == Token::Type::EqualSign)
		{
			p_prop->type = LexNode::Type::PropertyWrite;
			
			i++;
			LexNode* p_exp = GetNextNode(tokens, i);

			Affirm(
				p_exp->IsValueExpression(),
				"expected value expression at line %i",
				p_exp->token.line
			);

			Affirm(
				i < tokens.size() && tokens[i].type == Token::Type::Semicolon,
				"missing ';' at line %i",
				tokens[i - 1].line
			);
			i++;

			p_prop->children.push_back(p_exp);
		}

		return p_prop;
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

		Affirm(
			i < tokens.size() && tokens[i].type == Token::Type::Semicolon,
			"missing ';' at line %i",
			tokens[i - 1].line
		);
		i++;

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

		Affirm(
			i < tokens.size() && tokens[i].type == Token::Type::Semicolon,
			"missing ';' at line %i", 
			tokens[i - 1].line
		);
		i++;

		p_varDef->children.push_back(p_exp);

		return p_varDef;
	}

	LexNode* Lexer::GetFunctionDefinitionNode(const std::vector<Token>& tokens, size_t& i)
	{
		const Token& token = tokens[i];
		LexNode* p_funcDef = new LexNode(LexNode::Type::FunctionDefinition, token);

		Affirm(
			i + 5 < tokens.size(),
			"expected function definition at line %i",
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
				i+1 < tokens.size() && tokens[i].type == Token::Type::Name && tokens[i+1].type == Token::Type::Name,
				"expected identifier at line %i",
				tokens[i].line
			);

			p_funcDef->children.push_back(new LexNode(LexNode::Type::Identifier, tokens[i++]));
			p_funcDef->children.push_back(new LexNode(LexNode::Type::Identifier, tokens[i]));

			Affirm(i + 1 < tokens.size(), "unexpected end of tokens at line %i", tokens[i].line);
			i++;

			if (tokens[i].type == Token::Type::Comma)
				i++;
			else
			{
				Affirm(
					tokens[i].type == Token::Type::EndPar, 
					"unexpected token '%s' at line %i", 
					tokens[i].text.c_str(), tokens[i].line
				);
			}
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

	LexNode* Lexer::GetOperatorDefinitionNode(const std::vector<Token>& tokens, size_t& i)
	{
		const Token& token = tokens[i];
		LexNode* p_opDef = new LexNode(LexNode::Type::OperatorDefinition, token);

		Affirm(
			i + 6 < tokens.size(),
			"expected operator definition at line %i",
			token.line
		);

		Affirm(
			IsOverloadableOperator(tokens[i + 2]),
			"cannot overload operator '%s' at line %i",
			tokens[i + 2].text.c_str(), tokens[i + 2].line
		);

		p_opDef->children.push_back(new LexNode(LexNode::Type::Identifier, tokens[i + 2]));
		i += 4;

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
				i + 1 < tokens.size() && tokens[i].type == Token::Type::Name && tokens[i + 1].type == Token::Type::Name,
				"expected identifier at line %i",
				tokens[i].line
			);

			p_opDef->children.push_back(new LexNode(LexNode::Type::Identifier, tokens[i++]));
			p_opDef->children.push_back(new LexNode(LexNode::Type::Identifier, tokens[i]));

			Affirm(i + 1 < tokens.size(), "unexpected end of tokens at line %i", tokens[i].line);
			i++;

			if (tokens[i].type == Token::Type::Comma)
				i++;
			else
			{
				Affirm(
					tokens[i].type == Token::Type::EndPar,
					"unexpected token '%s' at line %i",
					tokens[i].text.c_str(), tokens[i].line
				);
			}
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

			p_opDef->children.push_back(p_node);
		}

		Affirm(
			foundEndCurly,
			"missing '}' after line %i",
			token.line
		);

		return p_opDef;
	}

	LexNode* Lexer::GetStructDefinitionNode(const std::vector<Token>& tokens, size_t& i)
	{
		const Token& token = tokens[i];
		LexNode* p_structDef = new LexNode(LexNode::Type::StructDefinition, token);

		Affirm(
			i + 3 < tokens.size() && tokens[i + 1].type == Token::Type::Name && tokens[i + 2].type == Token::Type::StartCurly,
			"expected struct definition at line %i", 
			token.line
		);

		p_structDef->children.push_back(new LexNode(LexNode::Type::Identifier, tokens[i + 1]));

		i += 3;

		bool foundEndCurly = false;
		while (i < tokens.size())
		{
			if (tokens[i].type == Token::Type::EndCurly)
			{
				Affirm(
					p_structDef->children.size() > 1, 
					"invalid struct definition at line %i, struct is empty", 
					token.line
				);
				foundEndCurly = true;
				i++;
				break;
			}

			Affirm(
				i + 2 < tokens.size() && tokens[i].type == Token::Type::Name && tokens[i + 1].type == Token::Type::Name && tokens[i + 2].type == Token::Type::Semicolon,
				"expected property definition at line %i", 
				tokens[i].line
			);

			p_structDef->children.push_back(new LexNode(LexNode::Type::Identifier, tokens[i]));
			p_structDef->children.push_back(new LexNode(LexNode::Type::Identifier, tokens[i+1]));

			i += 3;
		}

		Affirm(foundEndCurly, "missing '}' after line %i", token.line);
		Affirm(i < tokens.size() && tokens[i].type == Token::Type::Semicolon, "missing ';' at line %i", tokens[i - 1].line);
		i++;

		return p_structDef;
	}


	int Lexer::GetBinaryOpPrecedence(Token::Type tokenType)
	{
		switch (tokenType)
		{
		case Token::Type::DoubleAmpersand:
		case Token::Type::DoubleVerticalBar:
			return 1;
		case Token::Type::LeftArrow:
		case Token::Type::RightArrow:
		case Token::Type::DoubleEqualSign:
		case Token::Type::LeftArrowEqualSign:
		case Token::Type::RightArrowEqualSign:
		case Token::Type::ExclamationMarkEqualSign:
			return 2;
		case Token::Type::Plus:
		case Token::Type::Minus:
		case Token::Type::Ampersand:
		case Token::Type::VerticalBar:
		case Token::Type::Caret:
		case Token::Type::DoubleLeftArrow:
		case Token::Type::DoubleRightArrow:
			return 3;
		case Token::Type::Asterisk:
		case Token::Type::ForwardSlash:
			return 5;
		}

		return 0;
	}

	int Lexer::GetUnaryOpPrecedence(Token::Type tokenType)
	{
		switch (tokenType)
		{
		case Token::Type::ExclamationMark:
		case Token::Type::Minus:
			return 4;
		}

		return 0;
	}

	LexNode* Lexer::GetPrefix(const std::vector<Token>& tokens, size_t& i)
	{
		const Token& token = tokens[i];
		LexNode* p_result = nullptr;

		if (token.type == Token::Type::ConstChar || token.type == Token::Type::ConstInt || token.type == Token::Type::ConstFloat)
		{
			i++;
			p_result = new LexNode(LexNode::Type::LiteralConstant, token);
		}
		else if (token.type == Token::Type::Minus || token.type == Token::Type::ExclamationMark)
		{
			i++;
			p_result = new LexNode(LexNode::Type::UnaryOperation, token);
			p_result->children.push_back(GetNextNode(tokens, i, GetUnaryOpPrecedence(token.type)));
		}
		else if (token.type == Token::Type::StartPar)
		{
			i++;
			p_result = new LexNode(LexNode::Type::Parenthesis, token);
			p_result->children.push_back(GetNextNode(tokens, i));
			Affirm(i < tokens.size() && tokens[i].type == Token::Type::EndPar, "missing ')' after line %i", token.line);
			i++;
		}
		else if (token.type == Token::Type::Name)
		{
			if (token.text == "break")
			{
				Affirm(isInsideWhile, "cannot use keyword 'break' when not inside a 'while' body");
				p_result = new LexNode(LexNode::Type::Break, tokens[i++]);
			}
			else if (token.text == "continue")
			{
				Affirm(isInsideWhile, "cannot use keyword 'break' when not inside a 'while' body");
				p_result = new LexNode(LexNode::Type::Continue, tokens[i++]);
			}
			else if (token.text == "return")
				p_result = GetReturnNode(tokens, i);
			else if (token.text == "if")
				p_result = GetIfNode(tokens, i);
			else if (token.text == "while")
				p_result = GetWhileNode(tokens, i);
			else if (token.text == "struct")
				p_result = GetStructDefinitionNode(tokens, i);
			else if (i + 1 < tokens.size() && tokens[i + 1].type == Token::Type::StartPar)
				p_result = GetFunctionCallNode(tokens, i);
			else if (i + 1 < tokens.size() && tokens[i + 1].type == Token::Type::EqualSign)
				p_result = GetVariableWriteNode(tokens, i);
			else if (i + 1 < tokens.size() && tokens[i + 1].type == Token::Type::Dot)
				p_result = GetPropertyLoadOrWriteNode(tokens, i);
			else if (i + 2 < tokens.size() && tokens[i + 1].type == Token::Type::Name && tokens[i + 2].type == Token::Type::EqualSign)
				p_result = GetVariableDefinitionNode(tokens, i);
			else if (i + 2 < tokens.size() && tokens[i + 1].type == Token::Type::Name && tokens[i + 2].type == Token::Type::StartPar)
				p_result = GetFunctionDefinitionNode(tokens, i);
			else if (i + 1 < tokens.size() && tokens[i + 1].text == "operator")
				p_result = GetOperatorDefinitionNode(tokens, i);
			else
				p_result = GetVariableLoadNode(tokens, i);
		}
		else
			Affirm(false, "unexpected token '%s' at line %i", token.text.c_str(), token.line);

		return p_result;
	}

	LexNode* Lexer::GetInfix(const std::vector<Token>& tokens, size_t& i, LexNode* p_lhs)
	{
		const Token& token = tokens[i];
		int precedence = GetBinaryOpPrecedence(token.type);

		LexNode* p_result = new LexNode(LexNode::Type::BinaryOperation, token);
		p_result->children.push_back(p_lhs);
		p_result->children.push_back(GetNextNode(tokens, ++i, precedence));

		return p_result;
	}

	LexNode* Lexer::GetNextNode(const std::vector<Token>& tokens, size_t& i, int precedence)
	{
		Affirm(i < tokens.size(), "unexpected end of tokens");

		const Token& token = tokens[i];
		if (token.type == Token::Type::EndCurly)
		{
			i++;
			return new LexNode(LexNode::Type::EndCurly, token);
		}
		else if (token.type == Token::Type::EndPar)
		{
			i++;
			return new LexNode(LexNode::Type::EndPar, token);
		}

		LexNode* p_lhs = GetPrefix(tokens, i);

		while (i < tokens.size() && precedence < GetBinaryOpPrecedence(tokens[i].type))
		{
			p_lhs = GetInfix(tokens, i, p_lhs);
		}

		return p_lhs;
	}

	void Lexer::Lex(const std::vector<Token>& tokens, std::vector<LexNode*>& lexNodes)
	{
		for (size_t i = 0; i < tokens.size();)
			lexNodes.push_back(GetNextNode(tokens, i));
	}
}