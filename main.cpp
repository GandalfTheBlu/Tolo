#include "src/program_handle.h"
#include "src/file_io.h"
#include <iostream>

using namespace Tolo;

template<typename T>
void SetValue(VirtualMachine& vm)
{
	Ptr ptr = Pop<Ptr>(vm);
	Int index = Pop<Int>(vm);
	T value = Pop<T>(vm);

	Char* pData = reinterpret_cast<Char*>(ptr);
	*reinterpret_cast<T*>(pData + index) = value;
}

template<typename T>
void GetValue(VirtualMachine& vm)
{
	Ptr ptr = Pop<Ptr>(vm);
	Int index = Pop<Int>(vm);

	Char* pData = reinterpret_cast<Char*>(ptr);
	Push<T>(vm, *reinterpret_cast<T*>(pData + index));
}

template<typename T>
void PrintValue(VirtualMachine& vm)
{
	T value = Pop<T>(vm);
	std::cout << value;
}

int main()
{
	ProgramHandle handle("Script/ptr_test.tolo", 1024);
	handle.AddFunction({ "ptr", "malloc", {"int"}, [](VirtualMachine& vm)
		{
			Int size = Pop<Int>(vm);
			Push<Ptr>(vm, reinterpret_cast<Ptr>(std::malloc(size)));
		} 
	});
	handle.AddFunction({ "void", "free", {"ptr"}, [](VirtualMachine& vm)
		{
			Ptr ptr = Pop<Ptr>(vm);
			std::free(reinterpret_cast<void*>(ptr));
		}
	});
	handle.AddFunction({ "ptr", "null", {}, [](VirtualMachine& vm)
		{
			Push<Ptr>(vm, 0);
		}
	});

	handle.AddStruct({ "string",
		{
			{"ptr", "data"},
			{"int", "size"},
			{"int", "capacity"}
		}
	});

	struct ToloString
	{
		Ptr data;
		Int size;
		Int capacity;
	};

	handle.AddFunction({ "string", "operator+", {"string", "char"}, [](VirtualMachine& vm)
		{
			ToloString str = Pop<ToloString>(vm);
			Char value = Pop<Char>(vm);

			if (str.size + 1 > str.capacity)
			{
				if (str.capacity < 2)
					str.capacity = 2;

				str.capacity *= 3;
				str.capacity /= 2;

				Char* pOldData = reinterpret_cast<Char*>(str.data);
				Char* pNewData = static_cast<Char*>(std::malloc(str.capacity));

				for (size_t i = 0; i < str.size; i++)
					*(pNewData + i) = *(pOldData + i);

				std::free(pOldData);
				str.data = reinterpret_cast<Ptr>(pNewData);
			}

			Char* pData = reinterpret_cast<Char*>(str.data);
			pData[str.size++] = value;

			PushStruct<ToloString>(vm, str);
		}
	});

	handle.AddFunction({ "void", "print_char", {"char"}, PrintValue<Char> });
	handle.AddFunction({ "void", "print_int", {"int"}, PrintValue<Int> });
	handle.AddFunction({ "void", "print_float", {"float"}, PrintValue<Float> });
	handle.AddFunction({ "void", "print_ptr", {"ptr"}, PrintValue<Ptr> });
	handle.AddFunction({ "void", "print_string", {"string"}, [](VirtualMachine& vm)
		{
			ToloString str = Pop<ToloString>(vm);
			const Char* pData = reinterpret_cast<const Char*>(str.data);
			if(pData != nullptr)
				std::cout << pData;
		}
	});

	handle.AddFunction({ "void", "set_char", {"ptr", "int", "char"}, SetValue<Char> });
	handle.AddFunction({ "void", "set_int", {"ptr", "int"}, SetValue<Int> });
	handle.AddFunction({ "void", "set_float", {"ptr", "float"}, SetValue<Float> });
	handle.AddFunction({ "void", "set_ptr", {"ptr", "ptr"}, SetValue<Ptr> });

	handle.AddFunction({ "char", "get_char", {"ptr", "int"}, GetValue<Char> });
	handle.AddFunction({ "int", "get_int", {"ptr", "int"}, GetValue<Int> });
	handle.AddFunction({ "float", "get_float", {"ptr", "int"}, GetValue<Float> });
	handle.AddFunction({ "ptr", "get_ptr", {"ptr", "int"}, GetValue<Ptr> });
	
	handle.Compile();
	handle.Execute<void>();

	return 0;
}