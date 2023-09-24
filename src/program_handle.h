#pragma once
#include "virtual_machine.h"
#include "parser.h"
#include <string>
#include <vector>
#include <map>

namespace Tolo
{
	template<typename T>
	bool WriteValue(Char* p_data, Ptr& inoutIndex, const T& value)
	{
		if (inoutIndex < sizeof(T))
			return false;

		inoutIndex -= sizeof(T);
		*(T*)(p_data + inoutIndex) = value;

		return true;
	}

	struct FunctionHandle
	{
		native_func_t p_function;
		std::string returnTypeName;
		std::string functionName;
		std::vector<std::string> parameterTypeNames;

		FunctionHandle();
		FunctionHandle(const std::string& _returnTypeName, const std::string& _functionName, const std::vector<std::string>& _parameterTypeNames, native_func_t _p_function);
		FunctionHandle(const FunctionHandle& rhs);
		FunctionHandle& operator=(const FunctionHandle& rhs);
	};

	class ProgramHandle
	{
	private:
		std::string codePath;
		Char* p_stack;
		Ptr codeStart;
		Ptr codeEnd;
		Int mainReturnValueSize;
		std::map<std::string, NativeFunctionInfo> nativeFunctions;

		ProgramHandle() = delete;
		ProgramHandle(const ProgramHandle&) = delete;
		ProgramHandle& operator=(const ProgramHandle&) = delete;

	public:
		ProgramHandle(const std::string& _codePath, Ptr stackSize);

		~ProgramHandle();

		void AddFunction(const FunctionHandle& function);

		void Compile();

		template<typename RETURN_TYPE, typename... ARGUMENTS>
		std::enable_if_t<std::is_same<RETURN_TYPE, void>::value>
		Execute(const ARGUMENTS&... arguments)
		{
			// this version is for when RETURN_TYPE is void, so nothing should be returned

			Affirm(
				mainReturnValueSize == 0,
				"requested return type does not match size of 'main'-function's return type"
			);

			Ptr argByteOffset = codeStart;
			bool writeSuccess = (WriteValue(p_stack, argByteOffset, arguments) && ...);

			Affirm(
				writeSuccess && argByteOffset == 0,
				"argument list provided to 'main'-function does not match the size of parameter list"
			);

			RunProgram(p_stack, codeStart, codeEnd);
		}

		template<typename RETURN_TYPE, typename... ARGUMENTS>
		std::enable_if_t<!std::is_same<RETURN_TYPE, void>::value, RETURN_TYPE>
		Execute(const ARGUMENTS&... arguments)
		{
			// this version is for when RETURN_TYPE is not void, so the return value of the main function should be returned

			Affirm(
				mainReturnValueSize == sizeof(RETURN_TYPE),
				"requested return type does not match size of 'main'-function's return type"
			);

			Ptr argByteOffset = codeStart;
			bool writeSuccess = (WriteValue(p_stack, argByteOffset, arguments) && ...);

			Affirm(
				writeSuccess && argByteOffset == 0,
				"argument list provided to 'main'-function does not match the size of parameter list"
			);

			RunProgram(p_stack, codeStart, codeEnd);

			return *(RETURN_TYPE*)(p_stack + codeEnd);
		}
	};
}