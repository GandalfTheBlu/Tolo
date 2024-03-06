#include "standard_toolkit.h"
#include "file_io.h"
#include <iostream>

namespace Tolo
{
	template<typename T>
	void Print(VirtualMachine& vm)
	{
		T val = Pop<T>(vm);
		std::cout << val;
	}

	template<typename T>
	void Input(VirtualMachine& vm)
	{
		T val = T();
		std::cin >> val;

		Push<T>(vm, val);
	}

	void AddMemoryToolkit(ProgramHandle& program)
	{
		program.AddFunction("ptr", "malloc", {"int"}, [](VirtualMachine& vm)
			{
				Int size = Pop<Int>(vm);
				Affirm(size > 0, "failed to malloc, size was %i", size);

				Ptr p_data = static_cast<Ptr>(std::malloc(static_cast<size_t>(size)));
				Affirm(p_data != nullptr, "failed to malloc, size was %i", size);

				Push<Ptr>(vm, p_data);
			}
		);
		program.AddFunction("void", "free", {"ptr"}, [](VirtualMachine& vm)
			{
				Ptr p_data = Pop<Ptr>(vm);
				std::free(p_data);
			}
		);
		program.AddFunction("int", "strlen", {"ptr"}, [](VirtualMachine& vm)
			{
				Ptr p_str = Pop<Ptr>(vm);
				size_t len = std::strlen(static_cast<const char*>(p_str));
				Push<Int>(vm, static_cast<Int>(len));
			}
		);
		program.AddFunction("void", "memcpy", {"ptr", "ptr", "int"}, [](VirtualMachine& vm)
			{
				Ptr p_dst = Pop<Ptr>(vm);
				Ptr p_src = Pop<Ptr>(vm);
				Int size = Pop<Int>(vm);
				std::memcpy(p_dst, p_src, static_cast<size_t>(size));
			}
		);
		program.AddFunction("void", "memset", {"ptr", "char", "int"}, [](VirtualMachine& vm)
			{
				Ptr p_data = Pop<Ptr>(vm);
				Char val = Pop<Char>(vm);
				Int count = Pop<Int>(vm);
				std::memset(p_data, val, static_cast<size_t>(count));
			}
		);
		program.AddFunction("char", "cast", { "int" }, [](VirtualMachine& vm) 
			{
				Int val = Pop<Int>(vm);
				Push<Char>(vm, static_cast<Char>(val));
			}
		);
		program.AddFunction("char", "cast", { "float" }, [](VirtualMachine& vm)
			{
				Float val = Pop<Float>(vm);
				Push<Char>(vm, static_cast<Char>(val));
			}
		);
		program.AddFunction("int", "cast", { "char" }, [](VirtualMachine& vm)
			{
				Char val = Pop<Char>(vm);
				Push<Int>(vm, static_cast<Int>(val));
			}
		);
		program.AddFunction("int", "cast", { "float" }, [](VirtualMachine& vm)
			{
				Float val = Pop<Float>(vm);
				Push<Int>(vm, static_cast<Int>(val));
			}
		);
		program.AddFunction("float", "cast", { "char" }, [](VirtualMachine& vm)
			{
				Char val = Pop<Char>(vm);
				Push<Float>(vm, static_cast<Float>(val));
			}
		);
		program.AddFunction("float", "cast", { "int" }, [](VirtualMachine& vm)
			{
				Int val = Pop<Int>(vm);
				Push<Float>(vm, static_cast<Float>(val));
			}
		);
	}

	void AddIOToolkit(ProgramHandle& program)
	{
		program.AddFunction("void", "print", {"ptr"}, [](VirtualMachine& vm)
			{
				Ptr p_str = Pop<Ptr>(vm);
				std::cout << static_cast<const char*>(p_str);
			}
		);
		program.AddFunction("void", "print", {"char"}, Print<Char>);
		program.AddFunction("void", "print", {"int"}, Print<Int>);
		program.AddFunction("void", "print", {"float"}, Print<Float>);

		program.AddFunction("void", "input", {"ptr", "int"}, [](VirtualMachine& vm)
			{
				Ptr p_str = Pop<Ptr>(vm);
				size_t capacity = static_cast<size_t>(Pop<Int>(vm));

				std::string input;
				std::getline(std::cin, input);

				for (size_t i = 0; i < input.size() && i < capacity; i++)
					p_str[i] = input[i];
			}
		);
		program.AddFunction("char", "input", {},  Input<Char>);
		program.AddFunction("int", "input", {},  Input<Int>);
		program.AddFunction("float", "input", {},  Input<Float>);

		program.AddFunction("void", "get_file_txt", {"ptr", "ptr", "int"}, [](VirtualMachine& vm)
			{
				const char* p_filepath = static_cast<const char*>(Pop<Ptr>(vm));
				Ptr p_buffer = Pop<Ptr>(vm);
				size_t capacity = static_cast<size_t>(Pop<Int>(vm));

				std::string text;
				ReadTextFile(p_filepath, text);

				for (size_t i = 0; i < text.size() && i < capacity; i++)
					p_buffer[i] = text[i];
			}
		);
		program.AddFunction("void", "set_file_txt", {"ptr", "ptr"}, [](VirtualMachine& vm)
			{
				const char* p_filepath = static_cast<const char*>(Pop<Ptr>(vm));
				const char* p_str = static_cast<const char*>(Pop<Ptr>(vm));

				WriteTextFile(p_filepath, p_str, false);
			}
		);
		program.AddFunction("void", "add_file_txt", {"ptr", "ptr"}, [](VirtualMachine& vm)
			{
				const char* p_filepath = static_cast<const char*>(Pop<Ptr>(vm));
				const char* p_str = static_cast<const char*>(Pop<Ptr>(vm));

				WriteTextFile(p_filepath, p_str, true);
			}
		);
	}
}