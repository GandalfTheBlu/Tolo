#pragma once
#include "common.h"
#include "expression.h"
#include <string>
#include <vector>

namespace Tolo
{
	void ReadTextFile(const std::string& filePath, std::string& outText);

	void CompileProgram(const std::string& codePath, Char* p_stack, Ptr& outCodeLength, const std::string& expectedMainReturnType, const std::vector<Expression*>& mainArgumentLoaders);
}