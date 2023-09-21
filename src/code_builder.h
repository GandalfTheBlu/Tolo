#pragma once
#include "common.h"
#include <map>
#include <string>
#include <vector>

namespace Tolo
{
	struct CodeBuilder
	{
		Char* p_data;
		Ptr codeLength;
		std::map<std::string, Ptr> labelNameToLabelIp;
		std::map<std::string, std::vector<Ptr>> labelNameToRefIps;
		Int currentBranchDepth;
		Int currentWhileDepth;

		CodeBuilder(Char* _p_data);

		void Op(OpCode val);

		void ConstChar(Char val);

		void ConstInt(Int val);

		void ConstFloat(Float val);

		void ConstPtr(Ptr val);

		void ConstPtrToLabel(const std::string& labelName);

		void DefineLabel(const std::string& labelName);

		void RemoveLabel(const std::string& labelName);
	};
}