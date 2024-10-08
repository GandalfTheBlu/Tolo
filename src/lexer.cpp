#include "lexer.h"
#include "common.h"

namespace Tolo
{
	Lexer::Lexer() :
		p_tokens(nullptr),
		tokenIndex(0),
		isInsideWhile(false)
	{}

	void Lexer::AffirmTokensLeft()
	{
		Affirm(
			tokenIndex < p_tokens->size(),
			"unexpected end of tokens"
		);
	}

	void Lexer::ConsumeCurrentToken(Token::Type affirmType)
	{
		const Token& token = CurrentToken();

		Affirm(
			token.type == affirmType,
			"unexpected '%s' at line %i",
			token.text.c_str(), token.line
		);

		tokenIndex++;
	}

	void Lexer::ConsumeNextToken(Token::Type affirmType)
	{
		tokenIndex++;
		ConsumeCurrentToken(affirmType);
	}

	const Token& Lexer::NextToken(Token::Type affirmType)
	{
		tokenIndex++;
		AffirmTokensLeft();

		const Token& token = CurrentToken();
		Affirm(
			token.type == affirmType,
			"unexpected token '%s' at line %i",
			token.text.c_str(), token.line
		);

		return token;
	}

	bool Lexer::TryCompareCurrentToken(Token::Type compareType)
	{
		return tokenIndex < p_tokens->size() && p_tokens->at(tokenIndex).type == compareType;
	}

	bool Lexer::TryCompareNextToken(Token::Type compareType)
	{
		return tokenIndex + 1 < p_tokens->size() && p_tokens->at(tokenIndex + 1).type == compareType;
	}

	const Token& Lexer::CurrentToken()
	{
		AffirmTokensLeft();
		return p_tokens->at(tokenIndex);
	}

	const Token& Lexer::CurrentToken(Token::Type affirmType)
	{
		AffirmTokensLeft();

		const Token& token = p_tokens->at(tokenIndex);

		Affirm(
			token.type == affirmType,
			"unexpected '%s' at line %i",
			token.text.c_str(), token.line
		);

		return token;
	}

	int Lexer::BinaryOpPrecedence(Token::Type tokenType)
	{
		switch (tokenType)
		{
		case Token::Type::EqualSign:
			return 1;
		case Token::Type::DoubleAmpersand:
		case Token::Type::DoubleVerticalBar:
			return 2;
		case Token::Type::LeftArrow:
		case Token::Type::RightArrow:
		case Token::Type::DoubleEqualSign:
		case Token::Type::LeftArrowEqualSign:
		case Token::Type::RightArrowEqualSign:
		case Token::Type::ExclamationMarkEqualSign:
			return 3;
		case Token::Type::Plus:
		case Token::Type::Minus:
		case Token::Type::Ampersand:
		case Token::Type::VerticalBar:
		case Token::Type::Caret:
		case Token::Type::DoubleLeftArrow:
		case Token::Type::DoubleRightArrow:
			return 4;
		case Token::Type::Asterisk:
		case Token::Type::ForwardSlash:
			return 6;
		}

		return 0;
	}

	int Lexer::UnaryOpPrecedence(Token::Type tokenType)
	{
		switch (tokenType)
		{
		case Token::Type::ExclamationMark:
		case Token::Type::Minus:
		case Token::Type::Tilde:
			return 5;
		case Token::Type::Ampersand:
		case Token::Type::Asterisk:
			return 7;
		}

		return 0;
	}


	void Lexer::Lex(const std::vector<Token>& tokens, std::vector<SharedNode>& lexNodes)
	{
		p_tokens = &tokens;
		tokenIndex = 0;

		while (tokenIndex < p_tokens->size())
			lexNodes.push_back(LGlobalStructure());
	}


	Lexer::SharedNode Lexer::LGlobalStructure() 
	{
		const Token& idToken = CurrentToken(Token::Type::Name);

		if (idToken.text == "struct")
			return LStructDefinition();

		if (idToken.text == "enum")
			return LEnumDefinition();

		Affirm(
			TryCompareNextToken(Token::Type::Name),
			"unexpected token at line %i",
			idToken.line
		);

		const Token& nameToken = p_tokens->at(tokenIndex + 1);

		if (nameToken.text == "operator")
			return LOperatorDefinition();
		
		if (tokenIndex + 2 < p_tokens->size() &&
			p_tokens->at(tokenIndex + 2).type == Token::Type::DoubleColon)
		{
			return LMemberFunctionDefinition();
		}

		return LFunctionDefinition();
	}

	Lexer::SharedNode Lexer::LStructDefinition()
	{
		ConsumeCurrentToken(Token::Type::Name);

		const Token& nameToken = CurrentToken(Token::Type::Name);
		auto structDefNode = std::make_shared<LexNode>(LexNode::Type::StructDefinition, nameToken);

		// handle inheritance
		if (TryCompareNextToken(Token::Type::Colon))
		{
			ConsumeNextToken(Token::Type::Colon);
			structDefNode->type = LexNode::Type::StructDefinitionInheritance;
			structDefNode->children.push_back(std::make_shared<LexNode>(
				LexNode::Type::Identifier, 
				CurrentToken(Token::Type::Name
			)));
		}

		ConsumeNextToken(Token::Type::StartCurly);
		

		while (true)
		{
			if (CurrentToken().type == Token::Type::EndCurly)
			{
				ConsumeNextToken(Token::Type::Semicolon);
				break;
			}

			structDefNode->children.push_back(LIdentifier());
			const Token& membNameToken = CurrentToken(Token::Type::Name);

			ConsumeNextToken(Token::Type::Semicolon);

			structDefNode->children.push_back(std::make_shared<LexNode>(LexNode::Type::Identifier, membNameToken));
		}

		return structDefNode;
	}

	Lexer::SharedNode Lexer::LFunctionDefinition() 
	{
		auto funcDefNode = LIdentifier();
		funcDefNode->type = LexNode::Type::FunctionDefinition;

		const Token& nameToken = CurrentToken(Token::Type::Name);
		funcDefNode->children.push_back(std::make_shared<LexNode>(LexNode::Type::Identifier, nameToken));

		ConsumeNextToken(Token::Type::StartPar);

		if (CurrentToken().type == Token::Type::EndPar)
		{
			tokenIndex++;
		}
		else
		{
			while (true)
			{
				funcDefNode->children.push_back(LIdentifier());
				const Token& argNameToken = CurrentToken(Token::Type::Name);
				funcDefNode->children.push_back(std::make_shared<LexNode>(LexNode::Type::Identifier, argNameToken));

				if (TryCompareNextToken(Token::Type::EndPar))
				{
					tokenIndex += 2;
					break;
				}

				ConsumeNextToken(Token::Type::Comma);
			}
		}

		funcDefNode->children.push_back(LStatement());

		return funcDefNode;
	}

	Lexer::SharedNode Lexer::LOperatorDefinition() 
	{
		auto opDefNode = LIdentifier();
		opDefNode->type = LexNode::Type::OperatorDefinition;

		ConsumeCurrentToken(Token::Type::Name);

		const Token& opToken = CurrentToken();
		opDefNode->children.push_back(std::make_shared<LexNode>(LexNode::Type::Identifier, opToken));

		switch (opToken.type)
		{
		case Token::Type::Plus:
		case Token::Type::Minus:
		case Token::Type::Asterisk:
		case Token::Type::ForwardSlash:
		case Token::Type::LeftArrow:
		case Token::Type::RightArrow:
		case Token::Type::LeftArrowEqualSign:
		case Token::Type::RightArrowEqualSign:
		case Token::Type::DoubleEqualSign:
		case Token::Type::ExclamationMarkEqualSign:
			break;
		default:
			Affirm(false, "unexpected token '%s' at line %i", opToken.text.c_str(), opToken.line);
			break;
		}

		ConsumeNextToken(Token::Type::StartPar);

		if (CurrentToken().type == Token::Type::EndPar)
		{
			tokenIndex++;
		}
		else
		{
			while (true)
			{
				opDefNode->children.push_back(LIdentifier());
				const Token& argNameToken = CurrentToken(Token::Type::Name);
				opDefNode->children.push_back(std::make_shared<LexNode>(LexNode::Type::Identifier, argNameToken));

				if (TryCompareNextToken(Token::Type::EndPar))
				{
					tokenIndex += 2;
					break;
				}

				ConsumeNextToken(Token::Type::Comma);
			}
		}

		opDefNode->children.push_back(LStatement());

		return opDefNode;
	}

	Lexer::SharedNode Lexer::LMemberFunctionDefinition()
	{
		auto membFuncDefNode = LIdentifier();
		membFuncDefNode->type = LexNode::Type::MemberFunctionDefinition;

		const Token& structTypeToken = CurrentToken(Token::Type::Name);
		ConsumeNextToken(Token::Type::DoubleColon);
		const Token& funcNameToken = CurrentToken(Token::Type::Name);
		ConsumeNextToken(Token::Type::StartPar);

		membFuncDefNode->children.push_back(std::make_shared<LexNode>(LexNode::Type::Identifier, structTypeToken));
		membFuncDefNode->children.push_back(std::make_shared<LexNode>(LexNode::Type::Identifier, funcNameToken));

		if (CurrentToken().type == Token::Type::EndPar)
		{
			tokenIndex++;
		}
		else
		{
			while (true)
			{
				membFuncDefNode->children.push_back(LIdentifier());
				const Token& argNameToken = CurrentToken(Token::Type::Name);

				membFuncDefNode->children.push_back(std::make_shared<LexNode>(LexNode::Type::Identifier, argNameToken));

				if (TryCompareNextToken(Token::Type::EndPar))
				{
					tokenIndex += 2;
					break;
				}

				ConsumeNextToken(Token::Type::Comma);
			}
		}

		// handle virtual member function
		if (CurrentToken().type == Token::Type::Colon)
		{
			const Token& specToken = NextToken(Token::Type::Name);
			tokenIndex++;

			Affirm(
				specToken.text == "virtual",
				"unexpected token '%s' at line %i",
				specToken.text.c_str(), specToken.line
			);

			membFuncDefNode->type = LexNode::Type::MemberFunctionDefinitionVirtual;
		}

		membFuncDefNode->children.push_back(LStatement());

		return membFuncDefNode;
	}

	Lexer::SharedNode Lexer::LEnumDefinition()
	{
		ConsumeCurrentToken(Token::Type::Name);
		const Token& enumNamespaceToken = CurrentToken(Token::Type::Name);

		ConsumeNextToken(Token::Type::StartCurly);

		auto enumDefNode = std::make_shared<LexNode>(LexNode::Type::EnumDefinition, enumNamespaceToken);

		bool first = true;

		while (true)
		{
			if (CurrentToken().type == Token::Type::EndCurly)
			{
				ConsumeNextToken(Token::Type::Semicolon);
				break;
			}

			if (!first)
				ConsumeCurrentToken(Token::Type::Comma);

			const Token& nameToken = CurrentToken(Token::Type::Name);
			enumDefNode->children.push_back(std::make_shared<LexNode>(LexNode::Type::Identifier, nameToken));
			tokenIndex++;
			first = false;
		}

		return enumDefNode;
	}

	Lexer::SharedNode Lexer::LStatement() 
	{
		const Token& token = CurrentToken();

		if (token.type == Token::Type::StartCurly)
			return LScope();

		if (token.type == Token::Type::Name)
		{
			if (token.text == "if")
				return LIfStatement();
			if (token.text == "while")
				return LWhileStatement();
			if (token.text == "break")
				return LBreakStatement();
			if (token.text == "continue")
				return LContinueStatement();
			if (token.text == "return")
				return LReturnStatement();
		}

		auto statNode = LExpression(0);
		ConsumeCurrentToken(Token::Type::Semicolon);

		return statNode;
	}

	Lexer::SharedNode Lexer::LScope()
	{
		const Token& scopeToken = CurrentToken();
		tokenIndex++;

		auto scopeNode = std::make_shared<LexNode>(LexNode::Type::Scope, scopeToken);

		while (true)
		{
			if (CurrentToken().type == Token::Type::EndCurly)
			{
				tokenIndex++;
				break;
			}

			scopeNode->children.push_back(LStatement());
		}

		return scopeNode;
	}

	Lexer::SharedNode Lexer::LIfStatement() 
	{
		const Token& ifToken = CurrentToken();

		ConsumeNextToken(Token::Type::StartPar);

		auto ifNode = std::make_shared<LexNode>(LexNode::Type::IfSingle, ifToken);
		ifNode->children.push_back(LExpression(0));

		ConsumeCurrentToken(Token::Type::EndPar);

		ifNode->children.push_back(LStatement());

		if (tokenIndex < p_tokens->size())
		{
			const Token& elseToken = CurrentToken();

			if (elseToken.type == Token::Type::Name && elseToken.text == "else")
			{
				if (TryCompareNextToken(Token::Type::Name) && p_tokens->at(tokenIndex + 1).text == "if")
				{
					tokenIndex++;
					auto elseIfNode = LIfStatement();
					
					if (elseIfNode->type == LexNode::Type::IfChain)
						elseIfNode->type == LexNode::Type::ElseIfChain;
					else
						elseIfNode->type = LexNode::Type::ElseIfSingle;

					ifNode->children.push_back(elseIfNode);
				}
				else
				{
					ifNode->children.push_back(LElseStatement());
				}
			}
		}

		return ifNode;
	}

	Lexer::SharedNode Lexer::LElseStatement()
	{
		const Token& elseToken = CurrentToken();
		tokenIndex++;

		auto elseNode = std::make_shared<LexNode>(LexNode::Type::Else, elseToken);
		elseNode->children.push_back(LStatement());

		return elseNode;
	}

	Lexer::SharedNode Lexer::LWhileStatement() 
	{
		const Token& whileToken = CurrentToken();

		ConsumeNextToken(Token::Type::StartPar);

		auto whileNode = std::make_shared<LexNode>(LexNode::Type::While, whileToken);
		whileNode->children.push_back(LExpression(0));

		ConsumeCurrentToken(Token::Type::EndPar);

		bool wasInsideWhileLoop = isInsideWhile;
		isInsideWhile = true;
		whileNode->children.push_back(LStatement());
		isInsideWhile = wasInsideWhileLoop;

		return whileNode;
	}

	Lexer::SharedNode Lexer::LBreakStatement() 
	{
		Affirm(isInsideWhile, "cannot use 'break' outside while-loop at line %i", CurrentToken().line);

		auto breakNode = std::make_shared<LexNode>(LexNode::Type::Break, CurrentToken());
		
		if (TryCompareNextToken(Token::Type::ConstInt))
		{
			const Token& numToken = NextToken(Token::Type::ConstInt);
			breakNode->children.push_back(std::make_shared<LexNode>(LexNode::Type::LiteralConstant, numToken));
		}

		ConsumeNextToken(Token::Type::Semicolon);

		return breakNode;
	}

	Lexer::SharedNode Lexer::LContinueStatement() 
	{
		Affirm(isInsideWhile, "cannot use 'continue' outside while-loop at line %i", CurrentToken().line);

		auto continueNode = std::make_shared<LexNode>(LexNode::Type::Continue, CurrentToken());
	
		if (TryCompareNextToken(Token::Type::ConstInt))
		{
			const Token& numToken = NextToken(Token::Type::ConstInt);
			continueNode->children.push_back(std::make_shared<LexNode>(LexNode::Type::LiteralConstant, numToken));
		}

		ConsumeNextToken(Token::Type::Semicolon);

		return continueNode;
	}

	Lexer::SharedNode Lexer::LReturnStatement() 
	{
		auto returnNode = std::make_shared<LexNode>(LexNode::Type::Return, CurrentToken());
		tokenIndex++;

		returnNode->children.push_back(LExpression(0));
		ConsumeCurrentToken(Token::Type::Semicolon);

		return returnNode;
	}

	Lexer::SharedNode Lexer::LExpression(int precedence)
	{
		SharedNode lhsNode = LPrefix();

		while (tokenIndex < p_tokens->size() && precedence < BinaryOpPrecedence(CurrentToken().type))
		{
			SharedNode newLhs = LInfix(lhsNode);
			lhsNode = newLhs;
		}

		return lhsNode;
	}

	Lexer::SharedNode Lexer::LPrefix()
	{
		const Token& token = CurrentToken();

		if (token.type == Token::Type::ExclamationMark ||
			token.type == Token::Type::Minus ||
			token.type == Token::Type::Ampersand ||
			token.type == Token::Type::Asterisk || 
			token.type == Token::Type::Tilde)
		{
			return LUnaryOp();
		}
		if (token.type == Token::Type::StartPar)
		{
			return LParenthesis();
		}
		if (token.type == Token::Type::ConstChar ||
			token.type == Token::Type::ConstInt ||
			token.type == Token::Type::ConstFloat ||
			token.type == Token::Type::ConstString)
		{
			return LLiteral();
		}
		if (token.type == Token::Type::Name)
		{
			if (TryCompareNextToken(Token::Type::DoubleColon))
				return LNameWithDoubleColon();
			if (TryCompareNextToken(Token::Type::Dot))
				return LMemberAccess();
			if (TryCompareNextToken(Token::Type::StartPar))
				return LFunctionCall();
			if (TryCompareNextToken(Token::Type::Name))
				return LVariableDefinition();

			return LIdentifier();
		}
	}

	Lexer::SharedNode Lexer::LInfix(const SharedNode& lhsNode)
	{
		const Token& lhsToken = CurrentToken();
		int precedence = BinaryOpPrecedence(lhsToken.type);

		auto binOpNode = std::make_shared<LexNode>(LexNode::Type::BinaryOperation, lhsToken);
		binOpNode->children.push_back(lhsNode);

		tokenIndex++;
		binOpNode->children.push_back(LExpression(precedence));

		return binOpNode;
	}

	Lexer::SharedNode Lexer::LNameWithDoubleColon()
	{
		auto idNode = LIdentifier();

		const Token& token = CurrentToken();

		if (token.type == Token::Type::Name)
		{
			idNode->type = LexNode::Type::VariableDefinition;
			idNode->children.push_back(LIdentifier());
		}
		else if (token.type == Token::Type::StartPar)
		{
			idNode->type = LexNode::Type::FunctionCall;
			tokenIndex++;

			if (CurrentToken().type == Token::Type::EndPar)
			{
				tokenIndex++;
			}
			else
			{
				while (true)
				{
					idNode->children.push_back(LExpression(0));

					if (CurrentToken().type == Token::Type::EndPar)
					{
						tokenIndex++;
						break;
					}

					ConsumeCurrentToken(Token::Type::Comma);
				}
			}
		}

		return idNode;
	}

	Lexer::SharedNode Lexer::LVariableDefinition()
	{
		auto varDefNode = std::make_shared<LexNode>(LexNode::Type::VariableDefinition, CurrentToken());
		
		const Token& varNameToken = NextToken(Token::Type::Name);
		tokenIndex++;

		varDefNode->children.push_back(std::make_shared<LexNode>(LexNode::Type::Identifier, varNameToken));

		return varDefNode;
	}

	Lexer::SharedNode Lexer::LIdentifier() 
	{
		auto idNode = std::make_shared<LexNode>(LexNode::Type::Identifier, CurrentToken());

		if (TryCompareNextToken(Token::Type::DoubleColon))
		{
			idNode->token.text += NextToken(Token::Type::DoubleColon).text;
			idNode->token.text += NextToken(Token::Type::Name).text;
		}

		tokenIndex++;

		return idNode;
	}

	Lexer::SharedNode Lexer::LMemberAccess()
	{
		auto membAccessNode = std::make_shared<LexNode>(LexNode::Type::MemberVariableAccess, CurrentToken());
		tokenIndex++;

		while (true)
		{
			if (!TryCompareCurrentToken(Token::Type::Dot))
				break;
			
			const Token& nameToken = NextToken(Token::Type::Name);

			// handle member function call
			if (TryCompareNextToken(Token::Type::StartPar))
			{
				membAccessNode->type = LexNode::Type::MemberFunctionCall;
				membAccessNode->children.push_back(LFunctionCall());
				break;
			}

			membAccessNode->children.push_back(std::make_shared<LexNode>(LexNode::Type::Identifier, nameToken));
			tokenIndex++;
		}

		return membAccessNode;
	}

	Lexer::SharedNode Lexer::LLiteral() 
	{
		auto litNode = std::make_shared<LexNode>(LexNode::Type::LiteralConstant, CurrentToken());
		tokenIndex++;
		return litNode;
	}

	Lexer::SharedNode Lexer::LUnaryOp() 
	{
		const Token& opToken = CurrentToken();
		tokenIndex++;

		auto opNode = std::make_shared<LexNode>(LexNode::Type::UnaryOperation, opToken);
		
		opNode->children.push_back(LExpression(UnaryOpPrecedence(opToken.type)));

		return opNode;
	}

	Lexer::SharedNode Lexer::LParenthesis() 
	{
		const Token& parToken = CurrentToken();
		auto parNode = std::make_shared<LexNode>(LexNode::Type::Parenthesis, parToken);
		tokenIndex++;

		parNode->children.push_back(LExpression(0));

		ConsumeCurrentToken(Token::Type::EndPar);

		return parNode;
	}

	Lexer::SharedNode Lexer::LFunctionCall() 
	{
		const Token& nameToken = CurrentToken();

		ConsumeNextToken(Token::Type::StartPar);

		auto funcCallNode = std::make_shared<LexNode>(LexNode::Type::FunctionCall, nameToken);

		if (CurrentToken().type == Token::Type::EndPar)
		{
			tokenIndex++;
		}
		else
		{
			while (true)
			{
				funcCallNode->children.push_back(LExpression(0));

				if (CurrentToken().type == Token::Type::EndPar)
				{
					tokenIndex++;
					break;
				}

				ConsumeCurrentToken(Token::Type::Comma);
			}
		}

		return funcCallNode;
	}
}