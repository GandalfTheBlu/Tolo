#include "src/program_handle.h"
#include "src/pointer_toolkit.h"
#include "src/file_io.h"
#include <iostream>

using namespace Tolo;

const char* GetString(VirtualMachine& vm)
{
	Ptr p_str = Pop<Ptr>(vm);
	return static_cast<const char*>(p_str);
}

int main()
{
	ProgramHandle program("Script/ptr_test.tolo", 1024, 128);
	
	AddPointerToolkit(program);

	program.AddFunction({ "void", "print", {"ptr"}, [](VirtualMachine& vm)
		{
			std::cout << GetString(vm);
		}
	});

	program.Compile();
	program.Execute<void>();

	return 0;
}