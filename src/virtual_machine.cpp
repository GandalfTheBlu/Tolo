#include "virtual_machine.h"

//#define DEBUG_VM

namespace Tolo
{
	void RunProgram(Char* p_stack, Ptr codeStart, Ptr codeEnd)
	{
		VirtualMachine vm{ codeEnd, codeStart, 0, p_stack };
		void(*ops[])(VirtualMachine&)
		{
			Op_Load_FP,
			Op_Load_Bytes_From,
			Op_Load_Const_T<Char>,
			Op_Load_Const_T<Int>,
			Op_Load_Const_T<Float>,
			Op_Load_Const_T<Ptr>,

			Op_Write_IP,
			Op_Write_IP_If,
			Op_Write_Bytes_To,

			Op_Call,
			Op_Return,
			Op_Call_Native,

			Op_T_Equal<Char>,
			Op_T_Less<Char>,
			Op_T_Greater<Char>,
			Op_TU_Add<Char, Char>,
			Op_TU_Sub<Char, Char>,
			Op_T_Mul<Char>,
			Op_T_Div<Char>,
			Op_T_Negate<Char>,

			Op_T_Equal<Int>,
			Op_T_Less<Int>,
			Op_T_Greater<Int>,
			Op_TU_Add<Int, Int>,
			Op_TU_Sub<Int, Int>,
			Op_T_Mul<Int>,
			Op_T_Div<Int>,
			Op_T_Negate<Int>,

			Op_T_Equal<Float>,
			Op_T_Less<Float>,
			Op_T_Greater<Float>,
			Op_TU_Add<Float, Float>,
			Op_TU_Sub<Float, Float>,
			Op_T_Mul<Float>,
			Op_T_Div<Float>,
			Op_T_Negate<Float>,

			Op_T_Equal<Ptr>,
			Op_TU_Add<Ptr, Ptr>,
			Op_TU_Sub<Ptr, Ptr>
		};

#ifdef DEBUG_VM
		const char* debugOpNames[]
		{
			"Load_FP",
			"Load_Bytes_From",
			"Load_Const_Char",
			"Load_Const_Int",
			"Load_Const_Float",
			"Load_Const_Ptr",

			"Write_IP",
			"Write_IP_If",
			"Write_Bytes_To",

			"Call",
			"Return",
			"Call_Native",

			"Char_Equal",
			"Char_Less",
			"Char_Greater",
			"Char_Add",
			"Char_Sub",
			"Char_Mul",
			"Char_Div",
			"Char_Negate",

			"Int_Equal",
			"Int_Less",
			"Int_Greater",
			"Int_Add",
			"Int_Sub",
			"Int_Mul",
			"Int_Div",
			"Int_Negate",

			"Float_Equal",
			"Float_Less",
			"Float_Greater",
			"Float_Add",
			"Float_Sub",
			"Float_Mul",
			"Float_Div",
			"Float_Negate",

			"Ptr_Equal",
			"Ptr_Add",
			"Ptr_Sub"
		};
#endif

		while (vm.instructionPtr < codeEnd)
		{
			Char opCode = p_stack[vm.instructionPtr];
#ifdef DEBUG_VM
			std::printf("%s\n", debugOpNames[opCode]);
#endif
			ops[opCode](vm);
		}
	}
}