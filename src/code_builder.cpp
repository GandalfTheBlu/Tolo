#include "code_builder.h"

namespace Tolo
{
	CodeBuilder::CodeBuilder(Char* _p_stack, Ptr _stackSize, Ptr _constStringCapacity) :
		p_stack(_p_stack),
		stackSize(_stackSize),
		constStringCapacity(_constStringCapacity),
		nextConstStringIp(0),
		codeLength(_constStringCapacity),
		currentBranchDepth(0),
		currentWhileDepth(0)
	{}

	void CodeBuilder::Op(OpCode val)
	{
		Affirm(codeLength + sizeof(Char) <= stackSize, "stack overflowed when building code");

		*(p_stack + codeLength) = (Char)val;
		codeLength += sizeof(Char);
	}

	void CodeBuilder::ConstChar(Char val)
	{
		Affirm(codeLength + sizeof(Char) <= stackSize, "stack overflowed when building code");

		*(p_stack + codeLength) = val;
		codeLength += sizeof(Char);
	}

	void CodeBuilder::ConstInt(Int val)
	{
		Affirm(codeLength + sizeof(Int) <= stackSize, "stack overflowed when building code");

		*(Int*)(p_stack + codeLength) = val;
		codeLength += sizeof(Int);
	}

	void CodeBuilder::ConstFloat(Float val)
	{
		Affirm(codeLength + sizeof(Float) <= stackSize, "stack overflowed when building code");

		*(Float*)(p_stack + codeLength) = val;
		codeLength += sizeof(Float);
	}

	void CodeBuilder::ConstStringPtr(const std::string& val)
	{
		Affirm(codeLength + sizeof(Ptr) <= stackSize, "stack overflowed when building code");

		if(constStringToIp.count(val) == 0)
		{
			Affirm(nextConstStringIp + val.size() + 1 <= constStringCapacity, "const strings overflowed when building code");

			Ptr i = 0;
			for (; i < val.size(); i++)
				*(p_stack + nextConstStringIp + i) = val[i];
			
			*(p_stack + nextConstStringIp + i++) = '\0';

			constStringToIp[val] = nextConstStringIp;
			nextConstStringIp += i;
		}

		*(Ptr*)(p_stack + codeLength) = constStringToIp[val];
		codeLength += sizeof(Ptr);
	}

	void CodeBuilder::ConstPtr(Ptr val)
	{
		Affirm(codeLength + sizeof(Ptr) <= stackSize, "stack overflowed when building code");

		*(Ptr*)(p_stack + codeLength) = val;
		codeLength += sizeof(Ptr);
	}

	void CodeBuilder::ConstPtrToLabel(const std::string& labelName)
	{
		Affirm(codeLength + sizeof(Ptr) <= stackSize, "stack overflowed when building code");

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