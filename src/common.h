#pragma once
#include <cstdio>
#include <cstdlib>

namespace Tolo
{
	typedef char Char;
	typedef int Int;
	typedef float Float;
	typedef unsigned long Ptr;

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
		Char_Add,//			-					Char Char			Char
		Char_Sub,//			-					Char Char			Char
		Char_Mul,//			-					Char Char			Char
		Char_Div,//			-					Char Char			Char

		Int_Equal,//		-					Int Int				Char
		Int_Less,//			-					Int Int				Char
		Int_Greater,//		-					Int Int				Char
		Int_Add,//			-					Int Int				Int
		Int_Sub,//			-					Int Int				Int
		Int_Mul,//			-					Int Int				Int
		Int_Div,//			-					Int Int				Int

		Float_Equal,//		-					Float Float			Char
		Float_Less,//		-					Float Float			Char
		Float_Greater,//	-					Float Float			Char
		Float_Add,//		-					Float Float			Float
		Float_Sub,//		-					Float Float			Float
		Float_Mul,//		-					Float Float			Float
		Float_Div,//		-					Float Float			Float

		Ptr_Equal,//		-					Ptr Ptr				Char
		Ptr_Add,//			-					Ptr Int				Ptr
		Ptr_Sub,//			-					Ptr Int				Ptr

		INVALID
	};

	template<typename ...ARGS>
	void Affirm(bool test, const char* msg, ARGS... args)
	{
		if (test)
			return;

		std::printf("[ERROR] ");
		std::printf(msg, args...);
		std::printf("\n");
		std::exit(1);
	}
}