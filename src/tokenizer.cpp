#include "tokenizer.h"
#include "common.h"

namespace Tolo
{
	void Tokenize(const std::string& code, std::vector<Token>& tokens)
	{
		int line = 1;

		for (size_t i = 0; i < code.size();)
		{
			char c = code[i];

			if (c == ' ' || c == '\n' || c == '\t')
			{
				if (c == '\n')
					line++;

				i++;
			}
			else if (c == '#')
			{
				for (; i < code.size() && code[i] != '\n'; i++);
				if (code[i] == '\n')
					line++;

				i++;
			}
			else if (c == '(')
			{
				tokens.push_back({ Token::Type::StartPar, "(", line });
				i++;
			}
			else if (c == ')')
			{
				tokens.push_back({ Token::Type::EndPar, ")", line });
				i++;
			}
			else if (c == '{')
			{
				tokens.push_back({ Token::Type::StartCurly, "{", line });
				i++;
			}
			else if (c == '}')
			{
				tokens.push_back({ Token::Type::EndCurly, "}", line });
				i++;
			}
			else if (c == ',')
			{
				tokens.push_back({ Token::Type::Comma, ",", line });
				i++;
			}
			else if (c == '.')
			{
				tokens.push_back({ Token::Type::Dot, ".", line });
				i++;
			}
			else if (c == '=')
			{
				tokens.push_back({ Token::Type::EqualSign, "=", line });
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
					else if (ec >= '0' && ec <= '9')
						charStr += ('\0' + ec - '0');
					else
						Affirm(false, "invalid escape character [%c] at line %i", ec, line);

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