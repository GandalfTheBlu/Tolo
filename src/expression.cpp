#include "expression.h"

namespace Tolo
{
	Expression::Expression()
	{}

	Expression::~Expression()
	{}


	ELoadConstChar::ELoadConstChar(Char _value) :
		value(_value)
	{}

	void ELoadConstChar::Evaluate(CodeBuilder& cb)
	{
		cb.Op(OpCode::Load_Const_Char);
		cb.ConstChar(value);
	}

	std::string ELoadConstChar::GetDataType()
	{
		return "char";
	}


	ELoadConstInt::ELoadConstInt(Int _value) :
		value(_value)
	{}

	void ELoadConstInt::Evaluate(CodeBuilder& cb)
	{
		cb.Op(OpCode::Load_Const_Int);
		cb.ConstInt(value);
	}

	std::string ELoadConstInt::GetDataType()
	{
		return "int";
	}


	ELoadConstFloat::ELoadConstFloat(Float _value) :
		value(_value)
	{}

	void ELoadConstFloat::Evaluate(CodeBuilder& cb) 
	{
		cb.Op(OpCode::Load_Const_Float);
		cb.ConstFloat(value);
	}

	std::string ELoadConstFloat::GetDataType()
	{
		return "float";
	}


	ELoadConstPtr::ELoadConstPtr(Ptr _value) :
		value(_value)
	{}

	void ELoadConstPtr::Evaluate(CodeBuilder& cb) 
	{
		cb.Op(OpCode::Load_Const_Ptr);
		cb.ConstPtr(value);
	}

	std::string ELoadConstPtr::GetDataType()
	{
		return "ptr";
	}


	ELoadConstPtrToLabel::ELoadConstPtrToLabel(const std::string& _labelName) :
		labelName(_labelName)
	{}

	void ELoadConstPtrToLabel::Evaluate(CodeBuilder& cb)
	{
		cb.Op(OpCode::Load_Const_Ptr);
		cb.ConstPtrToLabel(labelName);
	}

	std::string ELoadConstPtrToLabel::GetDataType()
	{
		return "ptr";
	}


	EDefineFunction::EDefineFunction(const std::string& _functionName) :
		functionName(_functionName)
	{}

	EDefineFunction::~EDefineFunction()
	{
		for (auto e : body)
			delete e;
	}

	std::string EDefineFunction::GetDataType()
	{
		return "void";
	}


	void EDefineFunction::Evaluate(CodeBuilder& cb) 
	{
		cb.DefineLabel(functionName);

		for (auto e : body)
			e->Evaluate(cb);
	}

	ELoadVariable::ELoadVariable(Int _varOffset, Int _varSize, const std::string& _varTypeName) :
		varOffset(_varOffset),
		varSize(_varSize),
		varTypeName(_varTypeName)
	{}

	void ELoadVariable::Evaluate(CodeBuilder& cb) 
	{
		cb.Op(OpCode::Load_FP);
		cb.Op(OpCode::Load_Const_Int); cb.ConstInt(-12 - varOffset);
		cb.Op(OpCode::Ptr_Add);
		cb.Op(OpCode::Load_Const_Int); cb.ConstInt(varSize);
		cb.Op(OpCode::Load_Bytes_From);
	}

	std::string ELoadVariable::GetDataType()
	{
		return varTypeName;
	}


	ELoadVariablePtr::ELoadVariablePtr(Int _varOffset) :
		varOffset(_varOffset)
	{}

	void ELoadVariablePtr::Evaluate(CodeBuilder& cb) 
	{
		cb.Op(OpCode::Load_FP);
		cb.Op(OpCode::Load_Const_Int); cb.ConstInt(-12 - varOffset);
		cb.Op(OpCode::Ptr_Add);
	}

	std::string ELoadVariablePtr::GetDataType()
	{
		return "ptr";
	}


	EWriteBytesTo::EWriteBytesTo() :
		bytesSizeLoad(nullptr),
		writePtrLoad(nullptr),
		dataLoad(nullptr)
	{}

	EWriteBytesTo::~EWriteBytesTo()
	{
		delete bytesSizeLoad;
		delete writePtrLoad;
		delete dataLoad;
	}

	void EWriteBytesTo::Evaluate(CodeBuilder& cb) 
	{
		dataLoad->Evaluate(cb);
		writePtrLoad->Evaluate(cb);
		bytesSizeLoad->Evaluate(cb);
		cb.Op(OpCode::Write_Bytes_To);
	}

	std::string EWriteBytesTo::GetDataType()
	{
		return "void";
	}


	ECallFunction::ECallFunction(Int _paramsSize, Int _localsSize, const std::string& _returnTypeName) :
		paramsSize(_paramsSize),
		localsSize(_localsSize),
		functionIpLoad(nullptr),
		returnTypeName(_returnTypeName)
	{}

	ECallFunction::~ECallFunction()
	{
		for (auto e : argumentLoads)
			delete e;

		delete functionIpLoad;
	}

	void ECallFunction::Evaluate(CodeBuilder& cb)
	{
		for (int i = (int)argumentLoads.size() - 1; i >= 0; i--)
			argumentLoads[i]->Evaluate(cb);

		functionIpLoad->Evaluate(cb);

		cb.Op(OpCode::Call);
		cb.ConstInt(paramsSize);
		cb.ConstInt(localsSize);
	}

	std::string ECallFunction::GetDataType()
	{
		return returnTypeName;
	}


	EBinaryOp::EBinaryOp(OpCode _op) :
		op(_op),
		lhsLoad(nullptr),
		rhsLoad(nullptr)
	{}

	EBinaryOp::~EBinaryOp()
	{
		delete lhsLoad;
		delete rhsLoad;
	}

	void EBinaryOp::Evaluate(CodeBuilder& cb) 
	{
		rhsLoad->Evaluate(cb);
		lhsLoad->Evaluate(cb);
		cb.Op(op);
	}

	std::string EBinaryOp::GetDataType()
	{
		return lhsLoad->GetDataType();
	}


	EReturn::EReturn(Int _retValSize) :
		retValSize(_retValSize),
		retValLoad(nullptr)
	{}

	EReturn::~EReturn()
	{
		delete retValLoad;
	}

	void EReturn::Evaluate(CodeBuilder& cb) 
	{
		retValLoad->Evaluate(cb);
		cb.Op(OpCode::Return);
		cb.ConstInt(retValSize);
	}

	std::string EReturn::GetDataType()
	{
		return retValLoad->GetDataType();
	}


	EWhile::EWhile() :
		conditionLoad(nullptr)
	{}

	EWhile::~EWhile()
	{
		delete conditionLoad;

		for (auto e : body)
			delete e;
	}

	void EWhile::Evaluate(CodeBuilder& cb)
	{
		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel("__while_condition__");
		cb.Op(OpCode::Write_IP);

		cb.DefineLabel("__while_body__");
		for (auto e : body)
			e->Evaluate(cb);

		cb.DefineLabel("__while_condition__");
		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel("__while_body__");
		conditionLoad->Evaluate(cb);
		cb.Op(OpCode::Write_IP_If);

		cb.RemoveLabel("__while_condition__");
		cb.RemoveLabel("__while_body__");
	}

	std::string EWhile::GetDataType()
	{
		return "void";
	}


	EIf::EIf() :
		conditionLoad(nullptr)
	{}

	EIf::~EIf()
	{
		delete conditionLoad;

		for (auto e : body)
			delete e;
	}

	void EIf::Evaluate(CodeBuilder& cb)
	{
		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel("__if_body__");
		conditionLoad->Evaluate(cb);
		cb.Op(OpCode::Write_IP_If);

		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel("__if_end__");
		cb.Op(OpCode::Write_IP);

		cb.DefineLabel("__if_body__");
		for (auto e : body)
			e->Evaluate(cb);

		cb.DefineLabel("__if_end__");

		cb.RemoveLabel("__if_body__");
		cb.RemoveLabel("__if_end__");
	}

	std::string EIf::GetDataType()
	{
		return "void";
	}


	EDebugPrint::EDebugPrint(OpCode _printOp) :
		printOp(_printOp),
		valLoad(nullptr)
	{}

	EDebugPrint::~EDebugPrint()
	{
		delete valLoad;
	}

	void EDebugPrint::Evaluate(CodeBuilder& cb)
	{
		valLoad->Evaluate(cb);
		cb.Op(printOp);
	}

	std::string EDebugPrint::GetDataType()
	{
		return "void";
	}
}