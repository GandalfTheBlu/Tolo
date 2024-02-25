#pragma once
#include <cstdio>
#include <string>
#include <cassert>

namespace Tolo
{
	typedef char Char;
	typedef int Int;
	typedef float Float;
	typedef Char* Ptr;

	enum class OpCode : Char
	{
		//					Next instruction	Stack before		Stack after
		Load_FP,//			-					-					Ptr
		Load_Bytes_From,//	-					Int Ptr				[bytes]
		Load_Const_Char,//	Char				-					Char	
		Load_Const_Int,//	Int					-					Int		
		Load_Const_Float,//	Float				-					Float
		Load_Const_Ptr,//	Ptr					-					Ptr

		Write_IP,//			-					Ptr					-
		Write_IP_If,//		-					Char Ptr			-
		Write_Bytes_To,//	-					Int Ptr [bytes]		-		

		Call,//				Int Int				[bytes] Ptr			[bytes] [bytes] Int Ptr Ptr	= (*1)
		Return,//			Int					(*1) [bytes]		[bytes]
		Call_Native,//		-					[bytes] Ptr			[bytes]

		Char_Equal,//		-					Char Char			Char
		Char_Less,//		-					Char Char			Char
		Char_Greater,//		-					Char Char			Char
		Char_LessOrEqual,// -					Char Char			Char
		Char_GreaterOrEqual,//-					Char Char			Char
		Char_NotEqual,//	-					Char Char			Char
		Char_Add,//			-					Char Char			Char
		Char_Sub,//			-					Char Char			Char
		Char_Mul,//			-					Char Char			Char
		Char_Div,//			-					Char Char			Char
		Char_Negate,//		-					Char				Char

		Not,//				-					Char				Char
		And,//				-					Char Char			Char
		Or,//				-					Char Char			Char

		Int_Equal,//		-					Int Int				Char
		Int_Less,//			-					Int Int				Char
		Int_Greater,//		-					Int Int				Char
		Int_LessOrEqual,//	-					Int	Int				Char
		Int_GreaterOrEqual,//-					Int Int				Char
		Int_NotEqual,//		-					Int Int				Char
		Int_Add,//			-					Int Int				Int
		Int_Sub,//			-					Int Int				Int
		Int_Mul,//			-					Int Int				Int
		Int_Div,//			-					Int Int				Int
		Int_Negate,//		-					Int					Int

		Float_Equal,//		-					Float Float			Char
		Float_Less,//		-					Float Float			Char
		Float_Greater,//	-					Float Float			Char
		Float_LessOrEqual,//-					Float Flaot			Char
		Float_GreaterOrEqual,//-				Float FLoat			Char
		Float_NotEqual,//	-					Float Float			Char
		Float_Add,//		-					Float Float			Float
		Float_Sub,//		-					Float Float			Float
		Float_Mul,//		-					Float Float			Float
		Float_Div,//		-					Float Float			Float
		Float_Negate,//		-					Float				Float

		Ptr_Add,//			-					Ptr Int				Ptr
		Ptr_Sub,//			-					Ptr Int				Ptr
		Ptr_Less,//			-					Ptr Ptr				Char
		Ptr_Greater,//		-					Ptr Ptr				Char
		Ptr_Equal,//		-					Ptr Ptr				Char
		Ptr_LessOrEqual,//  -					Ptr Ptr				Char
		Ptr_GreaterOrEqual,//-					Ptr Ptr				Char
		Ptr_NotEqual,//		-					Ptr Ptr				Char

		Bit_8_And,//		-					[8-bit] [8-bit]		[8-bit]
		Bit_8_Or,//			-					[8-bit] [8-bit]		[8-bit]
		Bit_8_Xor,//		-					[8-bit] [8-bit]		[8-bit]
		Bit_8_LeftShift,//	-					[8-bit] [8-bit]		[8-bit]
		Bit_8_RightShift,//	-					[8-bit] [8-bit]		[8-bit]

		Bit_32_And,//		-					[32-bit] [32-bit]	[32-bit]
		Bit_32_Or,//		-					[32-bit] [32-bit]	[32-bit]
		Bit_32_Xor,//		-					[32-bit] [32-bit]	[32-bit]
		Bit_32_LeftShift,//	-					[32-bit] [32-bit]	[32-bit]
		Bit_32_RightShift,//-					[32-bit] [32-bit]	[32-bit]

		INVALID
	};

	struct Error
	{
		std::string message;

		void Print() const;
	};

	inline void Error::Print() const
	{
		std::printf("[ERROR] %s\n", message.c_str());
	}

	template<typename ...ARGS>
	void Affirm(bool test, const char* format, ARGS... args)
	{
		if (test)
			return;

		Error error;

		int strSize = std::snprintf(nullptr, 0, format, args ...) + 1;
		assert(strSize > 0);
		char* buf = new char[strSize];
		std::snprintf(buf, strSize, format, args ...);
		error.message = std::string(buf, buf + strSize - 1);
		delete[] buf;

		while (error.message.back() == '\n')
			error.message.pop_back();

		throw(error);
	}
}