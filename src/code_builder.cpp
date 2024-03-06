#include "code_builder.h"

namespace Tolo
{
	CodeBuilder::CodeBuilder(Ptr _p_stack, Int _stackSize, Int _constStringCapacity) :
		p_stack(_p_stack),
		stackSize(_stackSize),
		constStringCapacity(_constStringCapacity),
		p_nextConstStringIp(_p_stack),
		codeLength(_constStringCapacity),
		currentBranchDepth(0),
		currentWhileDepth(0)
	{}

	void CodeBuilder::Op(OpCode val)
	{
		Affirm(codeLength + sizeof(Char) <= stackSize, "stack overflowed when building code");

		*(p_stack + codeLength) = static_cast<Char>(val);
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

		*reinterpret_cast<Int*>(p_stack + codeLength) = val;
		codeLength += sizeof(Int);
	}

	void CodeBuilder::ConstFloat(Float val)
	{
		Affirm(codeLength + sizeof(Float) <= stackSize, "stack overflowed when building code");

		*reinterpret_cast<Float*>(p_stack + codeLength) = val;
		codeLength += sizeof(Float);
	}

	void CodeBuilder::ConstStringPtr(const std::string& val)
	{
		Affirm(codeLength + sizeof(Ptr) <= stackSize, "stack overflowed when building code");

		if(constStringToIp.count(val) == 0)
		{
			Affirm(p_nextConstStringIp + val.size() + 1 <= p_stack + constStringCapacity, "const strings overflowed when building code");

			Int i = 0;
			for (; i < val.size(); i++)
				*(p_nextConstStringIp + i) = val[i];
			
			*(p_nextConstStringIp + i++) = '\0';

			constStringToIp[val] = p_nextConstStringIp;
			p_nextConstStringIp += i;
		}

		*reinterpret_cast<Ptr*>(p_stack + codeLength) = constStringToIp[val];
		codeLength += sizeof(Ptr);
	}

	void CodeBuilder::ConstPtr(Ptr p_val)
	{
		Affirm(codeLength + sizeof(Ptr) <= stackSize, "stack overflowed when building code");

		*reinterpret_cast<Ptr*>(p_stack + codeLength) = p_val;
		codeLength += sizeof(Ptr);
	}

	void CodeBuilder::ConstPtrToLabel(const std::string& labelName)
	{
		Affirm(codeLength + sizeof(Ptr) <= stackSize, "stack overflowed when building code");

		if (labelNameToLabelIp.count(labelName) != 0)
			*reinterpret_cast<Ptr*>(p_stack + codeLength) = labelNameToLabelIp[labelName];
		else
			labelNameToStackOffsets[labelName].push_back(codeLength);

		codeLength += sizeof(Ptr);
	}

	void CodeBuilder::DefineLabel(const std::string& labelName)
	{
		labelNameToLabelIp[labelName] = p_stack + codeLength;

		if (labelNameToStackOffsets.count(labelName) == 0)
			return;

		const std::vector<Int>& stackOffsets = labelNameToStackOffsets[labelName];

		for (Int offset : stackOffsets)
			*reinterpret_cast<Ptr*>(p_stack + offset) = p_stack + codeLength;

		labelNameToStackOffsets.erase(labelName);
	}

	void CodeBuilder::RemoveLabel(const std::string& labelName)
	{
		labelNameToLabelIp.erase(labelName);
	}

	bool CodeBuilder::HasUnresolvedLabels()
	{
		return labelNameToStackOffsets.size() > 0;
	}
}