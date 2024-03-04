#include "src/program_handle.h"

using namespace Tolo;

int main()
{
	ProgramHandle program(
		"Script/method_test.tolo", 
		1024, 
		128,
		"void", "main", {}
	);

	program.AddStruct("test_struct", {{"int", "memb"}});

	program.Compile();
	program.Execute<void>();

	return 0;
}