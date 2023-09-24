#pragma once
#include "common.h"
#include "parser.h"
#include <string>
#include <vector>
#include <map>

namespace Tolo
{
	void ReadTextFile(const std::string& filePath, std::string& outText);

	void CompileProgram(const std::string& codePath, Char* p_stack, const std::map<std::string, NativeFunctionInfo>& nativeFunctions, Ptr& outCodeStart, Ptr& outCodeEnd, Int& outMainRetValByteSize);
}