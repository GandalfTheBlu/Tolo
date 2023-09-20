





#include "src/compiler.h"
#include "src/virtual_machine.h"

using namespace Tolo;

int main()
{
	Char p_programMemory[1024];
	Ptr codeLength = 0;

	Compile("Script/test.tolo", p_programMemory, codeLength);
	RunProgram(p_programMemory, codeLength);

	std::printf("\n");

	return 0;
}