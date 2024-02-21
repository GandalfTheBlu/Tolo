#include "src/program_handle.h"
#include "src/file_io.h"
#include <iostream>

using namespace Tolo;

int main()
{
	ProgramHandle handle("Script/ptr_test.tolo", 1024);
	handle.AddFunction({ "ptr", "string_new", {}, [](VirtualMachine& vm)
		{
			Push<Ptr>(vm, reinterpret_cast<Ptr>(new std::string()));
		} 
	});
	handle.AddFunction({ "void", "string_delete", {"ptr"}, [](VirtualMachine& vm)
		{
			std::string* p_string = reinterpret_cast<std::string*>(Pop<Ptr>(vm));
			delete p_string;
		}
	});
	handle.AddFunction({ "void", "string_print", {"ptr"}, [](VirtualMachine& vm)
		{
			std::string* p_string = reinterpret_cast<std::string*>(Pop<Ptr>(vm));
			std::cout << *p_string << std::endl;
		}
	});
	handle.AddFunction({ "void", "string_set_char", {"ptr", "int", "char"}, [](VirtualMachine& vm)
		{
			std::string* p_string = reinterpret_cast<std::string*>(Pop<Ptr>(vm));
			Int index = Pop<Int>(vm);
			Char c = Pop<Char>(vm);

			p_string->at(index) = c;
		}
	});
	handle.AddFunction({ "void", "string_add_char", {"ptr", "char"}, [](VirtualMachine& vm)
		{
			std::string* p_string = reinterpret_cast<std::string*>(Pop<Ptr>(vm));
			Char c = Pop<Char>(vm);

			p_string->push_back(c);
		}
	});
	handle.Compile();
	handle.Execute<void>();

	return 0;
}