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

	bool WriteTextFile(const std::string& path, const std::string& text, bool append)
	{
		std::ofstream file;
		if (append)
			file.open(path, std::ios::app);
		else
			file.open(path);

		Affirm(file.is_open(), "failed to open file '%s'", path.c_str());

		file << text;

		file.close();
		return true;
	}
}