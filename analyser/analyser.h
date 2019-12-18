#pragma once

#include "error/error.h"
#include "instruction/instruction.h"
#include "tokenizer/token.h"

#include <vector>
#include <optional>
#include <utility>
#include <map>
#include <cstdint>
#include <cstddef> // for std::size_t

namespace miniplc0 {

	class Analyser final {
	private:
		using uint64_t = std::uint64_t;
		using int64_t = std::int64_t;
		using uint32_t = std::uint32_t;
		using int32_t = std::int32_t;
	public:
		Analyser(std::vector<Token> v)
			: _tokens(std::move(v)), _offset(0), _instructions({}), _current_pos(0, 0),
			_uninitialized_vars({}), _vars({}), _consts({}), _nextTokenIndex(0) {}
		Analyser(Analyser&&) = delete;
		Analyser(const Analyser&) = delete;
		Analyser& operator=(Analyser) = delete;

		// 唯一接口
		std::pair<std::vector<Instruction>, std::optional<CompilationError>> Analyse();
	private:
		// 所有的递归子程序

		// <c0-program>
		std::optional<CompilationError> analyseProgram();
		// const <variable-declaration>
		std::optional<CompilationError> analyseConstantDeclaration();
		// var <variable-declaration>
		std::optional<CompilationError> analyseVariableDeclaration();
		// <expression>
		std::optional<CompilationError> analyseExpression();
		// <function-difinition>
		std::optional<CompilationError> analyseFunctionDifinition();
		// <init-decorator-list>
		std::optional<CompilationError> analyseDecoratorList(bool isConst);
		// <multiplicative-expression>
		std::optional<CompilationError> analyseMulExpression();
		// <function-call>
		std::optional<CompilationError> analyseFunctionCall();
		// <unary-expression>
		std::optional<CompilationError> analyseUnaryExpression();
		// <parameter-declaration-list>
		std::optional<CompilationError> analyseParameterList();
		// <parameter-declaration>
		std::optional<CompilationError> analyseParmater();
		// <compound-statement>
		std::optional<CompilationError> analyseCompoundStatement();
		// <condition-statement>
		std::optional<CompilationError> analyseConditionStatement();
		// <condition>
		std::optional<CompilationError> analyseCondition();
		// <statement>
		std::optional<CompilationError> analyseStatement();


		// TODO: Token 缓冲区相关操作

		// 返回下一个 token
		std::optional<Token> nextToken();
		// 回退一个 token
		void unreadToken();

		// 下面是符号表相关操作

		// helper function
		void _add(const Token&, std::map<std::string, int32_t>&);
		// 添加变量、常量、未初始化的变量
		void addVariable(const Token&);
		void addConstant(const Token&);
		void addUninitializedVariable(const Token&);
		void addSignal(const Token&);
		// 是否被声明过
		bool isDeclared(const std::string&);
		// 是否是未初始化的变量
		bool isUninitializedVariable(const std::string&);
		// 是否是已初始化的变量
		bool isInitializedVariable(const std::string&);
		// 是否是常量
		bool isConstant(const std::string&);
		// 获得 {变量，常量} 在栈上的偏移
		int32_t getIndex(const std::string&);
		int32_t getStackIndex(const std::string&);
	private:
		std::vector<Token> _tokens;
		std::size_t _offset;
		std::vector<Instruction> _instructions;
		std::pair<uint64_t, uint64_t> _current_pos;

		// 为了简单处理，我们直接把符号表耦合在语法分析里
		// 变量                   示例
		// _uninitialized_vars    var a;
		// _vars                  var a=1;
		// _consts                const a=1;
		std::map<std::string, int32_t> _uninitialized_vars;
		std::map<std::string, int32_t> _vars;
		std::map<std::string, int32_t> _consts;
		std::map<std::string, int32_t> _allsigns;
		// 下一个 token 在栈的偏移
		int32_t _nextTokenIndex;
		int32_t _indexCnt = 0;
	};
}
