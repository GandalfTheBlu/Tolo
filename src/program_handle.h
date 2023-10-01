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

	struct StructHandle
	{
		std::string typeName;
		std::vector<std::pair<std::string, std::string>> properties;

		StructHandle();
		StructHandle(const std::string& _typeName, const std::vector<std::pair<std::string, std::string>>& _properties);
		StructHandle(const StructHandle& rhs);
		StructHandle& operator=(const StructHandle& rhs);
	};

	class ProgramHandle
	{
	private:
		std::string codePath;
		Char* p_stack;
		std::string mainFunctionName;
		Ptr codeStart;
		Ptr codeEnd;
		Int mainReturnValueSize;
		std::map<std::string, Int> typeNameToSize;
		std::map<std::string, NativeFunctionInfo> nativeFunctions;
		std::map<std::string, StructInfo> typeNameToStructInfo;
		std::map<std::string, std::map<std::string, NativeFunctionInfo>> typeNameToPrimitiveOpFuncs;

		ProgramHandle() = delete;
		ProgramHandle(const ProgramHandle&) = delete;
		ProgramHandle& operator=(const ProgramHandle&) = delete;

		void ReadTextFile(const std::string& filePath, std::string& outText);

		void AddNativeFunction(const FunctionHandle& function);

		void AddNativeOperator(const FunctionHandle& function);

	public:
		ProgramHandle(const std::string& _codePath, Ptr stackSize, const std::string& _mainFunctionName = "main");

		~ProgramHandle();

		void AddFunction(const FunctionHandle& function);

		void AddStruct(const StructHandle& _struct);

		void Compile();

		template<typename RETURN_TYPE, typename... ARGUMENTS>
		RETURN_TYPE Execute(const ARGUMENTS&... arguments)
		{
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