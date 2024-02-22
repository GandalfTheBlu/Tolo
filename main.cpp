#include "src/program_handle.h"
#include "src/file_io.h"
#include <iostream>

using namespace Tolo;

const char* GetConstString(VirtualMachine& vm)
{
	Ptr strPtr = Pop<Ptr>(vm);
	return static_cast<const char*>(vm.p_stack + strPtr);
}

int main()
{
	ProgramHandle handle("Script/ptr_test.tolo", 1024, 128);
	
	handle.AddFunction({ "void", "print_string", {"ptr"}, [](VirtualMachine& vm)
		{
			std::cout << GetConstString(vm);
		}
	});
	
	handle.Compile();
	handle.Execute<void>();

	return 0;
}