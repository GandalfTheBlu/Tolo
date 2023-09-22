#include "src/program_handle.h"

using namespace Tolo;

struct vec3
{
	float x, y, z;
};

int main()
{
	ProgramHandle program("Script/test.tolo", 1024);
	program.Compile();

	float dotProduct = program.Execute<float>(1.f, 2.f, 3.f, 4.f, 5.f, 6.f);// = 32

	std::printf("\nprogram returned with %f\n", dotProduct);

	return 0;
}