#include "program_handle.h"
#include "tokenizer.h"
#include "lexer.h"
#include "file_io.h"

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


	ProgramHandle::ProgramHandle(const std::string& _codePath, Ptr stackSize, const std::string& _mainFunctionName) :
		codePath(_codePath),
		mainFunctionName(_mainFunctionName),
		codeStart(0),
		codeEnd(0),
		mainReturnValueSize(0)
	{
		p_stack = (Char*)std::malloc(stackSize);

		typeNameToSize["char"] = sizeof(Char);
		typeNameToSize["int"] = sizeof(Int);
		typeNameToSize["float"] = sizeof(Float);
		typeNameToSize["ptr"] = sizeof(Ptr);
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
		info.functionPtr = reinterpret_cast<Ptr>(function.p_function);
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
		std::map<std::string, NativeFunctionInfo>& opFunctions = typeNameToPrimitiveOpFuncs[operandTypeName];

		Affirm(
			opFunctions.count(opName) == 0,
			"operator '%s' for type '%s' is already defined",
			opName.c_str(), operandTypeName.c_str()
		);

		NativeFunctionInfo& info = opFunctions[opName];
		info.functionPtr = reinterpret_cast<Ptr>(function.p_function);
		info.returnTypeName = function.returnTypeName;
		info.parameterTypeNames = function.parameterTypeNames;
	}

	void ProgramHandle::AddFunction(const FunctionHandle& function)
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
			function.functionName.size() > 8 && 
			function.functionName.substr(0, 8) == "operator" && 
			operators.count(function.functionName.substr(8)) != 0)
		{
			FunctionHandle opHandle = function;
			opHandle.functionName = function.functionName.substr(8);
			AddNativeOperator(opHandle);
		}
		else
		{
			for (char c : function.functionName)
			{
				if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')
					continue;

				Affirm(false, "invalid character '%c' in function name '%s'", c, function.functionName.c_str());
			}

			AddNativeFunction(function);
		}
	}

	void ProgramHandle::AddStruct(const StructHandle& _struct)
	{
		Affirm(
			typeNameToStructInfo.count(_struct.typeName) == 0,
			"struct '%s' is already defined",
			_struct.typeName.c_str()
		);

		StructInfo& info = typeNameToStructInfo[_struct.typeName];
		
		Int propertyOffset = 0;
		for (auto& prop : _struct.properties)
		{
			Affirm(
				prop.first != _struct.typeName,
				"struct '%s' cannot contain itself",
				_struct.typeName.c_str()
			);

			Affirm(
				typeNameToSize.count(prop.first) != 0,
				"type name '%s' in struct '%s' is not defined",
				prop.first.c_str(), _struct.typeName.c_str()
			);

			Affirm(
				info.propNameToVarInfo.count(prop.second) == 0,
				"property '%s' is already defined in struct '%s'",
				prop.second.c_str(), _struct.typeName.c_str()
			);

			info.propNameToVarInfo[prop.second] = VariableInfo(prop.first, propertyOffset);
			info.propNames.push_back(prop.second);
			propertyOffset += typeNameToSize[prop.first];
		}

		typeNameToSize[_struct.typeName] = propertyOffset;
	}

	void ProgramHandle::Compile(std::string& outCode)
	{
		ReadTextFile(codePath, outCode);

		std::vector<Token> tokens;
		Tokenize(outCode, tokens);

		Lexer lexer;
		for (auto& e : nativeFunctions)
			lexer.nativeFunctions.insert(e.first);

		std::vector<LexNode*> lexNodes;
		lexer.Lex(tokens, lexNodes);

		Parser parser;
		parser.nativeFunctions = nativeFunctions;
		parser.typeNameToStructInfo = typeNameToStructInfo;
		parser.typeNameToNativeOpFuncs = typeNameToPrimitiveOpFuncs;

		// transfer struct type sizes
		for (auto& e : typeNameToStructInfo)
		{
			parser.typeNameToSize[e.first] = typeNameToSize[e.first];
		}

		std::vector<Expression*> expressions;
		parser.Parse(lexNodes, expressions);

		CodeBuilder cb(p_stack);

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

		cb.codeLength += mainParamsSize;// allocate space for main arguments in bottom of stack

		codeStart = cb.codeLength;
		mainReturnValueSize = parser.typeNameToSize[mainInfo.returnTypeName];

		ECallFunction mainCall(mainParamsSize, mainInfo.localsSize, mainInfo.returnTypeName);
		// tell the main function to load arguments from the beginning of the stack, where the user will write them
		mainCall.argumentLoads = { new ELoadConstBytes(mainParamsSize, 0) };
		mainCall.functionIpLoad = new ELoadConstPtrToLabel(mainFunctionName);
		mainCall.Evaluate(cb);
		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel("__program_end__");
		cb.Op(OpCode::Write_IP);

		for (auto e : expressions)
			e->Evaluate(cb);

		cb.DefineLabel("__program_end__");
		cb.RemoveLabel("__program_end__");

		for (auto e : lexNodes)
			delete e;

		for (auto e : expressions)
			delete e;

		codeEnd = cb.codeLength;
	}

	void ProgramHandle::Compile()
	{
		std::string outCode;
		Compile(outCode);
	}

	const std::string& ProgramHandle::GetCodePath() const
	{
		return codePath;
	}
}