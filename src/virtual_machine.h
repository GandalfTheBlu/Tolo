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
		Ptr p_stackPtr;
		Ptr p_instructionPtr;
		Ptr p_framePtr;
	};

	typedef void(*native_func_t)(VirtualMachine&);

	template<typename T>
	void Set(Ptr p_pos, T val)
	{
		*reinterpret_cast<T*>(p_pos) = val;
	}

	template<typename T>
	void SetStruct(Ptr p_pos, const T& val)
	{
		std::memcpy(p_pos, &val, sizeof(T));
	}

	template<typename T>
	T Get(Ptr p_pos)
	{
		return *reinterpret_cast<T*>(p_pos);
	}

	template<typename T>
	void Push(VirtualMachine& vm, T val)
	{
		Set<T>(vm.p_stackPtr, val);
		vm.p_stackPtr += sizeof(T);
	}

	template<typename T>
	void PushStruct(VirtualMachine& vm, const T& val)
	{
		SetStruct<T>(vm.p_stackPtr, val);
		vm.p_stackPtr += sizeof(T);
	}

	template<typename T>
	T Pop(VirtualMachine& vm)
	{
		vm.p_stackPtr -= sizeof(T);
		return Get<T>(vm.p_stackPtr);
	}

	inline void Op_Load_FP(VirtualMachine& vm)
	{
		Push<Ptr>(vm, vm.p_framePtr);
		vm.p_instructionPtr += sizeof(Char);
	}

	inline void Op_Load_Bytes_From(VirtualMachine& vm)
	{
		Int size = Pop<Int>(vm);
		Ptr p_addr = Pop<Ptr>(vm);

		std::memcpy(vm.p_stackPtr, p_addr, static_cast<size_t>(size));
		vm.p_stackPtr += size;

		vm.p_instructionPtr += sizeof(Char);
	}

	template<typename T>
	void Op_Load_Const_T(VirtualMachine& vm)
	{
		vm.p_instructionPtr += sizeof(Char);
		
		std::memcpy(vm.p_stackPtr, vm.p_instructionPtr, sizeof(T));
		vm.p_stackPtr += sizeof(T);
		
		vm.p_instructionPtr += sizeof(T);
	}

	inline void Op_Write_IP(VirtualMachine& vm)
	{
		vm.p_instructionPtr = Pop<Ptr>(vm);
	}

	inline void Op_Write_IP_If(VirtualMachine& vm)
	{
		if (Pop<Char>(vm) > 0)
			vm.p_instructionPtr = Pop<Ptr>(vm);
		else
			vm.p_instructionPtr += sizeof(Char);
	}

	inline void Op_Write_Bytes_To(VirtualMachine& vm)
	{
		Int size = Pop<Int>(vm);
		Ptr p_addr = Pop<Ptr>(vm);

		std::memcpy(p_addr, vm.p_stackPtr - size, static_cast<size_t>(size));
		vm.p_stackPtr -= size;

		vm.p_instructionPtr += sizeof(Char);
	}

	inline void Op_Call(VirtualMachine& vm)
	{
		vm.p_instructionPtr += sizeof(Char);
		Int paramsSize = Get<Int>(vm.p_instructionPtr);
		vm.p_instructionPtr += sizeof(Int);
		Int localsSize = Get<Int>(vm.p_instructionPtr);
		vm.p_instructionPtr += sizeof(Int);

		Ptr p_funcAddr = Pop<Ptr>(vm);
		vm.p_stackPtr += localsSize;
		Push<Int>(vm, paramsSize + localsSize);
		Push<Ptr>(vm, vm.p_instructionPtr);
		Push<Ptr>(vm, vm.p_framePtr);
		vm.p_framePtr = vm.p_stackPtr;

		vm.p_instructionPtr = p_funcAddr;
	}

	inline void Op_Return(VirtualMachine& vm)
	{
		vm.p_instructionPtr += sizeof(Char);
		Int retValSize = Get<Int>(vm.p_instructionPtr);
		Ptr p_retValAddr = vm.p_stackPtr - retValSize;
		vm.p_stackPtr = vm.p_framePtr;
		vm.p_framePtr = Pop<Ptr>(vm);
		vm.p_instructionPtr = Pop<Ptr>(vm);
		vm.p_stackPtr -= Pop<Int>(vm);

		std::memcpy(vm.p_stackPtr, p_retValAddr, static_cast<size_t>(retValSize));
		vm.p_stackPtr += retValSize;
	}

	inline void Op_Call_Native(VirtualMachine& vm)
	{
		Ptr p_funcAddr = Pop<Ptr>(vm);
		reinterpret_cast<native_func_t>(p_funcAddr)(vm);
		vm.p_instructionPtr += sizeof(Char);
	}

	template<typename T>
	void Op_T_Equal(VirtualMachine& vm)
	{
		T lhs = Pop<T>(vm);
		T rhs = Pop<T>(vm);
		Push<Char>(vm, lhs == rhs ? 1 : 0);
		vm.p_instructionPtr += sizeof(Char);
	}

	template<typename T>
	void Op_T_Less(VirtualMachine& vm)
	{
		T lhs = Pop<T>(vm);
		T rhs = Pop<T>(vm);
		Push<Char>(vm, lhs < rhs ? 1 : 0);
		vm.p_instructionPtr += sizeof(Char);
	}

	template<typename T>
	void Op_T_Greater(VirtualMachine& vm)
	{
		T lhs = Pop<T>(vm);
		T rhs = Pop<T>(vm);
		Push<Char>(vm, lhs > rhs ? 1 : 0);
		vm.p_instructionPtr += sizeof(Char);
	}

	template<typename T>
	void Op_T_LessOrEqual(VirtualMachine& vm)
	{
		T lhs = Pop<T>(vm);
		T rhs = Pop<T>(vm);
		Push<Char>(vm, lhs <= rhs ? 1 : 0);
		vm.p_instructionPtr += sizeof(Char);
	}

	template<typename T>
	void Op_T_GreaterOrEqual(VirtualMachine& vm)
	{
		T lhs = Pop<T>(vm);
		T rhs = Pop<T>(vm);
		Push<Char>(vm, lhs >= rhs ? 1 : 0);
		vm.p_instructionPtr += sizeof(Char);
	}

	template<typename T>
	void Op_T_NotEqual(VirtualMachine& vm)
	{
		T lhs = Pop<T>(vm);
		T rhs = Pop<T>(vm);
		Push<Char>(vm, lhs != rhs ? 1 : 0);
		vm.p_instructionPtr += sizeof(Char);
	}

	inline void Op_Not(VirtualMachine& vm)
	{
		Char val = Pop<Char>(vm);
		Push<Char>(vm, val > 0 ? 0 : 1);
		vm.p_instructionPtr += sizeof(Char);
	}

	inline void Op_And(VirtualMachine& vm)
	{
		Char lhs = Pop<Char>(vm);
		Char rhs = Pop<Char>(vm);
		Push<Char>(vm, (lhs > 0 && rhs > 0) ? 1 : 0);
		vm.p_instructionPtr += sizeof(Char);
	}

	inline void Op_Or(VirtualMachine& vm)
	{
		Char lhs = Pop<Char>(vm);
		Char rhs = Pop<Char>(vm);
		Push<Char>(vm, (lhs > 0 || rhs > 0) ? 1 : 0);
		vm.p_instructionPtr += sizeof(Char);
	}

	template<typename T, typename U>
	void Op_TU_Add(VirtualMachine& vm)
	{
		T lhs = Pop<T>(vm);
		U rhs = Pop<U>(vm);
		Push<T>(vm, lhs + rhs);
		vm.p_instructionPtr += sizeof(Char);
	}

	template<typename T, typename U>
	void Op_TU_Sub(VirtualMachine& vm)
	{
		T lhs = Pop<T>(vm);
		U rhs = Pop<U>(vm);
		Push<T>(vm, lhs - rhs);
		vm.p_instructionPtr += sizeof(Char);
	}

	template<typename T>
	void Op_T_Mul(VirtualMachine& vm)
	{
		T lhs = Pop<T>(vm);
		T rhs = Pop<T>(vm);
		Push<T>(vm, lhs * rhs);
		vm.p_instructionPtr += sizeof(Char);
	}

	template<typename T>
	void Op_T_Div(VirtualMachine& vm)
	{
		T lhs = Pop<T>(vm);
		T rhs = Pop<T>(vm);
		Push<T>(vm, lhs / rhs);
		vm.p_instructionPtr += sizeof(Char);
	}

	template<typename T>
	void Op_T_Negate(VirtualMachine& vm)
	{
		T val = Pop<T>(vm);
		Push<T>(vm, -val);
		vm.p_instructionPtr += sizeof(Char);
	}

	template<typename T>
	void Op_T_Bit_And(VirtualMachine& vm)
	{
		T lhs = Pop<T>(vm);
		T rhs = Pop<T>(vm);
		Push<T>(vm, lhs & rhs);
		vm.p_instructionPtr += sizeof(Char);
	}

	template<typename T>
	void Op_T_Bit_Or(VirtualMachine& vm)
	{
		T lhs = Pop<T>(vm);
		T rhs = Pop<T>(vm);
		Push<T>(vm, lhs | rhs);
		vm.p_instructionPtr += sizeof(Char);
	}

	template<typename T>
	void Op_T_Bit_Xor(VirtualMachine& vm)
	{
		T lhs = Pop<T>(vm);
		T rhs = Pop<T>(vm);
		Push<T>(vm, lhs ^ rhs);
		vm.p_instructionPtr += sizeof(Char);
	}

	template<typename T>
	void Op_T_Bit_LeftShift(VirtualMachine& vm)
	{
		T lhs = Pop<T>(vm);
		Int rhs = Pop<Int>(vm);
		Push<T>(vm, lhs << rhs);
		vm.p_instructionPtr += sizeof(Char);
	}

	template<typename T>
	void Op_T_Bit_RightShift(VirtualMachine& vm)
	{
		T lhs = Pop<T>(vm);
		Int rhs = Pop<Int>(vm);
		Push<T>(vm, lhs >> rhs);
		vm.p_instructionPtr += sizeof(Char);
	}

	template<typename T>
	void Op_T_Bit_Invert(VirtualMachine& vm)
	{
		T val = Pop<T>(vm);
		Push<T>(vm, ~val);
		vm.p_instructionPtr += sizeof(Char);
	}

	void RunProgram(Ptr p_stack, Int codeStart, Int codeEnd);
}