#pragma once
#include "code_builder.h"

namespace Tolo
{
	struct Expression
	{
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

	struct EDefineFunction : public Expression
	{
		std::string functionName;
		std::vector<Expression*> body;

		EDefineFunction(const std::string& _functionName);

		~EDefineFunction();

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
		Expression* p_bytesSizeLoad;
		Expression* p_writePtrLoad;
		Expression* p_dataLoad;

		EWriteBytesTo();

		~EWriteBytesTo();

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct ECallFunction : public Expression
	{
		Int paramsSize;
		Int localsSize;
		std::vector<Expression*> argumentLoads;
		Expression* p_functionIpLoad;
		std::string returnTypeName;

		ECallFunction(Int _paramsSize, Int _localsSize, const std::string& _returnTypeName);

		~ECallFunction();

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct ECallNativeFunction : public Expression
	{
		std::vector<Expression*> argumentLoads;
		Expression* p_functionPtrLoad;
		std::string returnTypeName;

		ECallNativeFunction(const std::string& _returnTypeName);

		~ECallNativeFunction();

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct EBinaryOp : public Expression
	{
		OpCode op;
		Expression* p_lhsLoad;
		Expression* p_rhsLoad;

		EBinaryOp(OpCode _op);

		~EBinaryOp();

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct EUnaryOp : public Expression
	{
		OpCode op;
		Expression* p_valLoad;

		EUnaryOp(OpCode _op);

		~EUnaryOp();

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct EReturn : public Expression
	{
		Int retValSize;
		Expression* p_retValLoad;

		EReturn(Int _retValSize);

		~EReturn();

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct EIfSingle : public Expression
	{
		Expression* p_conditionLoad;
		std::vector<Expression*> body;

		EIfSingle();

		~EIfSingle();

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};


	struct EIfChain : public Expression
	{
		Expression* p_conditionLoad;
		std::vector<Expression*> body;
		Expression* p_chain;

		EIfChain();

		~EIfChain();

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct EElseIfSingle : public Expression
	{
		Expression* p_conditionLoad;
		std::vector<Expression*> body;

		EElseIfSingle();

		~EElseIfSingle();

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct EElseIfChain : public Expression
	{
		Expression* p_conditionLoad;
		std::vector<Expression*> body;
		Expression* p_chain;

		EElseIfChain();

		~EElseIfChain();

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct EElse : public Expression
	{
		std::vector<Expression*> body;

		EElse();

		~EElse();

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};

	struct EWhile : public Expression
	{
		Expression* p_conditionLoad;
		std::vector<Expression*> body;

		EWhile();

		~EWhile();

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
		std::vector<Expression*> loaders;
		std::string dataTypeName;

		ELoadMulti(const std::string& _dataTypeName);

		~ELoadMulti();

		virtual void Evaluate(CodeBuilder& cb) override;

		virtual std::string GetDataType() override;
	};
}