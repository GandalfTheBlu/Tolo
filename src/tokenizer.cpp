#include "tokenizer.h"
#include "common.h"
#include <map>

namespace Tolo
{
	void Tokenize(const std::string& code, std::vector<Token>& tokens)
	{
		static std::map<char, Token::Type> singleToType
		{
			{'(', Token::Type::StartPar},
			{')', Token::Type::EndPar},
			{'{', Token::Type::StartCurly},
			{'}', Token::Type::EndCurly},
			{';', Token::Type::Semicolon},
			{',', Token::Type::Comma},
			{'.', Token::Type::Dot},
			{'+', Token::Type::Plus},
			{'-', Token::Type::Minus},
			{'*', Token::Type::Asterisk},
			{'/', Token::Type::ForwardSlash},
			{'<', Token::Type::LeftArrow},
			{'>', Token::Type::RightArrow},
			{'=', Token::Type::EqualSign},
			{'!', Token::Type::ExclamationMark},
			{'~', Token::Type::Tilde},
			{'^', Token::Type::Caret},
			{'&', Token::Type::Ampersand},
			{'|', Token::Type::VerticalBar}
		};

		static std::map<std::string, Token::Type> doubleToType
		{
			{"<=", Token::Type::LeftArrowEqualSign},
			{">=", Token::Type::RightArrowEqualSign},
			{"==", Token::Type::DoubleEqualSign},
			{"!=", Token::Type::ExclamationMarkEqualSign},
			{"<<", Token::Type::DoubleLeftArrow},
			{">>", Token::Type::DoubleRightArrow},
			{"&&", Token::Type::DoubleAmpersand},
			{"||", Token::Type::DoubleVerticalBar}
		};

		int line = 1;

		for (size_t i = 0; i < code.size();)
		{
			char c = code[i];
			std::string doubleStr;
			doubleStr += c;
			if (i + 1 < code.size())
				doubleStr += code[i + 1];

			if (c == ' ' || c == '\n' || c == '\t')
			{
				if (c == '\n')
					line++;

				i++;
			}
			else if (doubleStr == "//")
			{
				i += 2;
				for (; i < code.size() && code[i] != '\n'; i++);
				if (code[i] == '\n')
					line++;

				i++;
			}
			else if (doubleStr == "/*")
			{
				i += 2;
				for (; i + 1 < code.size() && !(code[i] == '*' && code[i + 1] == '/'); i++)
				{
					if (code[i] == '\n')
						line++;
				}

				Affirm(
					i + 1 < code.size() && code[i] == '*' && code[i + 1] == '/',
					"missing '*/' at line %i", 
					line
				);

				i += 2;
			}
			else if (doubleToType.count(doubleStr) != 0)
			{
				tokens.push_back({ doubleToType[doubleStr], doubleStr, line });
				i += 2;
			}
			else if (singleToType.count(c) != 0)
			{
				std::string charStr;
				charStr += c;
				tokens.push_back({ singleToType[c], charStr, line });
				i++;
			}
			else if (c == '\'')
			{
				Affirm(i + 2 < code.size(), "unexpected token ['] at line %i", line);

				std::string charStr;

				// check for escape character
				if (i + 3 < code.size() && code[i + 3] == '\'' && code[i + 1] == '\\')
				{
					char ec = code[i + 2];
					if (ec == '\'')
						charStr += '\'';
					else if (ec == '\\')
						charStr += '\\';
					else if (ec == 'n')
						charStr += '\n';
					else if (ec == 't')
						charStr += '\t';
					else if (ec == '0')
						charStr += '\0';
					else
						Affirm(false, "invalid escape character '%c' at line %i", ec, line);

					i += 4;
				}
				else
				{
					Affirm(i + 2 < code.size() && code[i + 2] == '\'', "missing ['] at line %i", line);
					charStr += code[i + 1];
					i += 3;
				}
				tokens.push_back({ Token::Type::ConstChar, charStr, line });
			}
			else if (c == '"')
			{
				std::string str;
				i++;
				bool foundEndQuote = false;

				for (; i < code.size(); i++)
				{
					c = code[i];
					Affirm(c != '\n', "unexpected newline at line %i", line);

					if (c == '"')
					{
						foundEndQuote = true;
						i++;
						break;
					}
					else if (c == '\\')
					{
						Affirm(i + 1 < code.size(), "unexpected end of tokens at line %i", line);

						char ec = code[i + 1];
						if (ec == '\'')
							str += '\'';
						else if (ec == '\\')
							str += '\\';
						else if (ec == 'n')
							str += '\n';
						else if (ec == 't')
							str += '\t';
						else if (ec == '0')
							str += '\0';
						else
							Affirm(false, "invalid escape character '%c' at line %i", ec, line);

						i++;
					}
					else
						str += c;
				}

				Affirm(foundEndQuote, "missing '\"' at line %i", line);

				tokens.push_back({ Token::Type::ConstString, str, line });
			}
			else if (c >= '0' && c <= '9')
			{
				std::string numStr;
				numStr += c;
				i++;

				bool hasDot = false;

				for (; i < code.size(); i++)
				{
					c = code[i];
					if (c == '.')
					{
						Affirm(!hasDot, "unexpected [.] at line %i", line);
						hasDot = true;
						numStr += c;
					}
					else if (c < '0' || c > '9')
						break;
					else
						numStr += c;
				}

				tokens.push_back({ hasDot ? Token::Type::ConstFloat : Token::Type::ConstInt, numStr, line });
			}
			else
			{
				std::string name;
				name += c;
				i++;

				for (; i < code.size(); i++)
				{
					c = code[i];
					if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')
						name += c;
					else
						break;
				}

				tokens.push_back({ Token::Type::Name, name, line });
			}
		}
	}
}