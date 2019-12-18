#include "analyser.h"
#include <climits>

namespace miniplc0 {

	// TODO: we can use a queue for temp storage of tokens 
	// in this case, we also need to adjust the implement of nextToken() & unreadToken()
	

	std::pair<std::vector<Instruction>, std::optional<CompilationError>> Analyser::Analyse() {
		auto err = analyseProgram();
		if (err.has_value())
			return std::make_pair(std::vector<Instruction>(), err);
		else
			return std::make_pair(_instructions, std::optional<CompilationError>());
	}

	// <C0-program> ::= {<variable-declaration>}{<function-definition>}
	std::optional<CompilationError> Analyser::analyseProgram() {

		while(true)
		{
			// pre-read (I'm so fucking lazy...)
			bool isVar = false;
			bool isConst = false;
			auto next = nextToken();
			if (!next.has_value())
				break;
			else if (next.value().GetType() == TokenType::CONST)
			{
				isVar = true;
				isConst = true;
				unreadToken();
			}
			else if (next.value().GetType() == TokenType::INT || next.value().GetType() == TokenType::VOID)
			{
				// pre-read again and again
				next = nextToken();
				if (!next.has_value())
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidVariableDeclaration);
				else if (next.value() != TokenType::IDENTIFIER)
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidVariableDeclaration);

				next = nextToken();
				if (!next.has_value())
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidVariableDeclaration);
				else if (next.value().GetType() == TokenType::EQUAL_SIGN || nextToken.value().GetType() == TokenType::COMMA || nextToken.value().GetType() == TokenType::SEMICOLON)
				{
					isVar = true;
					unreadToken();
					unreadToken();
					unreadToken();
				}
				else if (next.value().GetType() == TokenType::LEFT_BRACKET)
				{
					isVar = false;
					unreadToken();
					unreadToken();
					unreadToken();
				}
				else
				{
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidVariableDeclaration);
				}

			}
			else
			{
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidVariableDeclaration);
			}

			// const / var
			if (isVar && isConst)
			{
				auto err = analyseConstantDeclaration();
				if (err.has_value())
					return err;
			}
			else if (isVar && !isConst)
			{
				// variable-declaration
				auto err = analyseVariableDeclaration();
				if (err.has_value())
					return err;
			}
			else
			{
				break;
			}
		}

		// this time it's definitely function-definition, so we don't have to pre-read
		while(true)
		{
			auto err = analyseFunctionDifinition();
			if (err.has_value())
				return err;
		}

		return {};

	}

	// <function-difinition>
	/*
	<function-definition> ::= 
    	<type-specifier><identifier><parameter-clause><compound-statement>
		<type-specifier> ::= 'void' | 'int'
		<parameter-clause> ::= 
    		'(' [<parameter-declaration-list>] ')'
		<parameter-declaration-list> ::= 
    		<parameter-declaration>{','<parameter-declaration>}
		<parameter-declaration> ::= 
    		[<const-qualifier>]<type-specifier><identifier>
    	<compound-statement> ::= 
    		'{' {<variable-declaration>} <statement-seq> '}'
		<statement-seq> ::= 
			{<statement>}
		<statement> ::= 
     		<compound-statement>
    		|<condition-statement>
    		|<loop-statement>
    		|<jump-statement>
    		|<print-statement>
    		|<scan-statement>
    		|<assignment-expression>';'
    		|<function-call>';'
    		|';'   
	*/
	std::optional<CompilationError> Analyser::analyseFunctionDifinition(){
		// <type-specifier>
		auto next = nextToken();
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDifinition);


	}


	// <variable-declaration> ::= [<const-qualifier>]<type-specifier><init-declarator-list>';'
	// I'll deal with consts and vars apart
	std::optional<CompilationError> Analyser::analyseConstantDeclaration() {
		
		auto next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::CONST)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidVariableDeclaration);
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::INT)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidVariableDeclaration);
		// init-decorator-list
		auto err = analyseDecoratorList(true);
		if (err.has_value())
			return err;
		next = nextToken();
		if (!next.has_value() || next.value() != TokenType::SEMICOLON)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);

		return {};
	}


	// variable declaration
	std::optional<CompilationError> Analyser::analyseVariableDeclaration() {
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::INT)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidVariableDeclaration);
		// init-decorator-list
		auto err = analyseDecoratorList(false);
		if (err.has_value())
			return err;
		next = nextToken();
		if (!next.has_value() || next.value() != TokenType::SEMICOLON)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);

		return {};
	}

	// <init-decorator-list>
	/*
	<init-declarator-list> ::= 
    	<init-declarator>{','<init-declarator>}
	<init-declarator> ::= 
    	<identifier>[<initializer>]
	<initializer> ::= 
    	'='<expression>
    */
	std::optional<CompilationError> Analyser::analyseDecoratorList(bool isConst){
		while(true)
		{
			auto next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
			{
				unreadToken();
				break;
			}
			next = nextToken();
			if (!next.has_value())
			{
				unreadToken();
				break;
			}
			// ,
			else if (next.value().GetType() == TokenType::COMMA)
			{
				continue;
			}
			// =
			else if (next.value().GetType() == TokenType::EQUAL_SIGN)
			{
				auto err = analyseExpression();
				if (err.has_value())
					return err;
			}
			else
			{
				unreadToken();
				break;
			}
		}

		return {};
	}

	// <语句序列> ::= {<语句>}
	// <语句> :: = <赋值语句> | <输出语句> | <空语句>
	// <赋值语句> :: = <标识符>'='<表达式>';'
	// <输出语句> :: = 'print' '(' <表达式> ')' ';'
	// <空语句> :: = ';'
	// 需要补全
	std::optional<CompilationError> Analyser::analyseStatementSequence() {
		while (true) {
			// 预读
			auto next = nextToken();
			if (!next.has_value())
				return {};
			unreadToken();
			if (next.value().GetType() != TokenType::IDENTIFIER &&
				next.value().GetType() != TokenType::PRINT &&
				next.value().GetType() != TokenType::SEMICOLON) {
				return {};
			}
			std::optional<CompilationError> err;
			switch (next.value().GetType()) {
				// 这里需要你针对不同的预读结果来调用不同的子程序
				case  TokenType::IDENTIFIER:
				{
					auto ident = analyseAssignmentStatement();
					if (ident.has_value())
						return ident;
					break;
				}
				case TokenType::PRINT:
				{
					auto printSt = analyseOutputStatement();
					if (printSt.has_value())
						return printSt;
					break;
				}
				// 注意我们没有针对空语句单独声明一个函数，因此可以直接在这里返回 
				// and "read" on
				case TokenType::SEMICOLON:
				{
					next = nextToken();
					break;
				}
				default:
					break;
			}
		}
		return {};
	}

	// <常表达式> ::= [<符号>]<无符号整数>
	// 需要补全
	std::optional<CompilationError> Analyser::analyseConstantExpression(int32_t& out) {
		// out 是常表达式的结果
		// 这里你要分析常表达式并且计算结果
		// 注意以下均为常表达式
		// +1 -1 1
		// !!! 同时要注意是否溢出
		auto next = nextToken();

		if (!next.has_value() || (next.value().GetType() != TokenType::PLUS_SIGN && next.value().GetType() != TokenType::MINUS_SIGN && next.value().GetType() != TokenType::UNSIGNED_INTEGER))
		{
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
		}

		// + none -
		// + 
		if (next.value().GetType() == TokenType::PLUS_SIGN)
		{
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::UNSIGNED_INTEGER)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
			// TODO: overflow
			
			out = std::any_cast<int32_t>(next.value().GetValue());
		}
		// no
		else if (next.value().GetType() == TokenType::UNSIGNED_INTEGER)
		{
			// TODO: overflow
			out = std::any_cast<int32_t>(next.value().GetValue());
		}
		// -
		else 
		{
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::UNSIGNED_INTEGER)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::UNSIGNED_INTEGER)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
			// TODO: overflow

			out = -1 * std::any_cast<int32_t>(next.value().GetValue());
		}

		return {};
	}

	// <expression> equals to  <additive-expression>
	// <additive-expression> ::= <multiplicative-expression>{<additive-operator><multiplicative-expression>}
	std::optional<CompilationError> Analyser::analyseExpression() {
		auto err = analyseMulExpression();
		if (err.has_value())
			return err;
		while(true)
		{
			// + / -
			auto next = nextToken();
			if (!next.has_value() || (next.value().GetType() != TokenType::PLUS_SIGN && next.value().GetType() != TokenType::MINUS_SIGN))
			{
				unreadToken();
				break;
			}
			// mul-exp
			auto err = analyseMulExpression();
			if (err.has_value())
				return err;
		}

		return {};
	}

	// <multiplication-expression>
	/*
	<multiplicative-expression> ::= 
    	<unary-expression>{<multiplicative-operator><unary-expression>}
	<unary-expression> ::=
    	[<unary-operator>]<primary-expression>
	<primary-expression> ::=  
   		 '('<expression>')' 
    	|<identifier>
    	|<integer-literal>
    	|<function-call>
	*/
	std::optional<CompilationError> Analyser::analyseMulExpression(){
		auto err = analyseUnaryExpression();
		if (err.has_value())
			return err;
		while(true)
		{
			auto next = nextToken();
			if (!next.has_value())
			{
				unreadToken();
				break;
			}
			else if (next.value().GetType() == TokenType::MULTIPLICATION_SIGN)
			{
				// ...
			}
			else if (next.value().GetType() == TokenType::DIVISION_SIGN)
			{
				// ...
			}
			else
			{
				unreadToken();
				break;
			}

			err = analyseUnaryExpression();
			if (err.has_value())
				return err;
		}

		return {};

	}

	// <unary-expression>
	std::optional<CompilationError> Analyser::analyseUnaryExpression()
	{
		auto next = nextToken();
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
		if (next.value().GetType() == TokenType::PLUS_SIGN)
		{
			// ...
		}
		else if (next.value().GetType() == TokenType::MINUS_SIGN)
		{
			// ...
		}
		else if (next.value().GetType() == TokenType::LEFT_BRACE || next.value().GetType() == TokenType::IDENTIFIER || next.value().GetType() == TokenType::UNSIGNED_INTEGER)
		{
			unreadToken();
		}
		else
		{
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
		}

		// primary-expression
		next = nextToken();
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
		else if (next.has_value().GetType() == TokenType::UNSIGNED_INTEGER)
		{
			// ...
		}
		else if (next.has_value().GetType() == TokenType::IDENTIFIER)
		{
			// see if it's function-call
			auto next2 = nextToken();
			// just identifier
			if (!next2.has_value() || next2.has_value().GetType() != TokenType::LEFT_BRACE)
			{
				unreadToken();
			}
			// func call
			else
			{
				unreadToken();
				unreadToken();
				auto err = analyseFunctionCall();
				if (err.has_value())
					return err;
			}

		}
		else if (next.has_value().GetType() == TokenType::LEFT_BRACE)
		{
			auto err = analyseExpression();
			if(err.has_value())
				return err;
		}
		else
		{
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
		}

		return {};
	}

	// <function-call>
	/*
	<function-call> ::= 
    	<identifier> '(' [<expression-list>] ')'
	<expression-list> ::= 
    	<expression>{','<expression>}
	*/
	std::optional<CompilationError> Analyser::analyseFunctionCall(){
		auto next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteFunctionCall);

		// (
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACE)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteFunctionCall);
		// [expression-list]
		auto err = analyseExpression();
		if (err.has_value())
			return err;
		while(true)
		{
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::COMMA)
			{
				unreadToken();
				break;
			}
			else
			{
				err = analyseExpression();
				if (err.has_value())
					return err;
			}
		}
		// )
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACE)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteFunctionCall);

		return {};
	}

	

	// <赋值语句> ::= <标识符>'='<表达式>';'
	std::optional<CompilationError> Analyser::analyseAssignmentStatement() {
		// 这里除了语法分析以外还要留意
		// 标识符声明过吗？
		// 标识符是常量吗？
		// 需要生成指令吗？
		auto next = nextToken();
		if (!next.has_value())
			return {};
		if (next.value().GetType() != TokenType::IDENTIFIER)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidAssignment);
		// declared?
		if (!isDeclared(next.value().GetValueString()))
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
		// constant?
		if (isConstant(next.value().GetValueString()))
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrAssignToConstant);

		// =
		auto equl_sign = nextToken();
		if (!equl_sign.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidAssignment);

		// expression
		auto exp = analyseExpression();
		if (exp.has_value())
			return exp;
			
		// instructions
		_instructions.emplace_back(Operation::STO, getStackIndex(next.value().GetValueString()));
		addVariable(next.value());



		return {};
	}

	// <输出语句> ::= 'print' '(' <表达式> ')' ';'
	std::optional<CompilationError> Analyser::analyseOutputStatement() {
		// 如果之前 <语句序列> 的实现正确，这里第一个 next 一定是 TokenType::PRINT
		auto next = nextToken();

		// '('
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidPrint);

		// <表达式>
		auto err = analyseExpression();
		if (err.has_value())
			return err;

		// ')'
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidPrint);

		// ';'
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);

		// 生成相应的指令 WRT
		_instructions.emplace_back(Operation::WRT, 0);
		return {};
	}

	// <项> :: = <因子>{ <乘法型运算符><因子> }
	// 需要补全
	std::optional<CompilationError> Analyser::analyseItem() {
		// 可以参考 <表达式> 实现
		auto fac = analyseFactor();
		if (fac.has_value())
			return fac;

		while(true)
		{
			// pre-read
			auto next = nextToken();
			if (!next.has_value())
				return {};
			if (next.value().GetType() != TokenType::MULTIPLICATION_SIGN && next.value().GetType() != TokenType::DIVISION_SIGN)
			{
				unreadToken();
				return {};
			}

			// <factor>
			fac = analyseFactor();
			if (fac.has_value())
				return fac;

			// instructions

			// *
			if (next.value().GetType() == TokenType::MULTIPLICATION_SIGN)
			{
				_instructions.emplace_back(Operation::MUL, 0);
			}
			// /
			else
			{
				_instructions.emplace_back(Operation::DIV, 0);
			}

		}


		return {};
	}

	// <因子> ::= [<符号>]( <标识符> | <无符号整数> | '('<表达式>')' )
	// 需要补全
	std::optional<CompilationError> Analyser::analyseFactor() {
		// [<符号>]
		auto next = nextToken();
		auto prefix = 1;
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
		if (next.value().GetType() == TokenType::PLUS_SIGN)
			prefix = 1;
		else if (next.value().GetType() == TokenType::MINUS_SIGN) {
			prefix = -1;
			_instructions.emplace_back(Operation::LIT, 0);
		}
		else
			unreadToken();

		// 预读
		next = nextToken();
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
		switch (next.value().GetType()) {
			// 这里和 <语句序列> 类似，需要根据预读结果调用不同的子程序
			// 但是要注意 default 返回的是一个编译错误
			case TokenType::IDENTIFIER:
			{
				// declared?
				if(!isDeclared(next.value().GetValueString()))
				{
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
				}
				// initialized?
				if (!isInitializedVariable(next.value().GetValueString()) && !isConstant(next.value().GetValueString()))
				{
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotInitialized);
				}
				_instructions.emplace_back(Operation::LOD, getStackIndex(next.value().GetValueString()));
				break;

			}	
			case TokenType::UNSIGNED_INTEGER:
			{
				// TODO: overflow
				
				_instructions.emplace_back(Operation::LIT, std::any_cast<int32_t>(next.value().GetValue()));
				break;
			}
			case TokenType::LEFT_BRACKET:
			{
				auto exp = analyseExpression();
				if (exp.has_value())
					return exp;
				next = nextToken();
				if (!next.has_value())
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoRightBracket);
				break;
			}

			default:
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
		}

		// 取负
		if (prefix == -1)
			_instructions.emplace_back(Operation::SUB, 0);
		return {};
	}

	std::optional<Token> Analyser::nextToken() {
		if (_offset == _tokens.size())
			return {};
		// 考虑到 _tokens[0..._offset-1] 已经被分析过了
		// 所以我们选择 _tokens[0..._offset-1] 的 EndPos 作为当前位置
		_current_pos = _tokens[_offset].GetEndPos();
		return _tokens[_offset++];
	}

	void Analyser::unreadToken() {
		if (_offset == 0)
			DieAndPrint("analyser unreads token from the begining.");
		_current_pos = _tokens[_offset - 1].GetEndPos();
		_offset--;
	}

	void Analyser::_add(const Token& tk, std::map<std::string, int32_t>& mp) {
		if (tk.GetType() != TokenType::IDENTIFIER)
			DieAndPrint("only identifier can be added to the table.");
		mp[tk.GetValueString()] = _nextTokenIndex;
		_nextTokenIndex++;
	}

	void Analyser::addVariable(const Token& tk) {
		_add(tk, _vars);
	}

	void Analyser::addConstant(const Token& tk) {
		_add(tk, _consts);
	}

	void Analyser::addUninitializedVariable(const Token& tk) {
		_add(tk, _uninitialized_vars);
	}

	void Analyser::addSignal(const Token& tk)
	{
		if (tk.GetType() != TokenType::IDENTIFIER)
			DieAndPrint("only identifier can be added to the table.");
		_allsigns[tk.GetValueString()] = _indexCnt;
		_indexCnt++;
	}

	int32_t Analyser::getIndex(const std::string& s) {

		if (_uninitialized_vars.find(s) != _uninitialized_vars.end())
			return _uninitialized_vars[s];
		else if (_vars.find(s) != _vars.end())
			return _vars[s];
		else
			return _consts[s];
	}

	int32_t Analyser::getStackIndex(const std::string& s)
	{
		return _allsigns[s];
	}

	bool Analyser::isDeclared(const std::string& s) {
		return isConstant(s) || isUninitializedVariable(s) || isInitializedVariable(s);
	}

	bool Analyser::isUninitializedVariable(const std::string& s) {
		return _uninitialized_vars.find(s) != _uninitialized_vars.end();
	}
	bool Analyser::isInitializedVariable(const std::string&s) {
		return _vars.find(s) != _vars.end();
	}

	bool Analyser::isConstant(const std::string&s) {
		return _consts.find(s) != _consts.end();
	}
}