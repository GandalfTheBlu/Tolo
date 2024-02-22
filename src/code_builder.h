#pragma once
#include "common.h"
#include <map>
#include <string>
#include <vector>

namespace Tolo
{
	struct CodeBuilder
	{
		Char* p_stack;
		std::map<std::string, Ptr> constStringToIp;
		Ptr stackSize;
		Ptr constStringCapacity;
		Ptr nextConstStringIp;
		Ptr codeLength;
		std::map<std::string, Ptr> labelNameToLabelIp;
		std::map<std::string, std::vector<Ptr>> labelNameToRefIps;
		Int currentBranchDepth;
		Int currentWhileDepth;

		CodeBuilder(Char* _p_stack, Ptr _stackSize, Ptr _constStringCapacity);

		void Op(OpCode val);

		void ConstChar(Char val);

		void ConstInt(Int val);

		void ConstFloat(Float val);

		void ConstStringPtr(const std::string& val);

		void ConstPtr(Ptr val);

		void ConstPtrToLabel(const std::string& labelName);

		void DefineLabel(const std::string& labelName);

		void RemoveLabel(const std::string& labelName);
	};
}