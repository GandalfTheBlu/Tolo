#pragma once
#include "common.h"
#include <string>

namespace Tolo
{
	void ReadTextFile(const std::string& filePath, std::string& outText);

	void Compile(const std::string& codePath, Char* p_programMemory, Ptr& outCodeLength);
}