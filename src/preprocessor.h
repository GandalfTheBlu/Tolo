#pragma once
#include <string>
#include <map>

namespace Tolo
{
	void Preprocess(
		const std::string& inCode, 
		std::string& outCode, 
		std::map<std::string, bool>& inoutStandardIncludeFlags
	);
}