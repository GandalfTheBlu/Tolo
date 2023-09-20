#pragma once
#include "token.h"
#include <vector>

namespace Tolo
{
	void Tokenize(const std::string& code, std::vector<Token>& tokens);
}