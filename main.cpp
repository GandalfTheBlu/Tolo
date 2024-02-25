#include "src/program_handle.h"

using namespace Tolo;

int main()
{
	ProgramHandle program("Script/ptr_test.tolo", 1024, 128);

	program.Compile();
	program.Execute<void>();

	return 0;
}