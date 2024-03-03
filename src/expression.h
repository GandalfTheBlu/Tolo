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

		virtual std::string GetDataType() = 0;
	};

	struct ELoadConstChar : public Expression
	{
		Char value;

		ELoadConstChar(Char _value);

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct ELoadConstInt : public Expression
	{
		Int value;

		ELoadConstInt(Int _value);

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct ELoadConstFloat : public Expression
	{
		Float value;

		ELoadConstFloat(Float _value);

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct ELoadConstString : public Expression
	{
		std::string value;

		ELoadConstString(const std::string& _value);

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct ELoadConstPtr : public Expression
	{
		Ptr p_value;

		ELoadConstPtr(Ptr _p_value);

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct ELoadConstPtrToLabel : public Expression
	{
		std::string labelName;

		ELoadConstPtrToLabel(const std::string& _labelName);

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct ELoadConstBytes : public Expression
	{
		Int bytesSize;
		Ptr p_bytesPtr;

		ELoadConstBytes(Int _bytesSize, Ptr _p_bytesPtr);

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct ELoadBytesFromPtr : public Expression
	{
		std::string dataTypeName;
		Int bytesSize;
		SharedExp ptrLoad;

		ELoadBytesFromPtr(const std::string& _dataTypeName, Int _bytesSize);

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct ELoadPtrWithOffset : public Expression
	{
		Int ptrOffset;
		SharedExp ptrLoad;

		ELoadPtrWithOffset(Int _ptrOffset);

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct EDefineFunction : public Expression
	{
		std::string functionName;
		std::vector<SharedExp> body;

		EDefineFunction(const std::string& _functionName);

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct ELoadVariable : public Expression
	{
		Int varOffset;
		Int varSize;
		std::string varTypeName;

		ELoadVariable(Int _varOffset, Int _varSize, const std::string& _varTypeName);

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct ELoadVariablePtr : public Expression
	{
		Int varOffset;

		ELoadVariablePtr(Int _varOffset);

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct EWriteBytesTo : public Expression
	{
		SharedExp bytesSizeLoad;
		SharedExp writePtrLoad;
		SharedExp dataLoad;

		EWriteBytesTo();

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct ECallFunction : public Expression
	{
		Int paramsSize;
		Int localsSize;
		std::vector<SharedExp> argumentLoads;
		SharedExp functionIpLoad;
		std::string returnTypeName;

		ECallFunction(Int _paramsSize, Int _localsSize, const std::string& _returnTypeName);

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct ECallNativeFunction : public Expression
	{
		std::vector<SharedExp> argumentLoads;
		SharedExp functionPtrLoad;
		std::string returnTypeName;

		ECallNativeFunction(const std::string& _returnTypeName);

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct EBinaryOp : public Expression
	{
		OpCode op;
		SharedExp lhsLoad;
		SharedExp rhsLoad;

		EBinaryOp(OpCode _op);

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct EUnaryOp : public Expression
	{
		OpCode op;
		SharedExp valLoad;

		EUnaryOp(OpCode _op);

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct EScope : public Expression
	{
		std::vector<SharedExp> statements;

		EScope();

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct EReturn : public Expression
	{
		Int retValSize;
		SharedExp retValLoad;

		EReturn(Int _retValSize);

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct EIfSingle : public Expression
	{
		SharedExp conditionLoad;
		std::vector<SharedExp> body;

		EIfSingle();

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};


	struct EIfChain : public Expression
	{
		SharedExp conditionLoad;
		std::vector<SharedExp> body;
		SharedExp chain;

		EIfChain();

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct EElseIfSingle : public Expression
	{
		SharedExp conditionLoad;
		std::vector<SharedExp> body;

		EElseIfSingle();

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct EElseIfChain : public Expression
	{
		SharedExp conditionLoad;
		std::vector<SharedExp> body;
		SharedExp chain;

		EElseIfChain();

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct EElse : public Expression
	{
		std::vector<SharedExp> body;

		EElse();

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct EWhile : public Expression
	{
		SharedExp conditionLoad;
		std::vector<SharedExp> body;

		EWhile();

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct EBreak : public Expression
	{
		EBreak();
		
		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct EContinue : public Expression
	{
		EContinue();

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct EEmpty : public Expression
	{
		EEmpty();

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct ELoadMulti : public Expression
	{
		std::vector<SharedExp> loaders;
		std::string dataTypeName;

		ELoadMulti(const std::string& _dataTypeName);

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};
}