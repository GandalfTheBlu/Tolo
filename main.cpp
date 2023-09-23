#include "src/program_handle.h"
#include <cmath>

using namespace Tolo;

struct vec3
{
	float x, y, z;
};

float length(const vec3& v)
{
	return std::sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
}

vec3 normalize(const vec3& v)
{
	float s = 1.f / length(v);
	return vec3{ v.x * s, v.y * s, v.z * s };
}

vec3 add(const vec3& a, const vec3& b)
{
	return vec3{ a.x + b.x, a.y + b.y, a.z + b.z };
}

vec3 mul(const vec3& a, float s)
{
	return vec3{ a.x * s, a.y * s, a.z * s };
}

float dot(const vec3& a, const vec3& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

namespace Test
{
	float march(vec3 origin, vec3 dir, ProgramHandle& program)
	{
		float t = 0.0;
		int i = 0;
		while (i < 32)
		{
			//float r = sdf(add(origin, mul(dir, t)));
			float r = program.Execute<float>(add(origin, mul(dir, t)));

			if (r < 0.001)
			{
				break;
			}

			t = t + r;
			i = i + 1;
		}

		return t;
	}

	void RayMarch(ProgramHandle& program)
	{
		std::string grid;
		size_t width = 40;
		size_t height = 40;
		grid.resize((2 * width + 1) * height, ' ');

		vec3 origin{ 0.f, 0.f, -3.f };
		vec3 toLight = normalize(vec3{ 1.f, 1.f, -1.f });

		char gradient[3] = { '.', '*', '@' };

		size_t index = 0;
		for (size_t y = 0; y < height; y++)
		{
			for (size_t x = 0; x < width; x++)
			{
				float u = (float)x / (width - 1);
				float v = 1.f - (float)y / (height - 1);
				float vx = 2.f * u - 1.f;
				float vy = 2.f * v - 1.f;
				vec3 dir = normalize(vec3{ vx, vy, 2.f });

				float t = Test::march(origin, dir, program);

				if (t > 20.f)
					grid[index] = ' ';
				else
				{
					vec3 point = add(origin, mul(dir, t));
					vec3 norm = normalize(point);
					float dif = std::max(dot(norm, toLight), 0.f);

					grid[index] = gradient[std::min(int(dif * 3.f), 2)];
				}

				index += 2;
			}
			grid[index++] = '\n';
		}

		std::printf("%s", grid.c_str());
	}
}

int main()
{
	ProgramHandle program("Script/test.tolo", 1024);
	program.Compile();

	Test::RayMarch(program);

	return 0;
}