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


	ELoadConstInt::ELoadConstInt(Int _value) :
		value(_value)
	{}

	void ELoadConstInt::Evaluate(CodeBuilder& cb)
	{
		cb.Op(OpCode::Load_Const_Int);
		cb.ConstInt(value);
	}


	ELoadConstFloat::ELoadConstFloat(Float _value) :
		value(_value)
	{}

	void ELoadConstFloat::Evaluate(CodeBuilder& cb) 
	{
		cb.Op(OpCode::Load_Const_Float);
		cb.ConstFloat(value);
	}


	ELoadConstString::ELoadConstString(const std::string& _value) :
		value(_value)
	{}

	void ELoadConstString::Evaluate(CodeBuilder& cb)
	{
		cb.Op(OpCode::Load_Const_Ptr);
		cb.ConstStringPtr(value);
	}


	ELoadConstPtr::ELoadConstPtr(Ptr _p_value) :
		p_value(_p_value)
	{}

	void ELoadConstPtr::Evaluate(CodeBuilder& cb) 
	{
		cb.Op(OpCode::Load_Const_Ptr);
		cb.ConstPtr(p_value);
	}


	ELoadConstPtrToLabel::ELoadConstPtrToLabel(const std::string& _labelName) :
		labelName(_labelName)
	{}

	void ELoadConstPtrToLabel::Evaluate(CodeBuilder& cb)
	{
		cb.Op(OpCode::Load_Const_Ptr);
		cb.ConstPtrToLabel(labelName);
	}


	ELoadConstBytes::ELoadConstBytes(Int _bytesSize, Ptr _p_bytesPtr) : 
		bytesSize(_bytesSize),
		p_bytesPtr(_p_bytesPtr)
	{}

	void ELoadConstBytes::Evaluate(CodeBuilder& cb)
	{
		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtr(p_bytesPtr);
		cb.Op(OpCode::Load_Const_Int); cb.ConstInt(bytesSize);
		cb.Op(OpCode::Load_Bytes_From);
	}


	ELoadBytesFromPtr::ELoadBytesFromPtr(Int _bytesSize) :
		bytesSize(_bytesSize)
	{}

	void ELoadBytesFromPtr::Evaluate(CodeBuilder& cb)
	{
		ptrLoad->Evaluate(cb);
		cb.Op(OpCode::Load_Const_Int); cb.ConstInt(bytesSize);
		cb.Op(OpCode::Load_Bytes_From);
	}


	EDefineFunction::EDefineFunction(const std::string& _functionName) :
		functionName(_functionName)
	{}


	void EDefineFunction::Evaluate(CodeBuilder& cb) 
	{
		cb.DefineLabel(functionName);

		for (auto e : body)
			e->Evaluate(cb);
	}

	ELoadVariable::ELoadVariable(Int _varOffset, Int _varSize) :
		varOffset(_varOffset),
		varSize(_varSize)
	{}

	void ELoadVariable::Evaluate(CodeBuilder& cb) 
	{
		cb.Op(OpCode::Load_Const_Int); cb.ConstInt(-static_cast<Int>(sizeof(Ptr) + sizeof(Ptr) + sizeof(Int)) + varOffset);
		cb.Op(OpCode::Load_FP);
		cb.Op(OpCode::Ptr_Add);
		cb.Op(OpCode::Load_Const_Int); cb.ConstInt(varSize);
		cb.Op(OpCode::Load_Bytes_From);
	}


	ELoadVariablePtr::ELoadVariablePtr(Int _varOffset) :
		varOffset(_varOffset)
	{}

	void ELoadVariablePtr::Evaluate(CodeBuilder& cb) 
	{
		cb.Op(OpCode::Load_Const_Int); cb.ConstInt(-static_cast<Int>(sizeof(Ptr) + sizeof(Ptr) + sizeof(Int)) + varOffset);
		cb.Op(OpCode::Load_FP);
		cb.Op(OpCode::Ptr_Add);
	}


	EPtrAdd::EPtrAdd()
	{}

	void EPtrAdd::Evaluate(CodeBuilder& cb)
	{
		cb.Op(OpCode::Ptr_Add);
	}


	ELoadPtrFromStackTopPtr::ELoadPtrFromStackTopPtr()
	{}

	void ELoadPtrFromStackTopPtr::Evaluate(CodeBuilder& cb)
	{
		cb.Op(OpCode::Load_Const_Int); cb.ConstInt(static_cast<Int>(sizeof(Ptr)));
		cb.Op(OpCode::Load_Bytes_From);
	}


	EWriteBytesTo::EWriteBytesTo()
	{}

	void EWriteBytesTo::Evaluate(CodeBuilder& cb) 
	{
		dataLoad->Evaluate(cb);
		writePtrLoad->Evaluate(cb);
		bytesSizeLoad->Evaluate(cb);
		cb.Op(OpCode::Write_Bytes_To);
	}


	ECallFunction::ECallFunction(Int _paramsSize, Int _localsSize) :
		paramsSize(_paramsSize),
		localsSize(_localsSize)
	{}

	void ECallFunction::Evaluate(CodeBuilder& cb)
	{
		for (int i = (int)argumentLoads.size() - 1; i >= 0; i--)
			argumentLoads[i]->Evaluate(cb);

		functionIpLoad->Evaluate(cb);

		cb.Op(OpCode::Call);
		cb.ConstInt(paramsSize);
		cb.ConstInt(localsSize);
	}


	ECallNativeFunction::ECallNativeFunction()
	{}

	void ECallNativeFunction::Evaluate(CodeBuilder& cb)
	{
		for (int i = (int)argumentLoads.size() - 1; i >= 0; i--)
			argumentLoads[i]->Evaluate(cb);

		functionPtrLoad->Evaluate(cb);

		cb.Op(OpCode::Call_Native);
	}


	EBinaryOp::EBinaryOp(OpCode _op) :
		op(_op)
	{}

	void EBinaryOp::Evaluate(CodeBuilder& cb) 
	{
		rhsLoad->Evaluate(cb);
		lhsLoad->Evaluate(cb);
		cb.Op(op);
	}


	EUnaryOp::EUnaryOp(OpCode _op) :
		op(_op)
	{}

	void EUnaryOp::Evaluate(CodeBuilder& cb)
	{
		valLoad->Evaluate(cb);
		cb.Op(op);
	}


	EScope::EScope()
	{}

	void EScope::Evaluate(CodeBuilder& cb)
	{
		for (auto e : statements)
			e->Evaluate(cb);
	}


	EReturn::EReturn(Int _retValSize) :
		retValSize(_retValSize),
		retValLoad(nullptr)
	{}

	void EReturn::Evaluate(CodeBuilder& cb) 
	{
		if(retValLoad != nullptr)
			retValLoad->Evaluate(cb);

		cb.Op(OpCode::Return);
		cb.ConstInt(retValSize);
	}


	EGoto::EGoto()
	{}

	void EGoto::Evaluate(CodeBuilder& cb)
	{
		instrPtrLoad->Evaluate(cb);
		cb.Op(OpCode::Write_IP);
	}


	EWhile::EWhile()
	{}

	void EWhile::Evaluate(CodeBuilder& cb)
	{
		cb.currentWhileDepth++;
		std::string depthId = std::to_string(cb.currentWhileDepth);

		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel(depthId + "while_condition");
		cb.Op(OpCode::Write_IP);

		cb.DefineLabel(depthId + "while_body");
		for (auto e : body)
			e->Evaluate(cb);

		cb.DefineLabel(depthId + "while_condition");
		cb.RemoveLabel(depthId + "while_condition");
		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel(depthId + "while_body");
		cb.RemoveLabel(depthId + "while_body");
		conditionLoad->Evaluate(cb);
		cb.Op(OpCode::Write_IP_If);

		cb.DefineLabel(depthId + "while_end");
		cb.RemoveLabel(depthId + "while_end");

		cb.currentWhileDepth--;
	}


	EIfSingle::EIfSingle()
	{}

	void EIfSingle::Evaluate(CodeBuilder& cb)
	{
		cb.currentBranchDepth++;
		std::string depthId = std::to_string(cb.currentBranchDepth);

		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel(depthId + "if_body");
		conditionLoad->Evaluate(cb);
		cb.Op(OpCode::Write_IP_If);

		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel(depthId + "if_end");
		cb.Op(OpCode::Write_IP);

		cb.DefineLabel(depthId + "if_body");
		cb.RemoveLabel(depthId + "if_body");

		for (auto e : body)
			e->Evaluate(cb);

		cb.DefineLabel(depthId + "if_end");
		cb.RemoveLabel(depthId + "if_end");

		cb.currentBranchDepth--;
	}


	EIfChain::EIfChain()
	{}

	void EIfChain::Evaluate(CodeBuilder& cb)
	{
		cb.currentBranchDepth++;
		std::string depthId = std::to_string(cb.currentBranchDepth);

		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel(depthId + "if_body");
		conditionLoad->Evaluate(cb);
		cb.Op(OpCode::Write_IP_If);

		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel(depthId + "if_end");
		cb.Op(OpCode::Write_IP);

		cb.DefineLabel(depthId + "if_body");
		cb.RemoveLabel(depthId + "if_body");

		for (auto e : body)
			e->Evaluate(cb);

		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel(depthId + "chain_end");
		cb.Op(OpCode::Write_IP);

		cb.DefineLabel(depthId + "if_end");
		cb.RemoveLabel(depthId + "if_end");

		chain->Evaluate(cb);

		cb.DefineLabel(depthId + "chain_end");
		cb.RemoveLabel(depthId + "chain_end");

		cb.currentBranchDepth--;
	}


	EElseIfSingle::EElseIfSingle()
	{}

	void EElseIfSingle::Evaluate(CodeBuilder& cb)
	{
		std::string depthId = std::to_string(cb.currentBranchDepth);

		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel(depthId + "if_body");
		conditionLoad->Evaluate(cb);
		cb.Op(OpCode::Write_IP_If);

		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel(depthId + "if_end");
		cb.Op(OpCode::Write_IP);

		cb.DefineLabel(depthId + "if_body");
		cb.RemoveLabel(depthId + "if_body");

		for (auto e : body)
			e->Evaluate(cb);

		cb.DefineLabel(depthId + "if_end");
		cb.RemoveLabel(depthId + "if_end");
	}


	EElseIfChain::EElseIfChain()
	{}

	void EElseIfChain::Evaluate(CodeBuilder& cb)
	{
		std::string depthId = std::to_string(cb.currentBranchDepth);

		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel(depthId + "if_body");
		conditionLoad->Evaluate(cb);
		cb.Op(OpCode::Write_IP_If);

		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel(depthId + "if_end");
		cb.Op(OpCode::Write_IP);

		cb.DefineLabel(depthId + "if_body");
		cb.RemoveLabel(depthId + "if_body");

		for (auto e : body)
			e->Evaluate(cb);

		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel(depthId + "chain_end");
		cb.Op(OpCode::Write_IP);

		cb.DefineLabel(depthId + "if_end");
		cb.RemoveLabel(depthId + "if_end");

		chain->Evaluate(cb);
	}


	EElse::EElse()
	{}

	void EElse::Evaluate(CodeBuilder& cb)
	{
		for (auto e : body)
			e->Evaluate(cb);
	}

	
	EBreak::EBreak(Int _depth, int _line) :
		depth(_depth),
		line(_line)
	{}

	void EBreak::Evaluate(CodeBuilder& cb)
	{
		Int destinationDepth = cb.currentWhileDepth - (depth - 1);

		Affirm(
			destinationDepth <= cb.currentWhileDepth,
			"break depth at line %i must be 1 or greater",
			line
		);

		Affirm(
			destinationDepth > 0,
			"break depth at line %i is greater than current loop depth",
			line
		);

		std::string depthId = std::to_string(destinationDepth);
		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel(depthId + "while_end");
		cb.Op(OpCode::Write_IP);
	}


	EContinue::EContinue(Int _depth, int _line) :
		depth(_depth),
		line(_line)
	{}

	void EContinue::Evaluate(CodeBuilder& cb)
	{
		Int destinationDepth = cb.currentWhileDepth - (depth - 1);

		Affirm(
			destinationDepth <= cb.currentWhileDepth,
			"continue depth at line %i must be 1 or greater",
			line
		);

		Affirm(
			destinationDepth > 0,
			"continue depth at line %i is greater than current loop depth",
			line
		);

		std::string depthId = std::to_string(destinationDepth);
		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel(depthId + "while_condition");
		cb.Op(OpCode::Write_IP);
	}


	EEmpty::EEmpty()
	{}

	void EEmpty::Evaluate(CodeBuilder& cb)
	{}


	ELoadMulti::ELoadMulti()
	{}

	void ELoadMulti::Evaluate(CodeBuilder& cb)
	{
		for (auto e : loaders)
			e->Evaluate(cb);
	}


	EVTable::EVTable(const std::string& _vTableName) :
		vTableName(_vTableName)
	{}

	void EVTable::Evaluate(CodeBuilder& cb)
	{
		cb.DefineLabel("0virtual_table_" + vTableName);
		
		for (const std::string& funcLabel : functionLabels)
			cb.ConstPtrToLabel(funcLabel);
	}


	ELoadVTablePtr::ELoadVTablePtr(const std::string& _vTableName) :
		vTableName(_vTableName)
	{}

	void ELoadVTablePtr::Evaluate(CodeBuilder& cb)
	{
		cb.Op(OpCode::Load_Const_Ptr); cb.ConstPtrToLabel("0virtual_table_" + vTableName);
	}
}