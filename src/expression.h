#pragma once
#include "code_builder.h"
#include <memory>

namespace Tolo
{
	struct Expression
	{
		using SharedExp = std::shared_ptr<Expression>;

		Expression();

		virtual ~Expression();

		virtual void Evaluate(CodeBuilder& cb) = 0;
	};

	struct ELoadConstChar : public Expression
	{
		Char value;

		ELoadConstChar(Char _value);

		virtual void Evaluate(CodeBuilder& cb) override;
	};

	struct ELoadConstInt : public Expression
	{
		Int value;

		ELoadConstInt(Int _value);

		virtual void Evaluate(CodeBuilder& cb) override;
	};

	struct ELoadConstFloat : public Expression
	{
		Float value;

		ELoadConstFloat(Float _value);

		virtual void Evaluate(CodeBuilder& cb) override;
	};

	struct ELoadConstString : public Expression
	{
		std::string value;

		ELoadConstString(const std::string& _value);

		virtual void Evaluate(CodeBuilder& cb) override;
	};

	struct ELoadConstPtr : public Expression
	{
		Ptr p_value;

		ELoadConstPtr(Ptr _p_value);

		virtual void Evaluate(CodeBuilder& cb) override;
	};

	struct ELoadConstPtrToLabel : public Expression
	{
		std::string labelName;

		ELoadConstPtrToLabel(const std::string& _labelName);

		virtual void Evaluate(CodeBuilder& cb) override;
	};

	struct ELoadConstBytes : public Expression
	{
		Int bytesSize;
		Ptr p_bytesPtr;

		ELoadConstBytes(Int _bytesSize, Ptr _p_bytesPtr);

		virtual void Evaluate(CodeBuilder& cb) override;
	};

	struct ELoadBytesFromPtr : public Expression
	{
		Int bytesSize;
		SharedExp ptrLoad;

		ELoadBytesFromPtr(Int _bytesSize);

		virtual void Evaluate(CodeBuilder& cb) override;
	};

	struct ELoadPtrWithOffset : public Expression
	{
		Int ptrOffset;
		SharedExp ptrLoad;

		ELoadPtrWithOffset(Int _ptrOffset);

		virtual void Evaluate(CodeBuilder& cb) override;
	};

	struct EDefineFunction : public Expression
	{
		std::string functionName;
		std::vector<SharedExp> body;

		EDefineFunction(const std::string& _functionName);

		virtual void Evaluate(CodeBuilder& cb) override;
	};

	struct ELoadVariable : public Expression
	{
		Int varOffset;
		Int varSize;

		ELoadVariable(Int _varOffset, Int _varSize);

		virtual void Evaluate(CodeBuilder& cb) override;
	};

	struct ELoadVariablePtr : public Expression
	{
		Int varOffset;

		ELoadVariablePtr(Int _varOffset);

		virtual void Evaluate(CodeBuilder& cb) override;
	};

	struct EWriteBytesTo : public Expression
	{
		SharedExp bytesSizeLoad;
		SharedExp writePtrLoad;
		SharedExp dataLoad;

		EWriteBytesTo();

		virtual void Evaluate(CodeBuilder& cb) override;
	};

	struct ECallFunction : public Expression
	{
		Int paramsSize;
		Int localsSize;
		std::vector<SharedExp> argumentLoads;
		SharedExp functionIpLoad;

		ECallFunction(Int _paramsSize, Int _localsSize);

		virtual void Evaluate(CodeBuilder& cb) override;
	};

	struct ECallNativeFunction : public Expression
	{
		std::vector<SharedExp> argumentLoads;
		SharedExp functionPtrLoad;

		ECallNativeFunction();

		virtual void Evaluate(CodeBuilder& cb) override;
	};

	struct EBinaryOp : public Expression
	{
		OpCode op;
		SharedExp lhsLoad;
		SharedExp rhsLoad;

		EBinaryOp(OpCode _op);

		virtual void Evaluate(CodeBuilder& cb) override;
	};

	struct EUnaryOp : public Expression
	{
		OpCode op;
		SharedExp valLoad;

		EUnaryOp(OpCode _op);

		virtual void Evaluate(CodeBuilder& cb) override;
	};

	struct EScope : public Expression
	{
		std::vector<SharedExp> statements;

		EScope();

		virtual void Evaluate(CodeBuilder& cb) override;
	};

	struct EReturn : public Expression
	{
		Int retValSize;
		SharedExp retValLoad;

		EReturn(Int _retValSize);

		virtual void Evaluate(CodeBuilder& cb) override;
	};

	struct EIfSingle : public Expression
	{
		SharedExp conditionLoad;
		std::vector<SharedExp> body;

		EIfSingle();

		virtual void Evaluate(CodeBuilder& cb) override;
	};


	struct EIfChain : public Expression
	{
		SharedExp conditionLoad;
		std::vector<SharedExp> body;
		SharedExp chain;

		EIfChain();

		virtual void Evaluate(CodeBuilder& cb) override;
	};

	struct EElseIfSingle : public Expression
	{
		SharedExp conditionLoad;
		std::vector<SharedExp> body;

		EElseIfSingle();

		virtual void Evaluate(CodeBuilder& cb) override;
	};

	struct EElseIfChain : public Expression
	{
		SharedExp conditionLoad;
		std::vector<SharedExp> body;
		SharedExp chain;

		EElseIfChain();

		virtual void Evaluate(CodeBuilder& cb) override;
	};

	struct EElse : public Expression
	{
		std::vector<SharedExp> body;

		EElse();

		virtual void Evaluate(CodeBuilder& cb) override;
	};

	struct EWhile : public Expression
	{
		SharedExp conditionLoad;
		std::vector<SharedExp> body;

		EWhile();

		virtual void Evaluate(CodeBuilder& cb) override;
	};

	struct EBreak : public Expression
	{
		EBreak();
		
		virtual void Evaluate(CodeBuilder& cb) override;
	};

	struct EContinue : public Expression
	{
		EContinue();

		virtual void Evaluate(CodeBuilder& cb) override;
	};

	struct EEmpty : public Expression
	{
		EEmpty();

		virtual void Evaluate(CodeBuilder& cb) override;
	};

	struct ELoadMulti : public Expression
	{
		std::vector<SharedExp> loaders;

		ELoadMulti();

		virtual void Evaluate(CodeBuilder& cb) override;
	};
}