#pragma once
#include "common.h"
#include <cmath>

namespace Tolo
{
	/*
	...    <- sp
	...
	...
	...	   <- current fp
	old fp (fp - 4)
	old ip (fp - 8)
	retval-offset (positive) (fp - 12)
	local3		|
	local2		|
	local1		| func ip is written here before call
	param3		|
	param2		|
	param1		| retval writes here
	...		<---'
	...
	...    <- current ip

	*/


	struct VirtualMachine
	{
		Ptr stackPtr;
		Ptr instructionPtr;
		Ptr framePtr;

		Char* p_stack;
	};

	typedef void(*native_func_t)(VirtualMachine&);

	template<typename T>
	void Set(VirtualMachine& vm, Ptr pos, T val)
	{
		*(T*)(vm.p_stack + pos) = val;
	}

	template<typename T>
	T Get(VirtualMachine& vm, Ptr pos)
	{
		return *(T*)(vm.p_stack + pos);
	}

	template<typename T>
	void Push(VirtualMachine& vm, T val)
	{
		Set<T>(vm, vm.stackPtr, val);
		vm.stackPtr += sizeof(T);
	}

	template<typename T>
	T Pop(VirtualMachine& vm)
	{
		vm.stackPtr -= sizeof(T);
		return Get<T>(vm, vm.stackPtr);
	}

	inline void Op_Load_FP(VirtualMachine& vm)
	{
		Push<Ptr>(vm, vm.framePtr);
		vm.instructionPtr += sizeof(Char);
	}

	inline void Op_Load_Bytes_From(VirtualMachine& vm)
	{
		Int size = Pop<Int>(vm);
		Ptr addr = Pop<Ptr>(vm);

		/*for (Int i = 0; i < size; i++)
			Push<Char>(vm, Get<Char>(vm, addr + i));*/

		std::memcpy(vm.p_stack + vm.stackPtr, vm.p_stack + addr, size);
		vm.stackPtr += size;

		vm.instructionPtr += sizeof(Char);
	}

	template<typename T>
	void Op_Load_Const_T(VirtualMachine& vm)
	{
		vm.instructionPtr += sizeof(Char);
		
		//Push<T>(vm, Get<T>(vm, vm.instructionPtr));
		std::memcpy(vm.p_stack + vm.stackPtr, vm.p_stack + vm.instructionPtr, sizeof(T));
		vm.stackPtr += sizeof(T);
		
		vm.instructionPtr += sizeof(T);
	}

	inline void Op_Write_IP(VirtualMachine& vm)
	{
		vm.instructionPtr = Pop<Ptr>(vm);
	}

	inline void Op_Write_IP_If(VirtualMachine& vm)
	{
		if (Pop<Char>(vm) > 0)
			vm.instructionPtr = Pop<Ptr>(vm);
		else
			vm.instructionPtr += sizeof(Char);
	}

	inline void Op_Write_Bytes_To(VirtualMachine& vm)
	{
		Int size = Pop<Int>(vm);
		Ptr addr = Pop<Ptr>(vm);

		/*for (Int i = size - 1; i >= 0; i--)
			Set<Char>(vm, addr + i, Pop<Char>(vm));*/

		std::memcpy(vm.p_stack + addr, vm.p_stack + vm.stackPtr - size, size);
		vm.stackPtr -= size;

		vm.instructionPtr += sizeof(Char);
	}

	inline void Op_Call(VirtualMachine& vm)
	{
		vm.instructionPtr += sizeof(Char);
		Int paramsSize = Get<Int>(vm, vm.instructionPtr);
		vm.instructionPtr += sizeof(Int);
		Int localsSize = Get<Int>(vm, vm.instructionPtr);
		vm.instructionPtr += sizeof(Int);

		Ptr funcAddr = Pop<Ptr>(vm);
		vm.stackPtr += localsSize;
		Push<Int>(vm, paramsSize + localsSize);
		Push<Ptr>(vm, vm.instructionPtr);
		Push<Ptr>(vm, vm.framePtr);
		vm.framePtr = vm.stackPtr;

		vm.instructionPtr = funcAddr;
	}

	inline void Op_Return(VirtualMachine& vm)
	{
		vm.instructionPtr += sizeof(Char);
		Int retValSize = Get<Int>(vm, vm.instructionPtr);
		Ptr retValAddr = vm.stackPtr - retValSize;
		vm.stackPtr = vm.framePtr;
		vm.framePtr = Pop<Ptr>(vm);
		vm.instructionPtr = Pop<Ptr>(vm);
		vm.stackPtr -= Pop<Int>(vm);

		/*for (Int i = 0; i < retValSize; i++)
			Push<Char>(vm, Get<Char>(vm, retValAddr + i));*/

		std::memcpy(vm.p_stack + vm.stackPtr, vm.p_stack + retValAddr, retValSize);
		vm.stackPtr += retValSize;
	}

	inline void Op_Call_Native(VirtualMachine& vm)
	{
		Ptr funcAddr = Pop<Ptr>(vm);
		reinterpret_cast<native_func_t>(funcAddr)(vm);
		vm.instructionPtr += sizeof(Char);
	}

	template<typename T>
	void Op_T_Equal(VirtualMachine& vm)
	{
		T lhs = Pop<T>(vm);
		T rhs = Pop<T>(vm);
		Push<Char>(vm, lhs == rhs ? 1 : 0);
		vm.instructionPtr += sizeof(Char);
	}

	template<typename T>
	void Op_T_Less(VirtualMachine& vm)
	{
		T lhs = Pop<T>(vm);
		T rhs = Pop<T>(vm);
		Push<Char>(vm, lhs < rhs ? 1 : 0);
		vm.instructionPtr += sizeof(Char);
	}

	template<typename T>
	void Op_T_Greater(VirtualMachine& vm)
	{
		T lhs = Pop<T>(vm);
		T rhs = Pop<T>(vm);
		Push<Char>(vm, lhs > rhs ? 1 : 0);
		vm.instructionPtr += sizeof(Char);
	}

	template<typename T, typename U>
	void Op_TU_Add(VirtualMachine& vm)
	{
		T lhs = Pop<T>(vm);
		U rhs = Pop<U>(vm);
		Push<T>(vm, lhs + rhs);
		vm.instructionPtr += sizeof(Char);
	}

	template<typename T, typename U>
	void Op_TU_Sub(VirtualMachine& vm)
	{
		T lhs = Pop<T>(vm);
		U rhs = Pop<U>(vm);
		Push<T>(vm, lhs - rhs);
		vm.instructionPtr += sizeof(Char);
	}

	template<typename T>
	void Op_T_Mul(VirtualMachine& vm)
	{
		T lhs = Pop<T>(vm);
		T rhs = Pop<T>(vm);
		Push<T>(vm, lhs * rhs);
		vm.instructionPtr += sizeof(Char);
	}

	template<typename T>
	void Op_T_Div(VirtualMachine& vm)
	{
		T lhs = Pop<T>(vm);
		T rhs = Pop<T>(vm);
		Push<T>(vm, lhs / rhs);
		vm.instructionPtr += sizeof(Char);
	}

	template<typename T>
	void Op_T_Negate(VirtualMachine& vm)
	{
		T val = Pop<T>(vm);
		Push<T>(vm, -val);
		vm.instructionPtr += sizeof(Char);
	}

	void RunProgram(Char* p_stack, Ptr codeStart, Ptr codeEnd);
}