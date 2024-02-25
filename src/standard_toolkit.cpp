#include "standard_toolkit.h"
#include <iostream>

namespace Tolo
{
	template<typename T>
	void SetValue(VirtualMachine& vm)
	{
		Ptr ptr = Pop<Ptr>(vm);
		T value = Pop<T>(vm);

		char* p_ptr = reinterpret_cast<char*>(ptr);
		*reinterpret_cast<T*>(p_ptr) = value;
	}

	template<typename T>
	void GetValue(VirtualMachine& vm)
	{
		Ptr ptr = Pop<Ptr>(vm);

		char* p_ptr = reinterpret_cast<char*>(ptr);
		T value = *reinterpret_cast<T*>(p_ptr);
		Push<T>(vm, value);
	}

	void AddMemoryToolkit(ProgramHandle& program)
	{
		program.AddFunction({ "ptr", "malloc", {"int"}, [](VirtualMachine& vm)
			{
				Int size = Pop<Int>(vm);
				Affirm(size > 0, "failed to malloc, size was %i", size);

				Ptr p_data = static_cast<Ptr>(std::malloc(static_cast<size_t>(size)));
				Affirm(p_data != nullptr, "failed to malloc, size was %i", size);

				Push<Ptr>(vm, p_data);
			}
		});
		program.AddFunction({ "void", "free", {"ptr"}, [](VirtualMachine& vm)
			{
				Ptr p_data = Pop<Ptr>(vm);
				std::free(p_data);
			}
		});
		program.AddFunction({ "void", "set_char", {"ptr", "char"}, SetValue<Char>});
		program.AddFunction({ "void", "set_int", {"ptr", "int"}, SetValue<Int>});
		program.AddFunction({ "void", "set_float", {"ptr", "float"}, SetValue<Float>});
		program.AddFunction({ "void", "set_ptr", {"ptr", "ptr"}, SetValue<Ptr>});
		program.AddFunction({ "char", "get_char", {"ptr"}, GetValue<Char> });
		program.AddFunction({ "int", "get_int", {"ptr"}, GetValue<Int> });
		program.AddFunction({ "float", "get_float", {"ptr"}, GetValue<Float> });
		program.AddFunction({ "ptr", "get_ptr", {"ptr"}, GetValue<Ptr> });
		program.AddFunction({ "int", "strlen", {"ptr"}, [](VirtualMachine& vm)
			{
				Ptr p_str = Pop<Ptr>(vm);
				size_t len = std::strlen(static_cast<const char*>(p_str));
				Push<Int>(vm, static_cast<Int>(len));
			}
		});
		program.AddFunction({ "void", "memcpy", {"ptr", "ptr", "int"}, [](VirtualMachine& vm)
			{
				Ptr p_dst = Pop<Ptr>(vm);
				Ptr p_src = Pop<Ptr>(vm);
				Int size = Pop<Int>(vm);
				std::memcpy(p_dst, p_src, static_cast<size_t>(size));
			}
		});
	}

	void AddIOToolkit(ProgramHandle& program)
	{
		program.AddFunction({ "void", "print", {"ptr"}, [](VirtualMachine& vm)
			{
				Ptr p_str = Pop<Ptr>(vm);
				std::cout << static_cast<const char*>(p_str);
			}
		});

		// todo: add input and file io
	}
}