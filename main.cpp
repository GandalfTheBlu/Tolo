#include "src/program_handle.h"
#include "src/file_io.h"
#include <iostream>

using namespace Tolo;


int main()
{
	
	ProgramHandle handle("Script/void_test.tolo", 1024);
	handle.AddFunction({ "void", "print_int", {"int"}, [](VirtualMachine& vm) 
		{
			Int arg = Pop<Int>(vm);
			std::cout << arg << std::endl;
		} 
	});
	handle.Compile();
	handle.Execute<void>();

	return 0;
}