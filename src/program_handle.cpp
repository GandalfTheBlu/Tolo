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
		returnTypeName(_returnTypeName),
		functionName(_functionName),
		parameterTypeNames(_parameterTypeNames),
		p_function(_p_function)
	{}

	FunctionHandle::FunctionHandle(const FunctionHandle& rhs) :
		returnTypeName(rhs.returnTypeName),
		functionName(rhs.functionName),
		parameterTypeNames(rhs.parameterTypeNames),
		p_function(rhs.p_function)
	{}

	FunctionHandle& FunctionHandle::operator=(const FunctionHandle& rhs)
	{
		returnTypeName = rhs.returnTypeName;
		functionName = rhs.functionName;
		parameterTypeNames = rhs.parameterTypeNames;
		p_function = rhs.p_function;
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


	ProgramHandle::ProgramHandle(
		const std::string& _codePath,
		Int _stackSize,
		Int _constStringCapacity,
		const std::string& mainFunctionReturnTypeName,
		const std::string mainFunctionName,
		const std::vector<std::string>& mainFunctionParameterTypeNames
	) :
		codePath(_codePath),
		stackSize(_stackSize),
		constStringCapacity(_constStringCapacity),
		codeStart(0),
		codeEnd(0),
		mainReturnValueSize(0),
		mainParameterCount(mainFunctionParameterTypeNames.size())
	{
		mainFunctionHash = GetFunctionHash(
			mainFunctionReturnTypeName, 
			mainFunctionName, 
			mainFunctionParameterTypeNames
		);

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
		std::string hash = GetFunctionHash(function.returnTypeName, function.functionName, function.parameterTypeNames);

		Affirm(
			hashToNativeFunctions.count(hash) == 0, 
			"native function '%s' is already defined", 
			hash.c_str()
		);

		NativeFunctionInfo& info = hashToNativeFunctions[hash];
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
		
		const std::string& operandTypeName = function.parameterTypeNames.front();
		std::string funcHash = GetFunctionHash(function.returnTypeName, "operator::" + opName, { operandTypeName });

		Affirm(
			hashToNativeFunctions.count(funcHash) == 0,
			"function '%s' is already defined",
			funcHash.c_str()
		);

		NativeFunctionInfo& info = hashToNativeFunctions[funcHash];
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
		static const std::set<std::string> operators
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
			AddNativeOperator(FunctionHandle(returnTypeName, functionName.substr(8), parameterTypeNames, functionPtr));
		}
		else
		{
			size_t i = 0;
			for (char c : functionName)
			{
				if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (i > 0 && c >= '0' && c <= '9') || c == '_')
					continue;

				Affirm(false, "invalid character '%c' in function name '%s'", c, functionName.c_str());

				i++;
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

	void ProgramHandle::AddStructInherit(
		const std::string& structName,
		const std::string& parentStructName,
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

		Affirm(
			parentStructName != structName &&
			typeNameToStructInfo.count(parentStructName) != 0,
			"struct type name '%s' is not defined",
			parentStructName.c_str()
		);

		structNameToParentStructName[structName] = parentStructName;

		info = typeNameToStructInfo.at(parentStructName);
		Int propertyOffset = typeNameToSize.at(parentStructName);

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

	void ProgramHandle::AddEnum(
		const std::vector<std::string>& enumNames
	)
	{
		Int enumValue = 0;
		for (const std::string& enumName : enumNames)
		{
			Affirm(
				nameToEnumValue.count(enumName) == 0,
				"enum '%s' is already defined",
				enumName.c_str()
			);

			nameToEnumValue[enumName] = enumValue++;
		}
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
		parser.hashToNativeFunctions = hashToNativeFunctions;
		parser.typeNameToStructInfo = typeNameToStructInfo;
		parser.structNameToParentStructName = structNameToParentStructName;
		parser.nameToEnumValue = nameToEnumValue;

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
			parser.hashToUserFunctions.count(mainFunctionHash) != 0,
			"failed to get main function, no function with signature '%s' found",
			mainFunctionHash.c_str()
		);

		FunctionInfo& mainInfo = parser.hashToUserFunctions.at(mainFunctionHash);
		Int mainParamsSize = 0;

		for (size_t i = 0; i < mainInfo.parameterNames.size(); i++)
		{
			const std::string& paramTypeName = mainInfo.varNameToVarInfo[mainInfo.parameterNames[i]].typeName;
			mainParamsSize += parser.typeNameToSize[paramTypeName];
		}

		CodeBuilder cb(p_stack, stackSize, constStringCapacity);

		// allocate space for main arguments
		cb.codeLength += mainParamsSize;

		codeStart = cb.codeLength;
		mainReturnValueSize = parser.typeNameToSize[mainInfo.returnTypeName];

		ECallFunction mainCall(mainParamsSize, mainInfo.localsSize);
		// tell the main function to load arguments
		mainCall.argumentLoads = { std::make_shared<ELoadConstBytes>(mainParamsSize, p_stack + constStringCapacity) };
		mainCall.functionIpLoad = std::make_shared<ELoadConstPtrToLabel>(mainFunctionHash);
		mainCall.Evaluate(cb);
		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel("0program_end");
		cb.Op(OpCode::Write_IP);

		for (auto e : expressions)
			e->Evaluate(cb);

		cb.DefineLabel("0program_end");
		cb.RemoveLabel("0program_end");

		Affirm(
			!cb.HasUnresolvedLabels(),
			"unresolved labels after building code"
		);

		codeEnd = cb.codeLength;
	}

	const std::string& ProgramHandle::GetCodePath() const
	{
		return codePath;
	}
}