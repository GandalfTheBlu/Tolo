#pragma once
#include "common.h"
#include <map>
#include <string>
#include <vector>

namespace Tolo
{
	struct CodeBuilder
	{
		Ptr p_stack;
		std::map<std::string, Ptr> constStringToIp;
		Int stackSize;
		Int constStringCapacity;
		Ptr p_nextConstStringIp;
		Int codeLength;
		std::map<std::string, Ptr> labelNameToLabelIp;
		std::map<std::string, std::vector<Int>> labelNameToStackOffsets;
		Int currentBranchDepth;
		Int currentWhileDepth;

		CodeBuilder(Ptr _p_stack, Int _stackSize, Int _constStringCapacity);

		void Op(OpCode val);

		void ConstChar(Char val);

		void ConstInt(Int val);

		void ConstFloat(Float val);

		void ConstStringPtr(const std::string& val);

		void ConstPtr(Ptr p_val);

		void ConstPtrToLabel(const std::string& labelName);

		void DefineLabel(const std::string& labelName);

		void RemoveLabel(const std::string& labelName);
	};
}