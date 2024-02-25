#include "preprocessor.h"
#include "file_io.h"
#include <regex>
#include <set>

namespace Tolo
{
	void RecursiveApplyIncludes(
		const std::string& inCode, 
		std::string& outCode,
		std::set<std::string>& inoutIncludePaths
	)
	{
		std::regex rgx("(?:^|\\n)#include\\s\"([\\S\\s]+?)\"");
		std::smatch match;
		auto start = inCode.cbegin();
		bool hasInclude = false;

		while (std::regex_search(start, inCode.cend(), match, rgx))
		{
			hasInclude = true;
			std::string includePath = match[1];

			outCode += match.prefix();

			if (inoutIncludePaths.count(includePath) == 0)
			{
				inoutIncludePaths.insert(includePath);
				std::string includeCode;
				ReadTextFile(includePath, includeCode);
				RecursiveApplyIncludes(includeCode, outCode, inoutIncludePaths);
			}

			start = match.suffix().first;
		}

		if (!hasInclude)
			outCode += inCode;
		else
			outCode += match.suffix();
	}

	void Preprocess(const std::string& inCode, std::string& outCode)
	{
		std::set<std::string> includePaths;
		RecursiveApplyIncludes(inCode, outCode, includePaths);
	}
}