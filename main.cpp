#include "src/program_handle.h"

using namespace Tolo;

int main()
{
	ProgramHandle program(
		"Script/method_test.tolo", 
		16 * 1024, 
		128,
		"void", "main", {}
	);

	program.Compile();
	program.Execute<void>();

	return 0;
}