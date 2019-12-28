#include "analyser.h"


namespace miniplc0 {

	// TODO: we can use a stack for temp storage of tokens 
	// in this case, we also need to adjust the implement of nextToken() & unreadToken()

	// some global vars
	std::stack<int> condition_stack;	// store unhandled instructions' index for jump only
	std::stack<int> condition_stack2;
	std::stack<int> loop_stack;	// for "while" jump
	std::vector<std::stack<int>> break_stack;	// for "break"
	std::vector<std::stack<int>> continue_stack;	// for "continue"

	int indexCnt = 0;	// for all index count
	bool returned = false;
	//std::vector<Instruction>::iterator funcIt;	// .functions: ... end pos 	// also, we need to notice that when constIt++, funcIt++ too. (we need to do this manually )
	//std::vector<Instruction>::iterator constIt;	// .constants: ... end pos
	int funcCnt = 0;
	int funcPos = 0;
	bool handleGlobal;	// to flag that what we are handling is global or functional...
	int levelCnt = 0;	// to see which level we are now (for every func-call, we need to +1)
	std::string currentFunc;	// for every single func-difinition / func-call, we need to give its name to this variable
	int param_size_tmp = 0;
	int loopLevel = 0;

	

	std::pair<std::vector<Instruction>, std::optional<CompilationError>> Analyser::Analyse() {
		auto err = analyseProgram();
		if (err.has_value())
			return std::make_pair(std::vector<Instruction>(), err);
		else
			return std::make_pair(_instructions, std::optional<CompilationError>());
	}

	// <C0-program> ::= {<variable-declaration>}{<function-definition>}
	std::optional<CompilationError> Analyser::analyseProgram() {

		_instructions.emplace_back(Operation::CONSTANTS);

		// .start: ... all in this loop
		// every var definied in this loop is global var
		handleGlobal = true;
		_instructions.emplace_back(Operation::START);
		//constIt = _instructions.end() - 1;
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
				else if (next.value().GetType() != TokenType::IDENTIFIER)
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidVariableDeclaration);

				next = nextToken();
				if (!next.has_value())
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidVariableDeclaration);
				else if (next.value().GetType() == TokenType::ASSIGN_SIGN || next.value().GetType() == TokenType::COMMA || next.value().GetType() == TokenType::SEMICOLON)
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



		handleGlobal = false;
		_instructions.emplace_back(Operation::FUNCTIONS);	// .functions:
		funcPos = std::distance(_instructions.begin(), _instructions.end());	// need to be upadted when appending new func or constant (actually they are done together, so +2 each func)
		// this time it's definitely function-definition, so we don't have to pre-read
		while(true)
		{
			indexCnt = 0;
			auto next = nextToken();
			if (!next.has_value())
				break;
			else
				unreadToken();

			// reset local
			levelCnt = 1;
			resetLocalMaps();
			// func-difinition
			auto err = analyseFunctionDifinition();
			if (err.has_value())
				return err;
			levelCnt = 0;
		}


		// ? maybe there are some more things need to take care of before returning...

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
		TokenType type_tmp;
		auto next = nextToken();
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDifinition);
		else if (next.value().GetType() == TokenType::VOID)
		{
			type_tmp = TokenType::VOID;
		}
		else if (next.value().GetType() == TokenType::INT)
		{
			type_tmp = TokenType::INT;
		}
		else
		{
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDifinition);
		}

		// <identifier>
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
		{
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDifinition);
		}
		// isDefinied?
		if (findFunc(next.value().GetValueString()) != -1)
		{
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrFunctionRedefined);
		}
		else
		{
			if (isGlobalDeclared(next.value().GetValueString()))
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrVarFun);
		}
		std::string ident_tmp = next.value().GetValueString();
		// update func name
		currentFunc = next.value().GetValueString();

		// <parameter-clause>
		// (
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDifinition);
		// [<parameter-declaration-list>]
		param_size_tmp = 0;
		next = nextToken();
		if (next.has_value() && next.value().GetType() != TokenType::RIGHT_BRACKET)
		{
			unreadToken();
			auto err = analyseParameterList();
			if (err.has_value())
				return err;
		}
		else
		{
			unreadToken();
		}
		// )
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoRightBracket);


		// add to table
		addFunc(ident_tmp, funcCnt, type_tmp);

		// ok, that's enough for us to genreate .functions
		// .constants first
		// we need to be very careful here!
		Instruction ins_tmp;
		ins_tmp.SetOperation(Operation::CONSTANT);
		ins_tmp.SetIndex(funcCnt);
		ins_tmp.SetStr(ident_tmp);
		_instructions.insert(_instructions.begin()+funcCnt+1, ins_tmp);
		//constIt++;
		funcPos++;
		// then .functions
		// also very careful!
		// I cannot think of a case that level != 1...
		
		_instructions.insert(_instructions.begin()+funcPos, Instruction(Operation::FUNCINFO, funcCnt, 0, funcCnt, param_size_tmp, 1));
		funcPos++;
		param_size_tmp = 0;
		

		// then we enter the "difinition" of this function
		_instructions.emplace_back(Operation::FUNCN, 0, funcCnt);	// we don't need index here, so filling any value is ok
		funcCnt++;

		returned = false;
		// <compound-statement>
		auto err = analyseCompoundStatement(false);
		if (err.has_value())
			return err;
		if (returned == false && getFuncType(currentFunc) != TokenType::VOID)
		{
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoReturn);
		}

		// update func name
		currentFunc = "";
		_instructions.emplace_back(Operation::RET, indexCnt++);
		return {};

	}


	// <compound-statement>
	std::optional<CompilationError> Analyser::analyseCompoundStatement(bool inLoop){
		auto next = nextToken();
		// {
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACE)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDifinition);
		// {variable-declaration} 
		while(true)
		{
			next = nextToken();
			if (!next.has_value())
			{
				unreadToken();
				break;
			}
			else if (next.value().GetType() == TokenType::CONST)
			{
				unreadToken();
				auto err = analyseConstantDeclaration();
				if (err.has_value())
					return err;
			}
			else if (next.value().GetType() == TokenType::INT)
			{
				unreadToken();
				auto err = analyseVariableDeclaration();
				if (err.has_value())
					return err;
			}
			else
			{
				unreadToken();
				break;
			}

		}

		// <statement-seq>
		// pre-read and guide into the right entrance
		while(true)
		{
			next = nextToken();
			if (!next.has_value() || (next.value().GetType() != TokenType::LEFT_BRACE && next.value().GetType() != TokenType::IF && next.value().GetType() != TokenType::WHILE
				&& next.value().GetType() != TokenType::PRINT && next.value().GetType() != TokenType::SCAN && next.value().GetType() != TokenType::IDENTIFIER
				&& next.value().GetType() != TokenType::SEMICOLON && next.value().GetType() != TokenType::RETURN && next.value().GetType() != TokenType::BREAK 
				&& next.value().GetType() != TokenType::CONTINUE && next.value().GetType() != TokenType::FOR))
			{
				unreadToken();
				break;
			}
			else
			{
				unreadToken();
				auto err = analyseStatement(inLoop);
				if (err.has_value())
					return err;
			}
		}


		// }
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACE)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoRightBrace);

		return {};

	}

	// <statement>
	std::optional<CompilationError> Analyser::analyseStatement(bool inLoop){
			auto next = nextToken();
			if (!next.has_value())
			{
				unreadToken();
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidStatement);
			}
			// <compound-statement>
			else if (next.value().GetType() == TokenType::LEFT_BRACE)
			{
				unreadToken();
				auto err = analyseCompoundStatement(inLoop);
				if (err.has_value())
					return err;
			}
			// <condition-statement>
			else if (next.value().GetType() == TokenType::IF)
			{
				unreadToken();
				auto err = analyseConditionStatement(inLoop);
				if (err.has_value())
					return err;

			}
			// <loop-statement>
			else if (next.value().GetType() == TokenType::WHILE || next.value().GetType() == TokenType::FOR)
			{
				unreadToken();
				auto err = analyseLoopStatement();
				if (err.has_value())
					return err;

			}
			// <jump-statement> 
			//(<return-statement>)
			else if (next.value().GetType() == TokenType::RETURN)
			{
				unreadToken();
				auto err = analyseReturnStatement();
				if (err.has_value())
					return err;
			}
			// break;
			else if (next.value().GetType() == TokenType::BREAK)
			{
				if (!inLoop)
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrJmpNotInLoop);
				next = nextToken();
				// ;
				if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
				// instruction
				_instructions.emplace_back(Operation::JMP, indexCnt++, 0);
				// add to stack
				break_stack[loopLevel-1].push(std::distance(_instructions.begin(), _instructions.end()) - 1);
			}
			//continue;
			else if (next.value().GetType() == TokenType::CONTINUE)
			{
				// similar to break
				if (!inLoop)
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrJmpNotInLoop);
				next = nextToken();
				if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
				_instructions.emplace_back(Operation::JMP, indexCnt++, 0);
				continue_stack[loopLevel-1].push(std::distance(_instructions.begin(), _instructions.end()) - 1);
			}
			// <print-statement>
			else if (next.value().GetType() == TokenType::PRINT)
			{
				unreadToken();
				auto err = analysePrintStatement();
				if (err.has_value())
					return err;

			}
			// <scan-statement>
			else if (next.value().GetType() == TokenType::SCAN)
			{
				unreadToken();
				auto err = analyseScanStatement();
				if (err.has_value())
					return err;

			}
			// <assignment-statement> or <function-call>
			else if (next.value().GetType() == TokenType::IDENTIFIER)
			{
				next = nextToken();
				if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET)
				{
					unreadToken();
					unreadToken();
					// <assignment-statement>
					auto err = analyseAssignmentStatement();
					if (err.has_value())
						return err;
					// ;
					next = nextToken();
					if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
						return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidStatement);
				}
				else
				{
					unreadToken();
					unreadToken();
					// <function-call>
					auto err = analyseFunctionCall();
					if (err.has_value())
						return err;
					// ;
					next = nextToken();
					if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
						return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidStatement);					
				}

			}
			// ;
			else if (next.value().GetType() == TokenType::SEMICOLON)
			{
				// do nothing
			}
			else
			{
				unreadToken();
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidStatement);
			}

			return {};

	}


	// <return-statement>
	// <return-statement> ::= 'return' [<expression>] ';'
	std::optional<CompilationError> Analyser::analyseReturnStatement(){
		returned = true;
		// return
		auto next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RETURN)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidStatement);
		// [expression]
		next = nextToken();
		if (!next.has_value())
		{
			if (getFuncType(currentFunc) == TokenType::INT)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidReturn);
			unreadToken();
			_instructions.emplace_back(Operation::RET, indexCnt++);
		}
		else if (next.value().GetType() == TokenType::PLUS_SIGN || next.value().GetType() == TokenType::MINUS_SIGN || next.value().GetType() == TokenType::LEFT_BRACKET
			|| next.value().GetType() == TokenType::UNSIGNED_INTEGER || next.value().GetType() == TokenType::IDENTIFIER)
		{
			if (getFuncType(currentFunc) == TokenType::VOID)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidReturn);
			unreadToken();
			auto err = analyseExpression();
			if (err.has_value())
				return err;
			_instructions.emplace_back(Operation::IRET, indexCnt++);
		}
		else
		{
			if (getFuncType(currentFunc) == TokenType::INT)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidReturn);
			unreadToken();
			_instructions.emplace_back(Operation::RET, indexCnt++);
		}
		// ;
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidStatement);

		return {};
	}


	// <assignment-expression> ::=  <identifier><assignment-operator><expression>
	std::optional<CompilationError> Analyser::analyseAssignmentStatement(){
		// <identifier>
		auto next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidAssignment);
		auto tmpvar = next;
		// load it in advance
		// instruction
		// !!!notice that local vars have higher pirority than global vars 
		if (isDeclared(tmpvar.value().GetValueString(), currentFunc))
		{
			if (isConstant(tmpvar.value().GetValueString(), currentFunc))
			{
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrConstantChange);
			}
			else
			{
				int offset_tmp = getStackIndex(tmpvar.value().GetValueString(), currentFunc);
				_instructions.emplace_back(Operation::LOADA, indexCnt++, offset_tmp, 0);
			}
		}
		else if (isGlobalDeclared(tmpvar.value().GetValueString()))
		{
			if (isGlobalConstant(tmpvar.value().GetValueString()))
			{
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrConstantChange);
			}
			else
			{
				int offset_tmp = getGlobalIndex(tmpvar.value().GetValueString());
				_instructions.emplace_back(Operation::LOADA, indexCnt++, offset_tmp, levelCnt);
			}
		}
		else
		{
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
		}

		// <assignment-operator>
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::ASSIGN_SIGN)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidAssignment);
		// <expression>
		auto err = analyseExpression();
		if (err.has_value())
			return err;
		// after it, add it to our map
		// instruction: store it 
		_instructions.emplace_back(Operation::ISTORE, indexCnt++);

		if (isDeclared(tmpvar.value().GetValueString(), currentFunc))
		{
			addVariable(tmpvar.value(), currentFunc);
		}
		else if (isGlobalDeclared(tmpvar.value().GetValueString()))
		{
			addGlobalVar(tmpvar.value());
		}
		else
		{
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
		}


		return {};

	}

	// <scan-statement>
	/*
	<scan-statement> ::= 
    	'scan' '(' <identifier> ')' ';'
	*/
	 std::optional<CompilationError> Analyser::analyseScanStatement(){
	 	auto next = nextToken();
	 	// scan
	 	if (!next.has_value() || next.value().GetType() != TokenType::SCAN)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidStatement);
		// (
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidStatement);
		// <identifier>
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
		
		// load var first
		// declared? 
		if (isDeclared(next.value().GetValueString(), currentFunc))
		{
			if (isConstant(next.value().GetValueString(), currentFunc))
			{
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrConstantChange);
			}
			int offset_tmp = getStackIndex(next.value().GetValueString(), currentFunc);
			_instructions.emplace_back(Operation::LOADA, indexCnt++, offset_tmp, 0);
			addVariable(next.value(), currentFunc);
		}
		else if (isGlobalDeclared(next.value().GetValueString()))
		{	
			if (isGlobalConstant(next.value().GetValueString()))
			{
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrConstantChange);
			}
			int offset_tmp = getGlobalIndex(next.value().GetValueString());
			_instructions.emplace_back(Operation::LOADA, indexCnt++, offset_tmp, levelCnt);
			addGlobalVar(next.value());
		}
		else
		{
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
		}
		

		// )
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoRightBracket);
		//;
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);

		// iscan and store
		_instructions.emplace_back(Operation::ISCAN, indexCnt++);
		_instructions.emplace_back(Operation::ISTORE, indexCnt++);

	
		return {};

	 }

	// <print-statement>
	/*
	<print-statement> ::= 
    	'print' '(' [<printable-list>] ')' ';'
	<printable-list>  ::= 
    	<printable> {',' <printable>}
	<printable> ::= 
    	<expression> 
	*/
	std::optional<CompilationError> Analyser::analysePrintStatement(){
		auto next = nextToken();
		// print
		if (!next.has_value() || next.value().GetType() != TokenType::PRINT)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidPrint);
		// (
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidPrint);
		// pre-read to see if there is <printable-list> or not
		next = nextToken();
		// )
		if (next.has_value() && next.value().GetType() == TokenType::RIGHT_BRACKET)
		{
			// pass
		}
		// <printable-list>
		else
		{
			unreadToken();
			auto err = analysePrintableList();
			if (err.has_value())
				return err;
			// )
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoRightBracket);
		}
		// \n
		_instructions.emplace_back(Operation::PRINTL, indexCnt++);
		// ;
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
		return {};
	}

	// <printable-list>
	std::optional<CompilationError> Analyser::analysePrintableList(){
		// <expression>
		auto err = analyseExpression();
		if (err.has_value())
			return err;
		_instructions.emplace_back(Operation::IPRINT, indexCnt++);


		while(true)
		{

			auto next = nextToken();
			// ,
			if (!next.has_value() || next.value().GetType() != TokenType::COMMA)
			{
				unreadToken();
				break;
			}
			// space
			_instructions.emplace_back(Operation::BIPUSH, indexCnt++, 32);
			_instructions.emplace_back(Operation::CPRINT, indexCnt++);
			// <expression>
			err = analyseExpression();
			if (err.has_value())
				return err;
			_instructions.emplace_back(Operation::IPRINT, indexCnt++);

		}

		return {};
	}


	// <loop-statement> ::= 'while' '(' <condition> ')' <statement>
	//					|| 'for' '('<for-init-statement> [<condition>]';' [<for-update-expression>]')' <statement>
	/*
	<for-init-statement> ::= 
    	[<assignment-expression>{','<assignment-expression>}]';'
	<for-update-expression> ::=
    	(<assignment-expression>|<function-call>){','(<assignment-expression>|<function-call>)}
	*/
	std::optional<CompilationError> Analyser::analyseLoopStatement(){
		loopLevel++;
		break_stack.push_back(std::stack<int>());
		continue_stack.push_back(std::stack<int>());
		bool isWhile = true;

		int conditionIndex = 0;
		int updateIndexBefore = 0;
		int unhandledJmpOverUpdateIndex = 0;

		// while || for
		auto next = nextToken();
		// while
		if (next.has_value() && next.value().GetType() == TokenType::WHILE)
		{
			isWhile = true;
			next = nextToken();
			// (
			if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidStatement);
			// <condition>
			// if condition not satisfied, we need to jump out of the loop
			// else we continue the loop
			conditionIndex = indexCnt;
			auto err = analyseCondition(false);
			if (err.has_value())
				return err;
			// )
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoRightBracket);
		}
		// for
		else if (next.has_value() && next.value().GetType() == TokenType::FOR)
		{	
			isWhile = false;
			// (
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidStatement);
			// <for-init-statement>
			auto err = analyseForInitStatement();
			if (err.has_value())
				return err;
			// [condition]
			conditionIndex = indexCnt;
			next = nextToken();
			if (next.has_value() && next.value().GetType() == TokenType::COMMA)
			{
				_instructions.emplace_back(Operation::JMP, indexCnt++, 0);
				// add to stack
				loop_stack.push(std::distance(_instructions.begin(), _instructions.end()) - 1);
			}
			else
			{
				unreadToken();
				auto err = analyseCondition(false);
				if (err.has_value())
					return err;
			}
			//;
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
			// jump over update
			_instructions.emplace_back(Operation::JMP, indexCnt++, 0);
			unhandledJmpOverUpdateIndex = std::distance(_instructions.begin(), _instructions.end()) - 1;

			// [for-update-expression]
			updateIndexBefore = indexCnt;
			next = nextToken();
			if (!next.has_value())
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoRightBracket);
			// )
			else if (next.value().GetType() == TokenType::RIGHT_BRACKET)
			{
				// pass
			}
			// for-update
			else
			{
				unreadToken();
				auto err = analyseForUpdate();
				if (err.has_value())
					return err;
				// )
				next = nextToken();
				if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoRightBracket);
			}
			// after update, jmp to condition
			_instructions.emplace_back(Operation::JMP, indexCnt++, conditionIndex);
			// deal with unhandled
			_instructions[unhandledJmpOverUpdateIndex].SetX(indexCnt);
		}
		else
		{
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidStatement);
		}



		// <statement>
		auto err = analyseStatement(true);
		if (err.has_value())
			return err;

		// statement over, handling the previous jump and jump back
		if (loop_stack.empty())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrLoop);
		int tmp_pos = loop_stack.top();	
		//int index_of_condition_ins = _instructions[tmp_pos].GetIndex();	// for another jump's use
		_instructions[tmp_pos].SetX(indexCnt+1);	// for we still have one jump ins here
		loop_stack.pop();

		// deal with 'break' & 'continue'

		// continue
		if (loopLevel > continue_stack.size())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrMyFault);
		std::stack tmp_continue_stack = continue_stack[loopLevel-1];
		while(!tmp_continue_stack.empty())
		{
			int tmp_index = tmp_continue_stack.top();
			_instructions[tmp_index].SetX(conditionIndex);
			tmp_continue_stack.pop();
		}
		// break
		if (loopLevel > break_stack.size())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrMyFault);
		std::stack tmp_break_stack = break_stack[loopLevel-1];
		while(!tmp_break_stack.empty())
		{
			int tmp_index = tmp_break_stack.top();
			_instructions[tmp_index].SetX(indexCnt+1);	// next ins is out of loop
			tmp_break_stack.pop();
		}

		// jmp back
		if (isWhile)
		{
			// jump back to condition calculate
			_instructions.emplace_back(Operation::JMP, indexCnt++, conditionIndex);
		}
		else
		{
			// jmup back to just before update
			_instructions.emplace_back(Operation::JMP, indexCnt++, updateIndexBefore);
		}



		continue_stack.pop_back();
		break_stack.pop_back();
		loopLevel--;
		return {};
	}


	// <for-update-expression>
	// (<assignment-expression>|<function-call>){','(<assignment-expression>|<function-call>)}
	std::optional<CompilationError> Analyser::analyseForUpdate(){
		auto next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
		{
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrForUpdate);
		}
		else 
		{
			// pre-read again
			next = nextToken();
			if (!next.has_value())
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrForUpdate);
			// <assignment-expression>
			else if (next.value().GetType() == TokenType::ASSIGN_SIGN)
			{
				unreadToken();
				unreadToken();
				auto err = analyseAssignmentStatement();
				if (err.has_value())
					return err;
			}
			// <function-call>
			else if (next.value().GetType() == TokenType::LEFT_BRACKET)
			{
				unreadToken();
				unreadToken();
				auto err = analyseFunctionCall();
				if (err.has_value())
					return err;
			}
			else
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrForUpdate);
		}

		while(true)
		{
			next = nextToken();
			if (!next.has_value())
			{
				unreadToken();
				break;
			}
			// ,
			else if (next.value().GetType() == TokenType::COMMA)
			{
				next = nextToken();
				if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
				{
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrForUpdate);
				}
				else 
				{
					// pre-read again
					next = nextToken();
					if (!next.has_value())
						return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrForUpdate);
					// <assignment-expression>
					else if (next.value().GetType() == TokenType::ASSIGN_SIGN)
					{
						unreadToken();
						unreadToken();
						auto err = analyseAssignmentStatement();
						if (err.has_value())
							return err;
					}
					// <function-call>
					else if (next.value().GetType() == TokenType::LEFT_BRACKET)
					{
						unreadToken();
						unreadToken();
						auto err = analyseFunctionCall();
						if (err.has_value())
							return err;
					}
					else
						return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrForUpdate);
				}

				continue;
			}
			// break
			else
			{
				unreadToken();
				break;
			}
		}


		return {};
	}



	// <for-init-statment>
	// [<assignment-expression>{','<assignment-expression>}]';'
	std::optional<CompilationError> Analyser::analyseForInitStatement(){
		auto next = nextToken();
		// ;
		if (next.has_value() && next.value().GetType() == TokenType::SEMICOLON)
		{
			//pass
		}
		// <assignment-expressoin>...
		else if (next.has_value() && next.value().GetType() == TokenType::IDENTIFIER)
		{
			unreadToken();
			auto err = analyseAssignmentStatement();
			if (err.has_value())
				return err;
			while(true)
			{
				auto next2 = nextToken();
				if (next2.has_value() && next2.value().GetType() == TokenType::COMMA)
				{
					// <assignment-expression>
					auto err = analyseAssignmentStatement();
					if (err.has_value())
						return err;
					continue;
				}
				else if (next2.has_value())
				{
					break;
				}
				else
				{
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrForInit);
				}
			}
		}
		else 
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrForInit);

		return {};
	}	

	// <condition-statement>
	/*
	<condition> ::= 
     	<expression>[<relational-operator><expression>] 
	<condition-statement> ::= 
     	'if' '(' <condition> ')' <statement> ['else' <statement>]
	*/
	std::optional<CompilationError> Analyser::analyseConditionStatement(bool inLoop){
		auto next = nextToken();
		// if
		if (!next.has_value() || next.value().GetType() != TokenType::IF)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidConditionStatement);
		// (
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidConditionStatement);
		// <condition>
		auto err = analyseCondition(true);
		if (err.has_value())
			return err;
		// )
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidConditionStatement);

		// if the condition is satisfied, we can process this statement
		// <statement>
		err = analyseStatement(inLoop);
		if (err.has_value())
			return err;



		// ['else' <statement>]
		next = nextToken();
		if (next.has_value() && next.value().GetType() == TokenType::ELSE)
		{
			// for condition satisfied, we also need to jump over "else statement"
			// so we append a jump, stored in condition_stack2
			_instructions.emplace_back(Operation::JMP, indexCnt++, 0);
			condition_stack2.push(std::distance(_instructions.begin(), _instructions.end()) - 1);

			// deal with the if jump
			if (condition_stack.empty())
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIfelse);
			int tmp_pos = condition_stack.top();
			_instructions[tmp_pos].SetX(indexCnt);
			condition_stack.pop();		
			// <statement>
			err = analyseStatement(false);
			if (err.has_value())
				return err;

			// statement over, we need to deal with the "jump over else-statement"
			if (condition_stack2.empty())
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIfelse);
			tmp_pos = condition_stack2.top();
			_instructions[tmp_pos].SetX(indexCnt);
			condition_stack2.pop();
		}
		else
		{
			unreadToken();
			// deal with the if jump
			if (condition_stack.empty())
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIfelse);
			int tmp_pos = condition_stack.top();
			_instructions[tmp_pos].SetX(indexCnt);
			condition_stack.pop();					
		}

		return {};


	}

	// <condition> ::= <expression>[<relational-operator><expression>] 
	std::optional<CompilationError> Analyser::analyseCondition(bool fromIf)	// fromIf: true - calling from condition-statement, false - calling from loop-statement
	{

		auto err = analyseExpression();
		if (err.has_value())
			return err;
		// now the stack has already had the first expression 

		// <realational-operator>
		auto next = nextToken();
		bool flag = true;
		int sign_type = 1;	
		if (!next.has_value())
		{
			unreadToken();
			flag = false;

			sign_type = 1;
		}
		// 0
		else if (next.value().GetType() == TokenType::EQUAL_SIGN)
		{
			sign_type = 0;
		}
		// 1
		else if (next.value().GetType() == TokenType::NOTEQUAL_SIGN)
		{
			sign_type = 1;
		}
		// 2
		else if (next.value().GetType() == TokenType::BIGGER_SIGN)
		{
			sign_type = 2;
		}
		// 3
		else if (next.value().GetType() == TokenType::SMALLER_SIGN)
		{
			sign_type = 3;
		}
		// 4
		else if (next.value().GetType() == TokenType::NOTBIGGER_SIGN)
		{
			sign_type = 4;
		}
		// 5
		else if (next.value().GetType() == TokenType::NOTSMALLER_SIGN)
		{
			sign_type = 5;
		}
		// no sign
		// if (i) is if (i != 0) ?
		else
		{
			unreadToken();
			flag = false;

			sign_type = 1;
		}

		// <expression>
		if (flag)
		{
			auto err = analyseExpression();
			if (err.has_value())
				return err;
			// first - second
			_instructions.emplace_back(Operation::ISUB, indexCnt++);
		}

		// now in both cases, we already push the right value into the stack
		// just need to see which instruction we should use.
		// (I really quite dislike to use switch-case...)

		// ! we need to notice that all operation needs to be reversed
		// for it's who do not fit the condition jumps away
		switch(sign_type)
		{
			case 0:
				_instructions.emplace_back(Operation::JNE, indexCnt++, 0);	// 0 is just for taking the place
				break;
			case 1:
				_instructions.emplace_back(Operation::JE, indexCnt++, 0);
				break;
			case 2:
				_instructions.emplace_back(Operation::JLE, indexCnt++, 0);
				break;
			case 3:
				_instructions.emplace_back(Operation::JGE, indexCnt++, 0);
				break;
			case 4:
				_instructions.emplace_back(Operation::JG, indexCnt++, 0);
				break;
			case 5:
				_instructions.emplace_back(Operation::JL, indexCnt++, 0);
				break;

			default:
				_instructions.emplace_back(Operation::ILL, indexCnt++);	// error occurs
		
		}

		if (fromIf)
			condition_stack.push(std::distance(_instructions.begin(), _instructions.end()) - 1);	// this is the unhandled jmp position of all instructions
		else
			loop_stack.push(std::distance(_instructions.begin(), _instructions.end()) - 1);

		return {};

	}


	// <parameter-declaration-list>
	// ::= <parameter-declaration>{','<parameter-declaration>}
	std::optional<CompilationError> Analyser::analyseParameterList(){
		 auto err = analyseParameter();
		 if (err.has_value())
		 	return err;
		 param_size_tmp++;
		 while(true)
		 {
		 	auto next = nextToken();
		 	// ,
		 	if (!next.has_value() || next.value().GetType() != TokenType::COMMA)
		 	{
		 		unreadToken();
		 		break;
		 	}

		 	err = analyseParameter();
		 	if (err.has_value())
		 		return err;
		 	param_size_tmp++;
		 }

		 return {};

	}

	// <parameter-declaration>
	std::optional<CompilationError> Analyser::analyseParameter(){
		// we need to add param in map
		bool isConst = false;

		auto next = nextToken();
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDifinition);
		else if (next.value().GetType() == TokenType::CONST)
		{
			next = nextToken();
			isConst = true;
		}
		else if (next.value().GetType() == TokenType::INT)
		{
			// pass
		}
		else
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDifinition);

		// I assume that parameter type cannot be 'void'
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDifinition);
		else if (next.value().GetType() == TokenType::INT)
		{
			// pass
		}
		else
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDifinition);

		// identifier
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDifinition);

		// add to map
		addVariable(next.value(), currentFunc);
		if (isConst)
			addConstant(next.value(), currentFunc);
		addSign(next.value(), currentFunc);

		return {};
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
		if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);

		return {};
	}


	// variable declaration
	std::optional<CompilationError> Analyser::analyseVariableDeclaration() {
		auto next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::INT)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidVariableDeclaration);
		// init-decorator-list
		auto err = analyseDecoratorList(false);
		if (err.has_value())
			return err;
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
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
			// <identifier>
			auto next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
			{
				unreadToken();
				break;
			}

			// see if it's already defined?

			if (handleGlobal)
			{
				if (isGlobalDeclared(next.value().GetValueString()))
				{
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrRedeclared);
				}
				if (findFunc(next.value().GetValueString()) != -1)
				{
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrVarFun);
				}
			}
			else
			{
				// even if there is already a global var with this name, we can always declare a local variable with the same name
				// see more on https://github.com/anlideer/c0-compiler/issues/7
				if (isDeclared(next.value().GetValueString(), currentFunc))
				{
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrRedeclared);
				}
			}
			// store identifier
			auto tmpvar = next;
			// global
			if (handleGlobal)
			{
				if (isConst)
				{
					// store const
					_instructions.emplace_back(Operation::IPUSH, indexCnt++, 0);	// 0 is to just take the place
					addGlobalConstant(next.value());
					addGlobalSign(next.value());
				}
				else
				{
					// store var
					_instructions.emplace_back(Operation::IPUSH, indexCnt++, 0);
					addGlobalUninitialized(next.value());
					addGlobalSign(next.value());
				}
			}
			// local
			else
			{
				if (isConst)
				{
					_instructions.emplace_back(Operation::IPUSH, indexCnt++, 0);
					addConstant(next.value(), currentFunc);
					addSign(next.value(), currentFunc);
				}
				else
				{
					_instructions.emplace_back(Operation::IPUSH, indexCnt++, 0);
					addUninitializedVariable(next.value(), currentFunc);
					addSign(next.value(), currentFunc);
				}
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
			else if (next.value().GetType() == TokenType::ASSIGN_SIGN)
			{	
				// load the address, get prepared for istore
				// both global and local, the level is always 0 (just defined...)
				if (handleGlobal)
				{
					_instructions.emplace_back(Operation::LOADA, indexCnt++, getGlobalIndex(tmpvar.value().GetValueString()), 0);
				}
				else
				{
					_instructions.emplace_back(Operation::LOADA, indexCnt++, getStackIndex(tmpvar.value().GetValueString(), currentFunc), 0);
				}

				auto err = analyseExpression();
				if (err.has_value())
					return err;

				// now the number is at the top of the stack, just istore it
				_instructions.emplace_back(Operation::ISTORE, indexCnt++);
				// update the maps
				if (handleGlobal)
				{
					addGlobalVar(tmpvar.value());
				}
				else
				{
					addVariable(tmpvar.value(), currentFunc);
				}

				auto next = nextToken();
				if (next.has_value() && next.value().GetType() == TokenType::COMMA)
				{
					continue;
				}
				else
				{
					unreadToken();
				}

			}
			else
			{
				unreadToken();
				break;
			}

			// const must be initialized
			if (isConst)
			{
				if (isGlobalInitialized(tmpvar.value().GetValueString()) || isInitializedVariable(tmpvar.value().GetValueString(), currentFunc))
				{
					// pass
				}
				else
				{
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrConstantNeedValue);
				}
			}

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
			bool isPlus = false;	// or it's minus
			// + / -
			auto next = nextToken();
			if (!next.has_value())
			{
				unreadToken();
				break;
			}
			// plus
			else if (next.value().GetType() == TokenType::PLUS_SIGN)
			{
				isPlus = true;
			}
			// minus
			else if (next.value().GetType() == TokenType::MINUS_SIGN)
			{
				isPlus = false;
			}
			else
			{
				unreadToken();
				break;
			}

			// mul-exp
			auto err = analyseMulExpression();
			if (err.has_value())
				return err;

			// do the instruction
			if (isPlus)
			{
				_instructions.emplace_back(Operation::IADD, indexCnt++);
			}
			else
			{
				_instructions.emplace_back(Operation::ISUB, indexCnt++);
			}
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
			bool isMul = false;	// or it's DIV
			auto next = nextToken();
			if (!next.has_value())
			{
				unreadToken();
				break;
			}
			else if (next.value().GetType() == TokenType::MULTIPLICATION_SIGN)
			{
				isMul = true;
			}
			else if (next.value().GetType() == TokenType::DIVISION_SIGN)
			{
				isMul = false;
			}
			else
			{
				unreadToken();
				break;
			}

			err = analyseUnaryExpression();
			if (err.has_value())
				return err;

			// do the instruction
			// mul
			if (isMul)
			{
				_instructions.emplace_back(Operation::IMUL, indexCnt++);
			}
			// div
			else
			{
				_instructions.emplace_back(Operation::IDIV, indexCnt++);
			}
		}

		return {};

	}

	// <unary-expression>
	std::optional<CompilationError> Analyser::analyseUnaryExpression()
	{
		auto next = nextToken();
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
		bool needNeg = false;
		if (next.value().GetType() == TokenType::PLUS_SIGN)
		{
			needNeg = false;
		}
		else if (next.value().GetType() == TokenType::MINUS_SIGN)
		{
			needNeg = true;
		}
		else if (next.value().GetType() == TokenType::LEFT_BRACKET || next.value().GetType() == TokenType::IDENTIFIER || next.value().GetType() == TokenType::UNSIGNED_INTEGER)
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
		else if (next.value().GetType() == TokenType::UNSIGNED_INTEGER)
		{
			// push it directly
			_instructions.emplace_back(Operation::IPUSH, indexCnt++, std::any_cast<int>(next.value().GetValue()));
		}
		else if (next.value().GetType() == TokenType::IDENTIFIER)
		{
			// see if it's function-call
			auto next2 = nextToken();
			// just identifier
			if (!next2.has_value() || next2.value().GetType() != TokenType::LEFT_BRACKET)
			{
				unreadToken();
				// load identifier to the top of the stack
				// isdeclared?

				// local
				if (isDeclared(next.value().GetValueString(), currentFunc))
				{
					if (!isInitializedVariable(next.value().GetValueString(), currentFunc))
					{
						return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotInitialized);
					}
					else
					{
						// load
						int offset_tmp = getStackIndex(next.value().GetValueString(), currentFunc);
						_instructions.emplace_back(Operation::LOADA, indexCnt++, offset_tmp, 0);
						_instructions.emplace_back(Operation::ILOAD, indexCnt++);
					}
				}
				// global
				else if (isGlobalDeclared(next.value().GetValueString()))
				{
					if (!isGlobalInitialized(next.value().GetValueString()))
					{
						return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotInitialized);
					}
					else
					{
						// load
						int offset_tmp = getGlobalIndex(next.value().GetValueString());
						_instructions.emplace_back(Operation::LOADA, indexCnt++, offset_tmp, levelCnt);
						_instructions.emplace_back(Operation::ILOAD, indexCnt++);
					}
				}				
				else
				{
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
				}
				
			}
			// func call
			else
			{
				unreadToken();
				unreadToken();
				// remember to push the needed value to stack
				// see if the func returns void or int
				// void is not acceptable
				if (getFuncType(next.value().GetValueString()) != TokenType::INT)
				{
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrVoidCantCalculate);
				}
				auto err = analyseFunctionCall();
				if (err.has_value())
					return err;
			}

		}
		else if (next.value().GetType() == TokenType::LEFT_BRACKET)
		{
			auto err = analyseExpression();
			if(err.has_value())
				return err;
			// )
			auto next2 = nextToken();
			if (!next2.has_value() || next2.value().GetType() != TokenType::RIGHT_BRACKET)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoRightBracket);
		}
		else
		{
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
		}

		// do the negative instruction at last
		if (needNeg)
			_instructions.emplace_back(Operation::INEG, indexCnt++);


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
		// <identifier>
		auto next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteFunctionCall);
		auto func_tmp = next;
		if (findFunc(next.value().GetValueString()) == -1)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoFunction);
		if (isDeclared(next.value().GetValueString(), currentFunc))
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrLocalFunConflict);

		// (
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET)
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
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteFunctionCall);


		// instruction call the func
		_instructions.emplace_back(Operation::CALL, indexCnt++, findFunc(func_tmp.value().GetValueString()));

		return {};
	}

	
	void Analyser::resetOffset()
	{
		_offset = 0;
	}

	void Analyser::resetLocalIndex()
	{
		_indexCnt = 0;
	}

	void Analyser::resetLocalMaps()
	{
		_indexCnt = 0;
		_uninitialized_vars.clear();
		_vars.clear();
		_consts.clear();
		_allsigns.clear();
	}

	std::optional<Token> Analyser::nextToken() 
	{
		if (_offset == _tokens.size())
			return {};
		// 考虑到 _tokens[0..._offset-1] 已经被分析过了
		// 所以我们选择 _tokens[0..._offset-1] 的 EndPos 作为当前位置
		_current_pos = _tokens[_offset].GetEndPos();
		return _tokens[_offset++];
	}

	void Analyser::unreadToken() 
	{
		if (_offset == 0)
			DieAndPrint("analyser unreads token from the begining.");
		_current_pos = _tokens[_offset - 1].GetEndPos();
		_offset--;
	}

	// for local vars add only
	void Analyser::_add(const Token& tk, const std::string& level, std::map<std::pair<std::string, std::string>, int32_t>& mp)
	 {
		if (tk.GetType() != TokenType::IDENTIFIER)
			DieAndPrint("only identifier can be added to the table.");
		mp[std::make_pair(tk.GetValueString(), level)] = _nextTokenIndex;
		_nextTokenIndex++;
	}

	void Analyser::_add_origin(const Token& tk, std::map<string, int32_t>& mp)
	{
		if (tk.GetType() != TokenType::IDENTIFIER)
			DieAndPrint("only identifier can be added to the table.");
		mp[tk.GetValueString()] = _nextTokenIndex;
		_nextTokenIndex++;
	}


	void Analyser::addVariable(const Token& tk, const std::string& level) {
		_add(tk, level, _vars);
	}
	
	void Analyser::addConstant(const Token& tk, const std::string& level) {
		_add(tk, level, _consts);
	}
	
	void Analyser::addUninitializedVariable(const Token& tk, const std::string& level) {
		_add(tk, level, _uninitialized_vars);
	}

	// Attention: it's used to track where the variable is
	void Analyser::addSign(const Token& tk, const std::string& level)
	{
		if (tk.GetType() != TokenType::IDENTIFIER)
			DieAndPrint("only identifier can be added to the table.");
		_allsigns[std::make_pair(tk.GetValueString(), level)] = _indexCnt;
		_indexCnt++;
	}
	
	// Attention: same attention as above
	void Analyser::addGlobalSign(const Token& tk)
	{
		if (tk.GetType() != TokenType::IDENTIFIER)
			DieAndPrint("only identifier can be added to the table.");
		_global_signs[tk.GetValueString()] = _global_index;
		_global_index++;
	}

	void Analyser::addGlobalVar(const Token& tk)
	{
		_add_origin(tk, _global_vars);
	}

	void Analyser::addGlobalUninitialized(const Token& tk)
	{
		_add_origin(tk, _global_uninitialized);
	}

	void Analyser::addGlobalConstant(const Token& tk)
	{
		_add_origin(tk, _global_consts);
	}


	int32_t Analyser::getStackIndex(const std::string& s, const std::string& level)
	{
		return _allsigns[std::make_pair(s, level)];
	}

	int32_t Analyser::getGlobalIndex(const std::string& s)
	{
		return _global_signs[s];
	}

	bool Analyser::isDeclared(const std::string& s, const std::string& level) {
		return  isUninitializedVariable(s, level) || isInitializedVariable(s, level) || isConstant(s, level);
	}

	bool Analyser::isUninitializedVariable(const std::string& s, const std::string& level) {
		return _uninitialized_vars.find(std::make_pair(s, level)) != _uninitialized_vars.end();
	}
	bool Analyser::isInitializedVariable(const std::string& s, const std::string& level) {
		return _vars.find(std::make_pair(s, level)) != _vars.end();
	}
	
	bool Analyser::isConstant(const std::string& s, const std::string& level) {
		return _consts.find(std::make_pair(s, level)) != _consts.end();
	}
	
	bool Analyser::isGlobalDeclared(const std::string& s)
	{
		return isGlobalUninitialized(s) || isGlobalInitialized(s) || isGlobalConstant(s);
	}

	bool Analyser::isGlobalUninitialized(const std::string& s)
	{
		return _global_uninitialized.find(s) != _global_uninitialized.end();
	}

	bool Analyser::isGlobalInitialized(const std::string& s)
	{
		return _global_vars.find(s) != _global_vars.end();
	}

	bool Analyser::isGlobalConstant(const std::string& s)
	{
		return _global_consts.find(s) != _global_consts.end();
	}



	void Analyser::addFunc(std::string name, int32_t pos, const TokenType type)
	{
		
		_func_map[name] = pos;

		_func_type_map[name] = type;
	}

	int32_t Analyser::findFunc(const std::string& name)
	{
		if (_func_map.count(name) == 0)
			return -1;
		else
			return _func_map[name];
	}

	TokenType Analyser::getFuncType(const std::string& name)
	{
		return _func_type_map[name];
	}


}