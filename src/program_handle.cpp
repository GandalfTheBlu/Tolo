#include "program_handle.h"
#include "compiler.h"

namespace Tolo
{
	FunctionHandle::FunctionHandle() :
		p_function(nullptr)
	{}

	FunctionHandle::FunctionHandle(const std::string& _returnTypeName, const std::string& _functionName, const std::vector<std::string>& _parameterTypeNames, native_func_t _p_function) :
		p_function(_p_function),
		returnTypeName(_returnTypeName),
		functionName(_functionName),
		parameterTypeNames(_parameterTypeNames)
	{}

	FunctionHandle::FunctionHandle(const FunctionHandle& rhs) :
		p_function(rhs.p_function),
		returnTypeName(rhs.returnTypeName),
		functionName(rhs.functionName),
		parameterTypeNames(rhs.parameterTypeNames)
	{}

	FunctionHandle& FunctionHandle::operator=(const FunctionHandle& rhs)
	{
		p_function = rhs.p_function;
		returnTypeName = rhs.returnTypeName;
		functionName = rhs.functionName;
		parameterTypeNames = rhs.parameterTypeNames;
		return *this;
	}


	ProgramHandle::ProgramHandle(const std::string& _codePath, Ptr stackSize) :
		codePath(_codePath),
		codeStart(0),
		codeEnd(0),
		mainReturnValueSize(0)
	{
		p_stack = (Char*)std::malloc(stackSize);
	}

	ProgramHandle::~ProgramHandle()
	{
		std::free(p_stack);
	}

	void ProgramHandle::AddFunction(const FunctionHandle& function)
	{
		Affirm(
			nativeFunctions.count(function.functionName) == 0, 
			"native function '%s' is already defined", 
			function.functionName.c_str()
		);

		NativeFunctionInfo& info = nativeFunctions[function.functionName];
		info.functionPtr = reinterpret_cast<Ptr>(function.p_function);
		info.returnTypeName = function.returnTypeName;
		info.parameterTypeNames = function.parameterTypeNames;
	}

	void ProgramHandle::Compile()
	{
		CompileProgram(codePath, p_stack, nativeFunctions, codeStart, codeEnd, mainReturnValueSize);
	}
}