#include "file_io.h"
#include "common.h"
#include <fstream>
#include <sstream>

namespace Tolo
{
	void ReadTextFile(const std::string& filePath, std::string& outText)
	{
		std::ifstream file;
		file.open(filePath);

		Affirm(file.is_open(), "failed to open file '%s'", filePath.c_str());

		std::stringstream stringStream;
		stringStream << file.rdbuf();
		outText = stringStream.str();
		file.close();
	}
}