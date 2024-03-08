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

	program.AddStruct("parent", {{"int", "memb1"}});
	program.AddStructInherit("child", "parent", { {"int", "memb2"} });

	program.Compile();
	program.Execute<void>();

	return 0;
}