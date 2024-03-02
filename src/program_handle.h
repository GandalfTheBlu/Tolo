#pragma once
#include "virtual_machine.h"
#include "parser.h"
#include <string>
#include <vector>
#include <map>

namespace Tolo
{
	template<typename T>
	bool WriteValue(Ptr p_data, Int& inoutOffset, const T& value)
	{
		if (inoutOffset < sizeof(T))
			return false;

		inoutOffset -= sizeof(T);
		*reinterpret_cast<T*>(p_data + inoutOffset) = value;

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
		Ptr p_stack;
		Int stackSize;
		Int constStringCapacity;
		std::string mainFunctionName;
		Int codeStart;
		Int codeEnd;
		Int mainReturnValueSize;
		std::map<std::string, Int> typeNameToSize;
		std::map<std::string, NativeFunctionInfo> nativeFunctions;
		std::map<std::string, StructInfo> typeNameToStructInfo;
		std::map<std::string, std::map<std::string, NativeFunctionInfo>> typeNameToNativeOpFuncs;
		std::map<std::string, void(*)(ProgramHandle&)> standardTookitAdders;

		ProgramHandle() = delete;
		ProgramHandle(const ProgramHandle&) = delete;
		ProgramHandle& operator=(const ProgramHandle&) = delete;

		void AddNativeFunction(const FunctionHandle& function);

		void AddNativeOperator(const FunctionHandle& function);

	public:
		ProgramHandle(const std::string& _codePath, Int _stackSize, Int _constStringCapacity, const std::string& _mainFunctionName = "main");

		~ProgramHandle();

		void AddFunction(const FunctionHandle& function);

		void AddStruct(const StructHandle& _struct);

		void Compile();

		template<typename RETURN_TYPE, typename... ARGUMENTS>
		std::enable_if_t<!std::is_same<RETURN_TYPE, void>::value, RETURN_TYPE>
		Execute(const ARGUMENTS&... arguments)
		{
			Affirm(
				mainReturnValueSize == sizeof(RETURN_TYPE),
				"requested return type does not match size of 'main'-function's return type"
			);

			Int argByteOffset = codeStart;
			bool writeSuccess = (WriteValue(p_stack, argByteOffset, arguments) && ...);

			Affirm(
				writeSuccess && argByteOffset == 0,
				"argument list provided to 'main'-function does not match the size of parameter list"
			);

			RunProgram(p_stack, codeStart, codeEnd);

			return *reinterpret_cast<RETURN_TYPE*>(p_stack + codeEnd);
		}

		template<typename RETURN_TYPE, typename... ARGUMENTS>
		std::enable_if_t<std::is_same<RETURN_TYPE, void>::value>
		Execute(const ARGUMENTS&... arguments)
		{
			Affirm(
				mainReturnValueSize == 0,
				"requested return type does not match size of 'main'-function's return type"
			);

			Int argByteOffset = constStringCapacity;
			bool writeSuccess = (WriteValue(p_stack, argByteOffset, arguments) && ...);

			Affirm(
				writeSuccess && argByteOffset == codeStart,
				"argument list provided to 'main'-function does not match the size of parameter list"
			);

			RunProgram(p_stack, codeStart, codeEnd);
		}

		const std::string& GetCodePath() const;
	};
}