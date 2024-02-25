#include "preprocessor.h"
#include "common.h"
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
		// find include files
		std::regex includeFileRgx("(?:^|\\n)#include\\s\"([\\S\\s]+?)\"");
		std::smatch includeFileMatch;
		auto start = inCode.cbegin();
		bool hasInclude = false;

		while (std::regex_search(start, inCode.cend(), includeFileMatch, includeFileRgx))
		{
			hasInclude = true;
			std::string includePath = includeFileMatch[1];

			outCode += includeFileMatch.prefix();

			if (inoutIncludePaths.count(includePath) == 0)
			{
				inoutIncludePaths.insert(includePath);
				std::string includeCode;
				ReadTextFile(includePath, includeCode);
				RecursiveApplyIncludes(includeCode, outCode, inoutIncludePaths);
			}

			start = includeFileMatch.suffix().first;
		}

		if (!hasInclude)
			outCode += inCode;
		else
			outCode += includeFileMatch.suffix();
	}

	void Preprocess(
		const std::string& inCode, 
		std::string& outCode,
		std::map<std::string, bool>& inoutStandardIncludeFlags
	)
	{
		std::string allCode;
		std::set<std::string> includePaths;
		RecursiveApplyIncludes(inCode, allCode, includePaths);

		// find include flags
		std::regex includeFlagRgx("(?:^|\\n)#include\\s<([\\S\\s]+?)>");
		std::smatch includeFlagMatch;
		auto start = allCode.cbegin();
		bool hasInclude = false;

		while (std::regex_search(start, allCode.cend(), includeFlagMatch, includeFlagRgx))
		{
			hasInclude = true;
			std::string includeFlag = includeFlagMatch[1];

			outCode += includeFlagMatch.prefix();

			Affirm(
				inoutStandardIncludeFlags.count(includeFlag) != 0,
				"include '%s' is not part of the standard",
				includeFlag.c_str()
			);

			inoutStandardIncludeFlags.at(includeFlag) = true;

			start = includeFlagMatch.suffix().first;
		}

		if (!hasInclude)
			outCode += allCode;
		else
			outCode += includeFlagMatch.suffix();
	}
}