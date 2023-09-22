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

	float res = program.Execute<float>(vec3{ 1.f, 2.f, 3.f }, vec3{4.f, 5.f, 6.f});

	std::printf("\nprogram returned with %f\n", res);

	return 0;
}