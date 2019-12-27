#include "fmt/core.h"
#include "tokenizer/tokenizer.h"
#include "analyser/analyser.h"

namespace fmt {
	template<>
	struct formatter<miniplc0::ErrorCode> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const miniplc0::ErrorCode &p, FormatContext &ctx) {
			std::string name;
			switch (p) {
			case miniplc0::ErrNoError:
				name = "No error.";
				break;
			case miniplc0::ErrStreamError:
				name = "Stream error.";
				break;
			case miniplc0::ErrEOF:
				name = "EOF";
				break;
			case miniplc0::ErrInvalidInput:
				name = "The input is invalid.";
				break;
			case miniplc0::ErrInvalidIdentifier:
				name = "Identifier is invalid";
				break;
			case miniplc0::ErrIntegerOverflow:
				name = "The integer is too big(int64_t).";
				break;
			case miniplc0::ErrNeedIdentifier:
				name = "Need an identifier here.";
				break;
			case miniplc0::ErrConstantNeedValue:
				name = "The constant need a value to initialize.";
				break;
			case miniplc0::ErrNoSemicolon:
				name = "Zai? Wei shen me bu xie fen hao.";
				break;
			case miniplc0::ErrInvalidVariableDeclaration:
				name = "The declaration is invalid.";
				break;
			case miniplc0::ErrIncompleteExpression:
				name = "The expression is incomplete.";
				break;
			case miniplc0::ErrNotDeclared:
				name = "The variable or constant must be declared before being used.";
				break;
			case miniplc0::ErrAssignToConstant:
				name = "Trying to assign value to a constant.";
				break;
			case miniplc0::ErrDuplicateDeclaration:
				name = "The variable or constant has been declared.";
				break;
			case miniplc0::ErrNotInitialized:
				name = "The variable has not been initialized.";
				break;
			case miniplc0::ErrInvalidAssignment:
				name = "The assignment statement is invalid.";
				break;
			case miniplc0::ErrInvalidPrint:
				name = "The output statement is invalid.";
				break;
			case miniplc0::ErrNoRightBracket:
				name = "No matching right bracket.";
				break;
			case miniplc0::ErrNoRightBrace:
				name = "No matching right brace.";
				break;
			case miniplc0::ErrInvalidInteger:
				name = "This should not be an integer.";
				break;
			case miniplc0::ErrInvalidNotEqual:
				name = "Do you mean '!='(not equal)? You should complete it.";
				break;
			case miniplc0::ErrIncompleteFunctionCall:
				name = "Incomplete function call";
				break;
			case miniplc0::ErrInvalidFunctionDifinition:
				name = "Invalid function difinition";
				break;
			case miniplc0::ErrInvalidConditionStatement:
				name = "Invalid condition statement";
				break;
			case miniplc0::ErrInvalidStatement:
				name = "Invalid statement";
				break;
			case miniplc0::ErrInvalidCondition:
				name = "Invalid condition";
				break;
			case miniplc0::ErrVoidCantCalculate:
				name = "Function returns void cannot be calculated";
				break;
			case miniplc0::ErrIfelse:
				name = "Invalid If-else (shouldn't have appeared)";
				break;
			case miniplc0::ErrLoop:
				name = "Invalid loop (shouldn't have appeared)";
				break;
			case miniplc0::ErrRedeclared:
				name = "This variable has been declared";
				break;
			case miniplc0::ErrInvalidNum:
				name = "Invalid hex number";
				break;
			case miniplc0::ErrConstantChange:
				name = "Constant can't be changed";
				break;
			case miniplc0::ErrInvalidReturn:
				name = "Invalid return";
				break;
			case miniplc0::ErrNoReturn:
				name = "Except return statement";
				break;
			case miniplc0::ErrFunctionRedefined:
				name = "This function has already been defined, you can't define it twice";
				break;
			case miniplc0::ErrNoFunction:
				name = "This function does not exist, please define it first";
				break;
			case miniplc0::ErrVarFun:
				name = "This identifier as been declared as a function/variable";
				break;
			case miniplc0::ErrLocalFunConflict:
				name = "Since you have a variable sharing the same name with this function, you cannot call it";
				break;

			default:
				name = "Error occurs.";
			}
			return format_to(ctx.out(), name);
		}
	};

	template<>
	struct formatter<miniplc0::CompilationError> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const miniplc0::CompilationError &p, FormatContext &ctx) {
			return format_to(ctx.out(), "Line: {} Column: {} Error: {}", p.GetPos().first, p.GetPos().second, p.GetCode());
		}
	};
}

namespace fmt {
	template<>
	struct formatter<miniplc0::Token> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const miniplc0::Token &p, FormatContext &ctx) {
			return format_to(ctx.out(),
				"Line: {} Column: {} Type: {} Value: {}",
				p.GetStartPos().first, p.GetStartPos().second, p.GetType(), p.GetValueString());
		}
	};

	template<>
	struct formatter<miniplc0::TokenType> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const miniplc0::TokenType &p, FormatContext &ctx) {
			std::string name;
			switch (p) {
			case miniplc0::NULL_TOKEN:
				name = "NullToken";
				break;
			case miniplc0::UNSIGNED_INTEGER:
				name = "UnsignedInteger";
				break;
			case miniplc0::IDENTIFIER:
				name = "Identifier";
				break;
			case miniplc0::VAR:
				name = "Var";
				break;
			case miniplc0::CONST:
				name = "Const";
				break;
			case miniplc0::PRINT:
				name = "Print";
				break;
			case miniplc0::PLUS_SIGN:
				name = "PlusSign";
				break;
			case miniplc0::MINUS_SIGN:
				name = "MinusSign";
				break;
			case miniplc0::MULTIPLICATION_SIGN:
				name = "MultiplicationSign";
				break;
			case miniplc0::DIVISION_SIGN:
				name = "DivisionSign";
				break;
			case miniplc0::EQUAL_SIGN:
				name = "EqualSign";
				break;
			case miniplc0::SEMICOLON:
				name = "Semicolon";
				break;
			case miniplc0::LEFT_BRACKET:
				name = "LeftBracket";
				break;
			case miniplc0::RIGHT_BRACKET:
				name = "RightBracket";
				break;
			case miniplc0::VOID:
				name = "Void";
				break;
			case miniplc0::INT:
				name = "Int";
				break;
			case miniplc0::SCAN:
				name = "Scan";
				break;
			case miniplc0::LEFT_BRACE:
				name = "LeftBrace";
				break;
			case miniplc0::RIGHT_BRACE:
				name = "RightBrace";
				break;
			case miniplc0::IF:
				name = "If";
				break;
			case miniplc0::ELSE:
				name = "Else";
				break;
			case miniplc0::WHILE:
				name = "While";
				break;
			case miniplc0::ASSIGN_SIGN:
				name = "AssignSign";
				break;
			case miniplc0::BIGGER_SIGN:
				name = "BiggerSign";
				break;
			case miniplc0::SMALLER_SIGN:
				name = "SmallerSign";
				break;
			case miniplc0::NOTBIGGER_SIGN:
				name = "NotBiggerSign";
				break;
			case miniplc0::NOTSMALLER_SIGN:
				name = "NotSmallerSign";
				break;
			case miniplc0::NOTEQUAL_SIGN:
				name = "NotEqualSign";
				break;
			case miniplc0::COMMA:
				name = "Comma";
				break;
			case miniplc0::RETURN:
				name = "Return";
				break;
			default:
				name = "Unknown sign";
			}
			return format_to(ctx.out(), name);
		}
	};
}

namespace fmt {
	template<>
	struct formatter<miniplc0::Operation> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const miniplc0::Operation &p, FormatContext &ctx) {
			std::string name;
			switch (p) {
			case miniplc0::ILL:
				name = "ILL";
				break;
			case miniplc0::CONSTANTS:
				name = ".constants:";
				break;
			// this is a special one, I only use it to store the name of function
			case miniplc0::CONSTANT:
				name = "S";
				break;
			case miniplc0::START:
				name = ".start:";
				break;
			case miniplc0::FUNCTIONS:
				name = ".functions:";
				break;
			case miniplc0::FUNCN:
				name = ".F";
				break;
			case miniplc0::FUNCINFO:
				name = "";
				break;
			case miniplc0::LOADA:
				name = "loada";
				break;
			case miniplc0::LOADC:
				name = "loadc";
				break;
			case miniplc0::IPUSH:
				name = "ipush";
				break;
			case miniplc0::ILOAD:
				name = "iload";
				break;
			case miniplc0::ISTORE:
				name = "istore";
				break;
			case miniplc0::NEW:
				name = "new";
				break;
			case miniplc0::SNEW:
				name = "snew";
				break;
			case miniplc0::POP:
				name = "pop";
				break;
			case miniplc0::DUP:
				name = "dup";
				break;
			case miniplc0::IADD:
				name = "iadd";
				break;
			case miniplc0::ISUB:
				name = "isub";
				break;
			case miniplc0::IMUL:
				name = "imul";
				break;
			case miniplc0::IDIV:
				name = "idiv";
				break;
			case miniplc0::INEG:
				name = "ineg";
				break;
			case miniplc0::ICMP:
				name = "icmp";
				break;
			case miniplc0::JE:
				name = "je";
				break;
			case miniplc0::JNE:
				name = "jne";
				break;
			case miniplc0::JL:
				name = "jl";
				break;
			case miniplc0::JGE:
				name = "jge";
				break;
			case miniplc0::JG:
				name = "jg";
				break;
			case miniplc0::JLE:
				name = "jle";
				break;
			case miniplc0::JMP:
				name = "jmp";
				break;
			case miniplc0::CALL:
				name = "call";
				break;
			case miniplc0::RET:
				name = "ret";
				break;
			case miniplc0::IRET:
				name = "iret";
				break;
			case miniplc0::IPRINT:
				name = "iprint";
				break;
			case miniplc0::CPRINT:
				name = "cprint";
				break;
			case miniplc0::ISCAN:
				name = "iscan";
				break;
			case miniplc0::BIPUSH:
				name = "bipush";
				break;
			case miniplc0::PRINTL:
				name = "printl";
				break;

			
			}
			return format_to(ctx.out(), name);
		}
	};
	template<>
	struct formatter<miniplc0::Instruction> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const miniplc0::Instruction &p, FormatContext &ctx) {
			std::string name;
			switch (p.GetOperation())
			{
			case miniplc0::ILL:
				return format_to(ctx.out(), "{}", p.GetOperation());

			case miniplc0::CONSTANTS:
				return format_to(ctx.out(), "{}", p.GetOperation());
			// CONST is a special operation, please pay more attention to it
			case miniplc0::CONSTANT:
				return format_to(ctx.out(), "{} {} \"{}\"", p.GetIndex(), p.GetOperation(), p.GetStr());
			case miniplc0::START:
				return format_to(ctx.out(), "{}", p.GetOperation());
			case miniplc0::FUNCTIONS:
				return format_to(ctx.out(), "{}", p.GetOperation());
			case miniplc0::FUNCN:
				return format_to(ctx.out(), "{}{}:", p.GetOperation(), p.GetX());
			case miniplc0::FUNCINFO:
				return format_to(ctx.out(), "{} {} {} {}", p.GetIndex(), p.GetNameIndex(), p.GetParamsSize(), p.GetLevel());

			case miniplc0::LOADA:
				return format_to(ctx.out(), "{} {} {}, {}", p.GetIndex(), p.GetOperation(), p.GetLevel(), p.GetX());
			case miniplc0::LOADC:
				return format_to(ctx.out(), "{} {} {}", p.GetIndex(), p.GetOperation(), p.GetX());
			case miniplc0::IPUSH:
				return format_to(ctx.out(), "{} {} {}", p.GetIndex(), p.GetOperation(), p.GetX());
			case miniplc0::BIPUSH:
				return format_to(ctx.out(), "{} {} {}", p.GetIndex(), p.GetOperation(), p.GetX());
			case miniplc0::ILOAD:
				return format_to(ctx.out(), "{} {}", p.GetIndex(), p.GetOperation());
			case miniplc0::ISTORE:
				return format_to(ctx.out(), "{} {}", p.GetIndex(), p.GetOperation());
			case miniplc0::NEW:
				return format_to(ctx.out(), "{} {}", p.GetIndex(), p.GetOperation());
			case miniplc0::SNEW:
				return format_to(ctx.out(), "{} {} {}", p.GetIndex(), p.GetOperation(), p.GetX());
			case miniplc0::POP:
				return format_to(ctx.out(), "{} {}", p.GetIndex(), p.GetOperation());
			case miniplc0::DUP:
				return format_to(ctx.out(), "{} {}", p.GetIndex(), p.GetOperation());

			case miniplc0::IADD:
			case miniplc0::ISUB:
			case miniplc0::IMUL:
			case miniplc0::IDIV:
			case miniplc0::INEG:
			case miniplc0::ICMP:
				return format_to(ctx.out(), "{} {}", p.GetIndex(), p.GetOperation());

			case miniplc0::JE:
			case miniplc0::JNE:
			case miniplc0::JL:
			case miniplc0::JGE:
			case miniplc0::JG:
			case miniplc0::JLE:
			case miniplc0::JMP:
			case miniplc0::CALL:
				return format_to(ctx.out(), "{} {} {}", p.GetIndex(), p.GetOperation(), p.GetX());
			case miniplc0::RET:
			case miniplc0::IRET:
				return format_to(ctx.out(), "{} {}", p.GetIndex(), p.GetOperation());

			case miniplc0::IPRINT:
			case miniplc0::CPRINT:
			case miniplc0::ISCAN:
			case miniplc0::PRINTL:
				return format_to(ctx.out(), "{} {}", p.GetIndex(), p.GetOperation());


			/*
			case miniplc0::ADD:
			case miniplc0::SUB:
			case miniplc0::MUL:
			case miniplc0::DIV:
			case miniplc0::WRT:
				return format_to(ctx.out(), "{}", p.GetOperation());
			case miniplc0::LIT:
			case miniplc0::LOD:
			case miniplc0::STO:
				return format_to(ctx.out(), "{} {}", p.GetOperation(), p.GetX());
			*/
			}
			return format_to(ctx.out(), "ILL");
		}
	};
}