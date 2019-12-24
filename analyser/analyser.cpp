#include "analyser.h"
#include <climits>
#include <queue>

namespace miniplc0 {

	// TODO: we can use a queue for temp storage of tokens 
	// in this case, we also need to adjust the implement of nextToken() & unreadToken()

	// some global vars
	std::queue<int32_t> jmp_queue;	// store unhandled instructions' index for jump only
	std::queue<int32_t> call_queue;	// for call only
	int globalCnt = 0;
	int localCnt = 0;
	

	std::pair<std::vector<Instruction>, std::optional<CompilationError>> Analyser::Analyse() {
		auto err = analyseProgram();
		if (err.has_value())
			return std::make_pair(std::vector<Instruction>(), err);
		else
			return std::make_pair(_instructions, std::optional<CompilationError>());
	}

	// <C0-program> ::= {<variable-declaration>}{<function-definition>}
	std::optional<CompilationError> Analyser::analyseProgram() {

		// normal procedure
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

		// this time it's definitely function-definition, so we don't have to pre-read
		while(true)
		{
			auto next = nextToken();
			if (!next.has_value())
				break;
			else
				unreadToken();
			auto err = analyseFunctionDifinition();
			if (err.has_value())
				return err;
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
		auto next = nextToken();
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDifinition);
		else if (next.value().GetType() == TokenType::VOID)
		{
			// ...
		}
		else if (next.value().GetType() == TokenType::INT)
		{
			// ...
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

		// <parameter-clause>
		// (
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDifinition);
		// [<parameter-declaration-list>]
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


		// <compound-statement>
		auto err = analyseCompoundStatement();
		if (err.has_value())
			return err;

		return {};

	}


	// <compound-statement>
	std::optional<CompilationError> Analyser::analyseCompoundStatement(){
		auto next = nextToken();
		// {
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACE)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDifinition);
		// {variable-declaration} 
		while(true)
		{
			next = nextToken();
			if (!next.has_value() || (next.value().GetType() != TokenType::CONST && next.value().GetType() != TokenType::INT))
			{
				unreadToken();
				break;
			}

			unreadToken();
			auto err = analyseVariableDeclaration();
			if (err.has_value())
				return err;
		}

		// <statement-seq>
		// pre-read and guide into the right entrance
		while(true)
		{
			next = nextToken();
			if (!next.has_value() || (next.value().GetType() != TokenType::LEFT_BRACE && next.value().GetType() != TokenType::IF && next.value().GetType() != TokenType::WHILE
				&& next.value().GetType() != TokenType::PRINT && next.value().GetType() != TokenType::SCAN && next.value().GetType() != TokenType::IDENTIFIER
				&& next.value().GetType() != TokenType::SEMICOLON))
			{
				unreadToken();
				break;
			}
			else
			{
				unreadToken();
				auto err = analyseStatement();
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
	std::optional<CompilationError> Analyser::analyseStatement(){
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
				auto err = analyseCompoundStatement();
				if (err.has_value())
					return err;
			}
			// <condition-statement>
			else if (next.value().GetType() == TokenType::IF)
			{
				unreadToken();
				auto err = analyseConditionStatement();
				if (err.has_value())
					return err;

			}
			// <loop-statement>
			else if (next.value().GetType() == TokenType::WHILE)
			{
				unreadToken();
				auto err = analyseLoopStatement();
				if (err.has_value())
					return err;

			}
			// <jump-statement> (now only <return-statement>)
			else if (next.value().GetType() == TokenType::RETURN)
			{
				unreadToken();
				auto err = analyseReturnStatement();
				if (err.has_value())
					return err;
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
				}
				else
				{
					unreadToken();
					unreadToken();
					// <function-call>
					auto err = analyseFunctionCall();
					if (err.has_value())
						return err;

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
		// return
		auto next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RETURN)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidStatement);
		// [expression]
		next = nextToken();
		if (!next.has_value())
		{
			unreadToken();
		}
		else if (next.value().GetType() == TokenType::PLUS_SIGN || next.value().GetType() == TokenType::MINUS_SIGN || next.value().GetType() == TokenType::LEFT_BRACKET
			|| next.value().GetType() == TokenType::UNSIGNED_INTEGER || next.value().GetType() == TokenType::IDENTIFIER)
		{
			unreadToken();
			auto err = analyseExpression();
			if (err.has_value())
				return err;
		}
		else
		{
			unreadToken();
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
		// <assignment-operator>
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::ASSIGN_SIGN)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidAssignment);
		// <expression>
		auto err = analyseExpression();
		if (err.has_value())
			return err;

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
		// ...

		// )
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoRightBracket);
		//;
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);

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
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidStatement);
		// (
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidStatement);
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
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidStatement);

		while(true)
		{
			auto next = nextToken();
			// ,
			if (!next.has_value() || next.value().GetType() != TokenType::COMMA)
			{
				unreadToken();
				break;
			}
			// <expression>
			err = analyseExpression();
			if (err.has_value())
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidStatement);
		}

		return {};
	}


	// <loop-statement> ::= 'while' '(' <condition> ')' <statement>
	std::optional<CompilationError> Analyser::analyseLoopStatement(){
		auto next = nextToken();
		// while
		if (!next.has_value() || next.value().GetType() != TokenType::WHILE)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidStatement);
		next = nextToken();
		// (
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidStatement);
		// <condition>
		auto err = analyseCondition();
		if (err.has_value())
			return err;
		// )
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoRightBracket);

		// <statement>
		err = analyseStatement();
		if (err.has_value())
			return err;

	}


	// <condition-statement>
	/*
	<condition> ::= 
     	<expression>[<relational-operator><expression>] 
	<condition-statement> ::= 
     	'if' '(' <condition> ')' <statement> ['else' <statement>]
	*/
	std::optional<CompilationError> Analyser::analyseConditionStatement(){
		auto next = nextToken();
		// if
		if (!next.has_value() || next.value().GetType() != TokenType::IF)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidConditionStatement);
		// (
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidConditionStatement);
		// <condition>
		auto err = analyseCondition();
		if (err.has_value())
			return err;
		// )
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidConditionStatement);

		// <statement>
		err = analyseStatement();
		if (err.has_value())
			return err;

		// ['else' <statement>]
		next = nextToken();
		if (next.has_value() && next.value().GetType() == TokenType::ELSE)
		{
			// <statement>
			err = analyseStatement();
			if (err.has_value())
				return err;
		}
		else
		{
			unreadToken();
		}

		return {};


	}

	// <condition> ::= <expression>[<relational-operator><expression>] 
	std::optional<CompilationError> Analyser::analyseCondition(){
		auto err = analyseExpression();
		if (err.has_value())
			return err;
		// <realational-operator>
		auto next = nextToken();
		bool flag = true;
		if (!next.has_value())
		{
			unreadToken();
			flag = false;
		}
		else if (next.value().GetType() == TokenType::EQUAL_SIGN)
		{
			// ...
		}
		else if (next.value().GetType() == TokenType::NOTEQUAL_SIGN)
		{
			// ...
		}
		else if (next.value().GetType() == TokenType::BIGGER_SIGN)
		{
			// ...
		}
		else if (next.value().GetType() == TokenType::SMALLER_SIGN)
		{
			// ... 
		}
		else if (next.value().GetType() == TokenType::NOTBIGGER_SIGN)
		{
			// ...
		}
		else if (next.value().GetType() == TokenType::NOTSMALLER_SIGN)
		{
			// ...
		}
		else
		{
			unreadToken();
			flag = false;
		}

		// <expression>
		if (flag)
		{
			auto err = analyseExpression();
			if (err.has_value())
				return err;
		}

		return {};

	}


	// <parameter-declaration-list>
	// ::= <parameter-declaration>{','<parameter-declaration>}
	std::optional<CompilationError> Analyser::analyseParameterList(){
		 auto err = analyseParameter();
		 if (err.has_value())
		 	return err;
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
		 }

		 return {};

	}

	// <parameter-declaration>
	std::optional<CompilationError> Analyser::analyseParameter(){
		auto next = nextToken();
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDifinition);
		else if (next.value().GetType() == TokenType::CONST)
		{
			next = nextToken();
			// ...
		}
		else if (next.value().GetType() == TokenType::INT)
		{
			// ...
		}
		else
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDifinition);

		// I assume that parameter type cannot be 'void'
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDifinition);
		else if (next.value().GetType() == TokenType::INT)
		{
			// ...
		}
		else
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDifinition);

		// identifier
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDifinition);
		// ...

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
			else if (next.value().GetType() == TokenType::ASSIGN_SIGN)
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
			// ...
		}
		else if (next.value().GetType() == TokenType::IDENTIFIER)
		{
			// see if it's function-call
			auto next2 = nextToken();
			// just identifier
			if (!next2.has_value() || next2.value().GetType() != TokenType::LEFT_BRACKET)
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
		else if (next.value().GetType() == TokenType::LEFT_BRACKET)
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
	void Analyser::_add(const Token& tk, const std::string level, std::map<std::pair<std::string, std::string>, int32_t>& mp)
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


	void Analyser::addVariable(const Token& tk, const std::string level) {
		_add(tk, level, _vars);
	}
	/*
	void Analyser::addConstant(const Token& tk) {
		_add(tk, _consts);
	}
	*/
	void Analyser::addUninitializedVariable(const Token& tk, const std::string level) {
		_add(tk, level, _uninitialized_vars);
	}

	// Attention: it's used to track where the variable is
	void Analyser::addSign(const Token& tk, const std::string level)
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

	// abandon
	/*
	int32_t Analyser::getIndex(const std::string& s) {

		if (_uninitialized_vars.find(s) != _uninitialized_vars.end())
			return _uninitialized_vars[s];
		else if (_vars.find(s) != _vars.end())
			return _vars[s];
		else
			return _consts[s];
	}
	*/

	int32_t Analyser::getStackIndex(const std::string& s, const std::string& level)
	{
		return _allsigns[std::make_pair(s, level)];
	}

	int32_t Analyser::getGlobalIndex(const std::string &s)
	{
		return _global_signs[s];
	}

	bool Analyser::isDeclared(const std::string& s, const std::string& level) {
		return  isUninitializedVariable(s, level) || isInitializedVariable(s, level);
	}

	bool Analyser::isUninitializedVariable(const std::string& s, const std::string level) {
		return _uninitialized_vars.find(std::make_pair(s, level)) != _uninitialized_vars.end();
	}
	bool Analyser::isInitializedVariable(const std::string& s, const std::string level) {
		return _vars.find(std::make_pair(s, level)) != _vars.end();
	}
	/*
	bool Analyser::isConstant(const std::string&s) {
		return _consts.find(s) != _consts.end();
	}
	*/
	bool Analyser::isGlobalDeclared(const std::string& s)
	{
		return isGlobalUninitialized(s) || isGlobalInitialized(s);
	}

	bool Analyser::isGlobalUninitialized(const std::string& s)
	{
		return _global_uninitialized.find(s) != _global_uninitialized.end();
	}

	bool Analyser::isGlobalInitialized(const std::string& s)
	{
		return _global_vars.find(s) != _global_vars.end();
	}


	void Analyser::addFunc(const Token& tk, int32_t pos)
	{
		if (tk.GetType() != TokenType::IDENTIFIER)
			DieAndPrint("only identifier can be added to the table.");
		_func_map[tk.GetValueString()] = pos;
	}

	int32_t Analyser::findFunc(const std::string name)
	{
		return _func_map[name];
	}


}