#pragma once
#include "compiler.h"
#include "virtual_machine.h"
#include "expression.h"
#include <vector>

namespace Tolo
{
	template<typename RETURN_TYPE>
	class ProgramHandle
	{
	private:
		std::string codePath;
		Char* p_stack;
		Ptr codeLength;
		Ptr stackSize;

		ProgramHandle() = delete;
		ProgramHandle(const ProgramHandle&) = delete;
		ProgramHandle& operator=(const ProgramHandle&) = delete;

	public:
		ProgramHandle(const std::string& _codePath, Ptr _stackSize) :
			codePath(_codePath),
			stackSize(_stackSize),
			codeLength(0)
		{
			p_stack = (Char*)std::malloc(stackSize);
		}

		~ProgramHandle()
		{
			std::free(p_stack);
		}

		void Compile()
		{
			std::vector<Expression*> argLoaders;
			argLoaders.push_back(new ELoadConstInt(1));
			CompileProgram(codePath, p_stack, codeLength, "int", argLoaders);
		}

		RETURN_TYPE Execute()
		{
			RunProgram(p_stack, codeLength);
			return *(RETURN_TYPE*)(p_stack + codeLength);
		}
	};
}