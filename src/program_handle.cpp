#include "program_handle.h"
#include "compiler.h"

namespace Tolo
{
	ProgramHandle::ProgramHandle(const std::string& _codePath, Ptr stackSize) :
		codePath(_codePath),
		codeStart(0),
		codeEnd(0),
		mainReturnValueSize(0)
	{
		p_stack = (Char*)std::malloc(stackSize);
	}

	ProgramHandle::~ProgramHandle()
	{
		std::free(p_stack);
	}

	void ProgramHandle::Compile()
	{
		CompileProgram(codePath, p_stack, codeStart, codeEnd, mainReturnValueSize);
	}
}