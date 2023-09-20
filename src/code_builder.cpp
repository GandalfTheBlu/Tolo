#include "code_builder.h"

namespace Tolo
{
	CodeBuilder::CodeBuilder(Char* _p_data) :
		p_data(_p_data),
		codeLength(0)
	{}

	void CodeBuilder::Op(OpCode val)
	{
		*(Char*)(p_data + codeLength) = (Char)val;
		codeLength += sizeof(Char);
	}

	void CodeBuilder::ConstChar(Char val)
	{
		*(Char*)(p_data + codeLength) = val;
		codeLength += sizeof(Char);
	}

	void CodeBuilder::ConstInt(Int val)
	{
		*(Int*)(p_data + codeLength) = val;
		codeLength += sizeof(Int);
	}

	void CodeBuilder::ConstFloat(Float val)
	{
		*(Float*)(p_data + codeLength) = val;
		codeLength += sizeof(Float);
	}

	void CodeBuilder::ConstPtr(Ptr val)
	{
		*(Ptr*)(p_data + codeLength) = val;
		codeLength += sizeof(Ptr);
	}

	void CodeBuilder::ConstPtrToLabel(const std::string& labelName)
	{
		if (labelNameToLabelIp.count(labelName) != 0)
			*(Ptr*)(p_data + codeLength) = labelNameToLabelIp[labelName];
		else
			labelNameToRefIps[labelName].push_back(codeLength);

		codeLength += sizeof(Ptr);
	}

	void CodeBuilder::DefineLabel(const std::string& labelName)
	{
		labelNameToLabelIp[labelName] = codeLength;

		std::vector<Ptr>& refIps = labelNameToRefIps[labelName];

		for (Ptr refIp : refIps)
			*(Ptr*)(p_data + refIp) = codeLength;
	}

	void CodeBuilder::RemoveLabel(const std::string& labelName)
	{
		labelNameToLabelIp.erase(labelName);
		labelNameToRefIps.erase(labelName);
	}
}