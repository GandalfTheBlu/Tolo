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
		if(retValLoad != nullptr)
			retValLoad->Evaluate(cb);

		cb.Op(OpCode::Return);
		cb.ConstInt(retValSize);
	}

	std::string EReturn::GetDataType()
	{
		if (retValLoad == nullptr)
			return "void";
		else
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
		cb.currentWhileDepth++;
		std::string depthId = std::to_string(cb.currentWhileDepth);

		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel(depthId + "__while_condition__");
		cb.Op(OpCode::Write_IP);

		cb.DefineLabel(depthId + "__while_body__");
		for (auto e : body)
			e->Evaluate(cb);

		cb.DefineLabel(depthId + "__while_condition__");
		cb.RemoveLabel(depthId + "__while_condition__");
		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel(depthId + "__while_body__");
		cb.RemoveLabel(depthId + "__while_body__");
		conditionLoad->Evaluate(cb);
		cb.Op(OpCode::Write_IP_If);

		cb.DefineLabel(depthId + "__while_end__");
		cb.RemoveLabel(depthId + "__while_end__");

		cb.currentWhileDepth--;
	}

	std::string EWhile::GetDataType()
	{
		return "void";
	}


	EIfSingle::EIfSingle() :
		conditionLoad(nullptr)
	{}

	EIfSingle::~EIfSingle()
	{
		delete conditionLoad;

		for (auto e : body)
			delete e;
	}

	void EIfSingle::Evaluate(CodeBuilder& cb)
	{
		cb.currentBranchDepth++;
		std::string depthId = std::to_string(cb.currentBranchDepth);

		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel(depthId + "__if_body__");
		conditionLoad->Evaluate(cb);
		cb.Op(OpCode::Write_IP_If);

		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel(depthId + "__if_end__");
		cb.Op(OpCode::Write_IP);

		cb.DefineLabel(depthId + "__if_body__");
		cb.RemoveLabel(depthId + "__if_body__");

		for (auto e : body)
			e->Evaluate(cb);

		cb.DefineLabel(depthId + "__if_end__");
		cb.RemoveLabel(depthId + "__if_end__");

		cb.currentBranchDepth--;
	}

	std::string EIfSingle::GetDataType()
	{
		return "void";
	}


	EIfChain::EIfChain() :
		conditionLoad(nullptr),
		chain(nullptr)
	{}

	EIfChain::~EIfChain()
	{
		delete conditionLoad;

		for (auto e : body)
			delete e;

		delete chain;
	}

	void EIfChain::Evaluate(CodeBuilder& cb)
	{
		cb.currentBranchDepth++;
		std::string depthId = std::to_string(cb.currentBranchDepth);

		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel(depthId + "__if_body__");
		conditionLoad->Evaluate(cb);
		cb.Op(OpCode::Write_IP_If);

		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel(depthId + "__if_end__");
		cb.Op(OpCode::Write_IP);

		cb.DefineLabel(depthId + "__if_body__");
		cb.RemoveLabel(depthId + "__if_body__");

		for (auto e : body)
			e->Evaluate(cb);

		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel(depthId + "__chain_end__");
		cb.Op(OpCode::Write_IP);

		cb.DefineLabel(depthId + "__if_end__");
		cb.RemoveLabel(depthId + "__if_end__");

		chain->Evaluate(cb);

		cb.DefineLabel(depthId + "__chain_end__");
		cb.RemoveLabel(depthId + "__chain_end__");

		cb.currentBranchDepth--;
	}

	std::string EIfChain::GetDataType()
	{
		return "void";
	}


	EElseIfSingle::EElseIfSingle() :
		conditionLoad(nullptr)
	{}

	EElseIfSingle::~EElseIfSingle()
	{
		delete conditionLoad;

		for (auto e : body)
			delete e;
	}

	void EElseIfSingle::Evaluate(CodeBuilder& cb)
	{
		std::string depthId = std::to_string(cb.currentBranchDepth);

		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel(depthId + "__if_body__");
		conditionLoad->Evaluate(cb);
		cb.Op(OpCode::Write_IP_If);

		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel(depthId + "__if_end__");
		cb.Op(OpCode::Write_IP);

		cb.DefineLabel(depthId + "__if_body__");
		cb.RemoveLabel(depthId + "__if_body__");

		for (auto e : body)
			e->Evaluate(cb);

		cb.DefineLabel(depthId + "__if_end__");
		cb.RemoveLabel(depthId + "__if_end__");
	}

	std::string EElseIfSingle::GetDataType()
	{
		return "void";
	}


	EElseIfChain::EElseIfChain() :
		conditionLoad(nullptr),
		chain(nullptr)
	{}

	EElseIfChain::~EElseIfChain()
	{
		delete conditionLoad;

		for (auto e : body)
			delete e;

		delete chain;
	}

	void EElseIfChain::Evaluate(CodeBuilder& cb)
	{
		std::string depthId = std::to_string(cb.currentBranchDepth);

		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel(depthId + "__if_body__");
		conditionLoad->Evaluate(cb);
		cb.Op(OpCode::Write_IP_If);

		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel(depthId + "__if_end__");
		cb.Op(OpCode::Write_IP);

		cb.DefineLabel(depthId + "__if_body__");
		cb.RemoveLabel(depthId + "__if_body__");

		for (auto e : body)
			e->Evaluate(cb);

		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel(depthId + "__chain_end__");
		cb.Op(OpCode::Write_IP);

		cb.DefineLabel(depthId + "__if_end__");
		cb.RemoveLabel(depthId + "__if_end__");

		chain->Evaluate(cb);
	}

	std::string EElseIfChain::GetDataType()
	{
		return "void";
	}


	EElse::EElse()
	{}

	EElse::~EElse()
	{
		for (auto e : body)
			delete e;
	}

	void EElse::Evaluate(CodeBuilder& cb)
	{
		for (auto e : body)
			e->Evaluate(cb);
	}

	std::string EElse::GetDataType()
	{
		return "void";
	}

	
	EBreak::EBreak()
	{}

	void EBreak::Evaluate(CodeBuilder& cb)
	{
		std::string depthId = std::to_string(cb.currentWhileDepth);
		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel(depthId + "__while_end__");
		cb.Op(OpCode::Write_IP);
	}

	std::string EBreak::GetDataType()
	{
		return "void";
	}


	EContinue::EContinue()
	{}

	void EContinue::Evaluate(CodeBuilder& cb)
	{
		std::string depthId = std::to_string(cb.currentWhileDepth);
		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel(depthId + "__while_condition__");
		cb.Op(OpCode::Write_IP);
	}

	std::string EContinue::GetDataType()
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