#include "virtual_machine.h"

//#define DEBUG_VM

namespace Tolo
{
	void RunProgram(Ptr p_stack, Int codeStart, Int codeEnd)
	{
		static void(*ops[])(VirtualMachine&)
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
			Op_T_LessOrEqual<Char>,
			Op_T_GreaterOrEqual<Char>,
			Op_T_NotEqual<Char>,
			Op_TU_Add<Char, Char>,
			Op_TU_Sub<Char, Char>,
			Op_T_Mul<Char>,
			Op_T_Div<Char>,
			Op_T_Negate<Char>,

			Op_Not,
			Op_And,
			Op_Or,

			Op_T_Equal<Int>,
			Op_T_Less<Int>,
			Op_T_Greater<Int>,
			Op_T_LessOrEqual<Int>,
			Op_T_GreaterOrEqual<Int>,
			Op_T_NotEqual<Int>,
			Op_TU_Add<Int, Int>,
			Op_TU_Sub<Int, Int>,
			Op_T_Mul<Int>,
			Op_T_Div<Int>,
			Op_T_Negate<Int>,

			Op_T_Equal<Float>,
			Op_T_Less<Float>,
			Op_T_Greater<Float>,
			Op_T_LessOrEqual<Float>,
			Op_T_GreaterOrEqual<Float>,
			Op_T_NotEqual<Float>,
			Op_TU_Add<Float, Float>,
			Op_TU_Sub<Float, Float>,
			Op_T_Mul<Float>,
			Op_T_Div<Float>,
			Op_T_Negate<Float>,

			Op_TU_Add<Ptr, Int>,
			Op_TU_Sub<Ptr, Int>,
			Op_T_Less<Ptr>,
			Op_T_Greater<Ptr>,
			Op_T_Equal<Ptr>,
			Op_T_LessOrEqual<Ptr>,
			Op_T_GreaterOrEqual<Ptr>,
			Op_T_NotEqual<Ptr>,
				
			Op_T_Bit_And<Char>,
			Op_T_Bit_Or<Char>,
			Op_T_Bit_Xor<Char>,
			Op_T_Bit_LeftShift<Char>,
			Op_T_Bit_RightShift<Char>,

			Op_T_Bit_And<Int>,
			Op_T_Bit_Or<Int>,
			Op_T_Bit_Xor<Int>,
			Op_T_Bit_LeftShift<Int>,
			Op_T_Bit_RightShift<Int>
		};

#ifdef DEBUG_VM
		static const char* debugOpNames[]
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
			"Char_LessOrEqual",
			"Char_GreaterOrEqual",
			"Char_NotEqual",
			"Char_Add",
			"Char_Sub",
			"Char_Mul",
			"Char_Div",
			"Char_Negate",

			"Not",
			"And",
			"Or",

			"Int_Equal",
			"Int_Less",
			"Int_Greater",
			"Int_LessOrEqual",
			"Int_GreaterOrEqual",
			"Int_NotEqual",
			"Int_Add",
			"Int_Sub",
			"Int_Mul",
			"Int_Div",
			"Int_Negate",

			"Float_Equal",
			"Float_Less",
			"Float_Greater",
			"Float_LessOrEqual",
			"Float_GreaterOrEqual",
			"Float_NotEqual",
			"Float_Add",
			"Float_Sub",
			"Float_Mul",
			"Float_Div",
			"Float_Negate",

			"Ptr_Add",
			"Ptr_Sub",
			"Ptr_Less",
			"Ptr_Greater",
			"Ptr_Equal",
			"Ptr_LessOrEqual",
			"Ptr_GreaterOrEqual",
			"Ptr_NotEqual",

			"Bit_8_And",
			"Bit_8_Or",
			"Bit_8_Xor",
			"Bit_8_LeftShift",
			"Bit_8_RightShift",

			"Bit_32_And",
			"Bit_32_Or",
			"Bit_32_Xor",
			"Bit_32_LeftShift",
			"Bit_32_RightShift"
		};
#endif

		VirtualMachine vm{
			p_stack + codeEnd,
			p_stack + codeStart,
			p_stack + 0
		};

		const Ptr p_codeEnd = p_stack + codeEnd;

		while (vm.p_instructionPtr < p_codeEnd)
		{
			Char opCode = *vm.p_instructionPtr;
#ifdef DEBUG_VM
			std::printf("%s\n", debugOpNames[opCode]);
#endif
			ops[opCode](vm);
		}
	}
}