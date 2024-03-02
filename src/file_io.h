#pragma once
#include <string>

namespace Tolo
{
	void ReadTextFile(const std::string& filePath, std::string& outText);
	bool WriteTextFile(const std::string& path, const std::string& text, bool append);
}