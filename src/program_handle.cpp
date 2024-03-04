#include "program_handle.h"
#include "preprocessor.h"
#include "tokenizer.h"
#include "lexer.h"
#include "file_io.h"
#include "standard_toolkit.h"
#include <set>

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


	StructHandle::StructHandle()
	{}

	StructHandle::StructHandle(const std::string& _typeName, const std::vector<std::pair<std::string, std::string>>& _properties) :
		typeName(_typeName),
		properties(_properties)
	{}

	StructHandle::StructHandle(const StructHandle& rhs) :
		typeName(rhs.typeName),
		properties(rhs.properties)
	{}

	StructHandle& StructHandle::operator=(const StructHandle& rhs)
	{
		typeName = rhs.typeName;
		properties = rhs.properties;
		return *this;
	}


	ProgramHandle::ProgramHandle(const std::string& _codePath, Int _stackSize, Int _constStringCapacity, const std::string& _mainFunctionName) :
		codePath(_codePath),
		stackSize(_stackSize),
		constStringCapacity(_constStringCapacity),
		mainFunctionName(_mainFunctionName),
		codeStart(0),
		codeEnd(0),
		mainReturnValueSize(0)
	{
		p_stack = static_cast<Char*>(std::malloc(stackSize));

		typeNameToSize["char"] = sizeof(Char);
		typeNameToSize["int"] = sizeof(Int);
		typeNameToSize["float"] = sizeof(Float);
		typeNameToSize["ptr"] = sizeof(Ptr);

		standardTookitAdders =
		{
			{"memory", AddMemoryToolkit},
			{"io", AddIOToolkit}
		};
	}

	ProgramHandle::~ProgramHandle()
	{
		std::free(p_stack);
	}

	void ProgramHandle::AddNativeFunction(const FunctionHandle& function)
	{
		Affirm(
			nativeFunctions.count(function.functionName) == 0, 
			"native function '%s' is already defined", 
			function.functionName.c_str()
		);

		NativeFunctionInfo& info = nativeFunctions[function.functionName];
		info.p_functionPtr = reinterpret_cast<Ptr>(function.p_function);
		info.returnTypeName = function.returnTypeName;
		info.parameterTypeNames = function.parameterTypeNames;
	}

	void ProgramHandle::AddNativeOperator(const FunctionHandle& function)
	{
		Affirm(
			typeNameToSize.count(function.returnTypeName) != 0,
			"undefined type '%s' in operator function '%s'",
			function.returnTypeName.c_str(), function.functionName.c_str()
		);

		Affirm(
			function.parameterTypeNames.size() > 0, 
			"missing parameters in operator function '%s'", 
			function.functionName.c_str()
		);

		std::string opName = function.functionName;

		if (opName == "-" && function.parameterTypeNames.size() == 1)
		{
			opName = "negate";
		}
		
		const std::string operandTypeName = function.parameterTypeNames.front();
		std::map<std::string, NativeFunctionInfo>& opFunctions = typeNameToNativeOpFuncs[operandTypeName];

		Affirm(
			opFunctions.count(opName) == 0,
			"operator '%s' for type '%s' is already defined",
			opName.c_str(), operandTypeName.c_str()
		);

		NativeFunctionInfo& info = opFunctions[opName];
		info.p_functionPtr = reinterpret_cast<Ptr>(function.p_function);
		info.returnTypeName = function.returnTypeName;
		info.parameterTypeNames = function.parameterTypeNames;
	}

	void ProgramHandle::AddFunction(
		const std::string& returnTypeName, 
		const std::string& functionName, 
		const std::vector<std::string>& parameterTypeNames,
		native_func_t functionPtr
	)
	{
		static std::set<std::string> operators
		{
			"+",
			"-",
			"*",
			"/",
			"<",
			">",
			"<=",
			">=",
			"==",
			"!=",
			"<<",
			">>",
			"&",
			"|",
			"^"
		};

		if (
			functionName.size() > 8 && 
			functionName.substr(0, 8) == "operator" && 
			operators.count(functionName.substr(8)) != 0)
		{
			FunctionHandle opHandle = FunctionHandle(returnTypeName, functionName, parameterTypeNames, functionPtr);
			opHandle.functionName = functionName.substr(8);
			AddNativeOperator(opHandle);
		}
		else
		{
			for (char c : functionName)
			{
				if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')
					continue;

				Affirm(false, "invalid character '%c' in function name '%s'", c, functionName.c_str());
			}

			AddNativeFunction(FunctionHandle(returnTypeName, functionName, parameterTypeNames, functionPtr));
		}
	}

	void ProgramHandle::AddStruct(
		const std::string& structName,
		const std::vector<std::pair<std::string, std::string>>& members
	)
	{
		Affirm(
			typeNameToStructInfo.count(structName) == 0,
			"struct '%s' is already defined",
			structName.c_str()
		);

		StructInfo& info = typeNameToStructInfo[structName];
		
		const std::string structPtrName = structName + "::ptr";
		typeNameToSize[structPtrName] = static_cast<Int>(sizeof(Ptr));

		Int propertyOffset = 0;
		for (auto& prop : members)
		{
			Affirm(
				prop.first != structName,
				"struct '%s' cannot contain itself",
				structName.c_str()
			);

			Affirm(
				typeNameToSize.count(prop.first) != 0,
				"type name '%s' in struct '%s' is not defined",
				prop.first.c_str(), structName.c_str()
			);

			Affirm(
				info.memberNameToVarInfo.count(prop.second) == 0,
				"property '%s' is already defined in struct '%s'",
				prop.second.c_str(), structName.c_str()
			);

			info.memberNameToVarInfo[prop.second] = VariableInfo(prop.first, propertyOffset);
			info.memberNames.push_back(prop.second);
			propertyOffset += typeNameToSize[prop.first];
		}

		typeNameToSize[structName] = propertyOffset;
	}

	void ProgramHandle::Compile()
	{
		std::string rawCode;
		ReadTextFile(codePath, rawCode);

		std::map<std::string, bool> standardIncludeFlags;
		for (auto pair : standardTookitAdders)
			standardIncludeFlags[pair.first] = false;

		std::string code;
		Preprocess(rawCode, code, standardIncludeFlags);

		for (auto pair : standardIncludeFlags)
		{
			if (pair.second)
				standardTookitAdders.at(pair.first)(*this);
		}

		std::vector<Token> tokens;
		Tokenize(code, tokens);

		Lexer lexer;
		std::vector<std::shared_ptr<LexNode>> lexNodes;
		lexer.Lex(tokens, lexNodes);

		Parser parser;
		parser.nativeFunctions = nativeFunctions;
		parser.typeNameToStructInfo = typeNameToStructInfo;
		parser.typeNameToNativeOpFuncs = typeNameToNativeOpFuncs;

		// transfer struct type sizes and struct pointers
		for (auto& e : typeNameToStructInfo)
		{
			parser.typeNameToSize[e.first] = typeNameToSize[e.first];
			std::string structPtrName = e.first + "::ptr";
			parser.typeNameToSize[structPtrName] = static_cast<Int>(sizeof(Ptr));
			parser.ptrTypeNameToStructTypeName[structPtrName] = e.first;
		}

		std::vector<std::shared_ptr<Expression>> expressions;
		parser.Parse(lexNodes, expressions);

		Affirm(
			parser.userFunctions.count(mainFunctionName) != 0,
			"failed to get main function, no function called '%s' found",
			mainFunctionName.c_str()
		);

		FunctionInfo& mainInfo = parser.userFunctions[mainFunctionName];
		Int mainParamsSize = 0;

		for (size_t i = 0; i < mainInfo.parameterNames.size(); i++)
		{
			const std::string& paramTypeName = mainInfo.varNameToVarInfo[mainInfo.parameterNames[i]].typeName;
			mainParamsSize += parser.typeNameToSize[paramTypeName];
		}

		CodeBuilder cb(p_stack, stackSize, constStringCapacity);

		// allocate space for main arguments in bottom of stack
		cb.codeLength += mainParamsSize;

		codeStart = cb.codeLength;
		mainReturnValueSize = parser.typeNameToSize[mainInfo.returnTypeName];

		ECallFunction mainCall(mainParamsSize, mainInfo.localsSize);
		// tell the main function to load arguments from the beginning of the stack, where the user will write them
		mainCall.argumentLoads = { std::make_shared<ELoadConstBytes>(mainParamsSize, p_stack) };
		mainCall.functionIpLoad = std::make_shared<ELoadConstPtrToLabel>(mainFunctionName);
		mainCall.Evaluate(cb);
		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel("0program_end");
		cb.Op(OpCode::Write_IP);

		for (auto e : expressions)
			e->Evaluate(cb);

		cb.DefineLabel("0program_end");
		cb.RemoveLabel("0program_end");

		codeEnd = cb.codeLength;
	}

	const std::string& ProgramHandle::GetCodePath() const
	{
		return codePath;
	}
}