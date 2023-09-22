#include "src/program_handle.h"

using namespace Tolo;

int main()
{
	ProgramHandle<int> program("Script/test.tolo", 1024);
	program.Compile();
	int retVal = program.Execute();

	std::printf("\nprogram returned with state %i\n", retVal);

	return 0;
}