#include "code_builder.h"

namespace Tolo
{
	CodeBuilder::CodeBuilder(Char* _p_stack) :
		p_stack(_p_stack),
		codeLength(0),
		currentBranchDepth(0),
		currentWhileDepth(0)
	{}

	void CodeBuilder::Op(OpCode val)
	{
		*(Char*)(p_stack + codeLength) = (Char)val;
		codeLength += sizeof(Char);
	}

	void CodeBuilder::ConstChar(Char val)
	{
		*(Char*)(p_stack + codeLength) = val;
		codeLength += sizeof(Char);
	}

	void CodeBuilder::ConstInt(Int val)
	{
		*(Int*)(p_stack + codeLength) = val;
		codeLength += sizeof(Int);
	}

	void CodeBuilder::ConstFloat(Float val)
	{
		*(Float*)(p_stack + codeLength) = val;
		codeLength += sizeof(Float);
	}

	void CodeBuilder::ConstPtr(Ptr val)
	{
		*(Ptr*)(p_stack + codeLength) = val;
		codeLength += sizeof(Ptr);
	}

	void CodeBuilder::ConstPtrToLabel(const std::string& labelName)
	{
		if (labelNameToLabelIp.count(labelName) != 0)
			*(Ptr*)(p_stack + codeLength) = labelNameToLabelIp[labelName];
		else
			labelNameToRefIps[labelName].push_back(codeLength);

		codeLength += sizeof(Ptr);
	}

	void CodeBuilder::DefineLabel(const std::string& labelName)
	{
		labelNameToLabelIp[labelName] = codeLength;

		std::vector<Ptr>& refIps = labelNameToRefIps[labelName];

		for (Ptr refIp : refIps)
			*(Ptr*)(p_stack + refIp) = codeLength;
	}

	void CodeBuilder::RemoveLabel(const std::string& labelName)
	{
		labelNameToLabelIp.erase(labelName);
		labelNameToRefIps.erase(labelName);
	}
}