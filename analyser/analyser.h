#pragma once

#include "error/error.h"
#include "instruction/instruction.h"
#include "tokenizer/token.h"

#include <vector>
#include <optional>
#include <utility>
#include <map>
#include <cstdint>
#include <cstddef> // for std::size_t'
#include <stack>
#include <climits>
#include <string>


namespace miniplc0 {

	class Analyser final {
	private:
		using uint64_t = std::uint64_t;
		using int64_t = std::int64_t;
		using uint32_t = std::uint32_t;
		using int32_t = std::int32_t;
		using string = std::string;
	public:
		Analyser(std::vector<Token> v)
			: _tokens(std::move(v)), _offset(0), _instructions({}), _current_pos(0, 0),
			_uninitialized_vars({}), _vars({}),  _nextTokenIndex(0),  _indexCnt(0), _global_index(0), 
			_allsigns({}), _global_signs({}), _global_vars({}), _global_uninitialized({}){}
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
		std::optional<CompilationError> analyseParameter();
		// <compound-statement>
		std::optional<CompilationError> analyseCompoundStatement();
		// <condition-statement>
		std::optional<CompilationError> analyseConditionStatement();
		// <condition>
		std::optional<CompilationError> analyseCondition(bool fromIf);
		// <statement>
		std::optional<CompilationError> analyseStatement(bool inLoop);
		// <loop-statement>
		std::optional<CompilationError> analyseLoopStatement();
		// <print-statement>
		std::optional<CompilationError> analysePrintStatement();
		// <printable-list>
		std::optional<CompilationError> analysePrintableList();
		// <scan-statement>
		std::optional<CompilationError> analyseScanStatement();
		// <assignment-statement>
		std::optional<CompilationError> analyseAssignmentStatement();
		// <return-statement>
		std::optional<CompilationError> analyseReturnStatement();


		// TODO: Token 缓冲区相关操作

		// 返回下一个 token
		std::optional<Token> nextToken();
		// 回退一个 token
		void unreadToken();

		// 下面是符号表相关操作

		// helper function
		void _add(const Token&, const std::string&, std::map<std::pair<std::string, std::string>, int32_t>&);
		void _add_origin(const Token&, std::map<std::string, int32_t>&);

		// 添加变量、常量、未初始化的变量
		void addVariable(const Token&, const std::string&);
		void addConstant(const Token&, const std::string&);
		void addUninitializedVariable(const Token&, const std::string&);
		void addSign(const Token&, const std::string&);
		void addGlobalSign(const Token&);
		void addGlobalVar(const Token&);
		void addGlobalUninitialized(const Token&);
		void addGlobalConstant(const Token&);



		// 是否被声明过
		bool isDeclared(const std::string&, const std::string& );
		// 是否是未初始化的变量
		bool isUninitializedVariable(const std::string&, const std::string&);
		// 是否是已初始化的变量
		bool isInitializedVariable(const std::string&, const std::string&);
		// 是否是常量	// we still need this... for const can't be reassigned
		bool isConstant(const std::string&, const std::string&);
		// 获得 {变量，常量} 在栈上的偏移
		bool isGlobalDeclared(const std::string&);
		bool isGlobalUninitialized(const std::string&);
		bool isGlobalInitialized(const std::string& );
		bool isGlobalConstant(const std::string&);


		
		int32_t getStackIndex(const std::string&, const std::string&);
		int32_t getGlobalIndex(const std::string&);

		// reset
		void resetOffset();
		void resetLocalIndex();
		void resetLocalMaps();

		// to remember function index in .constants
		void addFunc(std::string, int32_t pos, const TokenType);
		int32_t findFunc(const std::string&);
		TokenType getFuncType(const std::string&);

	private:
		std::vector<Token> _tokens;
		std::size_t _offset;
		std::vector<Instruction> _instructions;
		std::pair<uint64_t, uint64_t> _current_pos;

		// 为了简单处理，我们直接把符号表耦合在语法分析里
		// for local vars only
		// 变量                   示例
		// _uninitialized_vars    var a;
		// _vars                  var a=1;
		// _consts                const a=1;
		// for local variables, there is a string(function name) to see where it is
		std::map<std::pair<std::string, std::string>, int32_t> _uninitialized_vars;	// uninitialized
		std::map<std::pair<std::string, std::string>, int32_t> _vars;	// initialized vars, including consts
		std::map<std::pair<std::string, std::string>, int32_t> _consts;	// const
		std::map<std::pair<std::string, std::string>, int32_t> _allsigns;	// all vars
		std::map<std::string, int32_t> _global_signs;	// global signs
		std::map<std::string, int32_t> _global_vars;	// global initialized vars
		std::map<std::string, int32_t> _global_uninitialized;	// global uninitialized
		std::map<std::string, int32_t> _global_consts;

		// for functions
		std::map<std::string, int32_t> _func_map;
		std::map<std::string, TokenType> _func_type_map;

		// 下一个 token 在栈的偏移
		int32_t _nextTokenIndex;	// actually this var is useless (but we use it... so can't be deleted)
		int32_t _indexCnt = 0;
		int32_t _global_index = 0;
	};
}
