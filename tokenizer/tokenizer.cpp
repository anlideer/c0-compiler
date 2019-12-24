#include "tokenizer/tokenizer.h"
#include <cctype>
#include <sstream>

namespace miniplc0 {

	std::pair<std::optional<Token>, std::optional<CompilationError>> Tokenizer::NextToken() {
		if (!_initialized)
			readAll();
		if (_rdr.bad())
			return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrorCode::ErrStreamError));
		if (isEOF())
			return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrorCode::ErrEOF));
		auto p = nextToken();
		if (p.second.has_value())
			return std::make_pair(p.first, p.second);
		auto err = checkToken(p.first.value());
		if (err.has_value())
			return std::make_pair(p.first, err.value());
		return std::make_pair(p.first, std::optional<CompilationError>());
	}

	std::pair<std::vector<Token>, std::optional<CompilationError>> Tokenizer::AllTokens() {
		std::vector<Token> result;
		while (true) {
			auto p = NextToken();
			if (p.second.has_value()) {
				if (p.second.value().GetCode() == ErrorCode::ErrEOF)
					return std::make_pair(result, std::optional<CompilationError>());
				else
					return std::make_pair(std::vector<Token>(), p.second);
			}
			result.emplace_back(p.first.value());
		}
	}

	// 注意：这里的返回值中 Token 和 CompilationError 只能返回一个，不能同时返回。
	std::pair<std::optional<Token>, std::optional<CompilationError>> Tokenizer::nextToken() {
		// 用于存储已经读到的组成当前token字符
		std::stringstream ss;
		// 分析token的结果，作为此函数的返回值
		std::pair<std::optional<Token>, std::optional<CompilationError>> result;
		// <行号，列号>，表示当前token的第一个字符在源代码中的位置
		std::pair<int64_t, int64_t> pos;
		// 记录当前自动机的状态，进入此函数时是初始状态
		DFAState current_state = DFAState::INITIAL_STATE;
		// 这是一个死循环，除非主动跳出
		// 每一次执行while内的代码，都可能导致状态的变更
		while (true) {
			// 读一个字符，请注意auto推导得出的类型是std::optional<char>
			// 这里其实有两种写法
			// 1. 每次循环前立即读入一个 char
			// 2. 只有在可能会转移的状态读入一个 char
			// 因为我们实现了 unread，为了省事我们选择第一种
			auto current_char = nextChar();
			// 针对当前的状态进行不同的操作
			switch (current_state) {

				// 初始状态
				// 这个 case 我们给出了核心逻辑，但是后面的 case 不用照搬。
			case INITIAL_STATE: {
				// 已经读到了文件尾
				if (!current_char.has_value())
					// 返回一个空的token，和编译错误ErrEOF：遇到了文件尾
					return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrEOF));

				// 获取读到的字符的值，注意auto推导出的类型是char
				auto ch = current_char.value();
				// 标记是否读到了不合法的字符，初始化为否
				auto invalid = false;

				// 使用了自己封装的判断字符类型的函数，定义于 tokenizer/utils.hpp
				// see https://en.cppreference.com/w/cpp/string/byte/isblank
				if (miniplc0::isspace(ch)) // 读到的字符是空白字符（空格、换行、制表符等）
					current_state = DFAState::INITIAL_STATE; // 保留当前状态为初始状态，此处直接break也是可以的
				else if (!miniplc0::isprint(ch)) // control codes and backspace
					invalid = true;
				// I directly judge if it is 10-based or 16-based integer right here... don't know if it's ok
				else if (ch == '0')
				{
					ss << ch;
					current_char = nextChar();
					if (!current_char.has_value())
						return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrEOF));
					ch = current_char.value();
					if (ch == 'x')
					{
						current_state = DFAState::UNSIGNED_INTEGER16_STATE;	// hex
					}
					// only 0
					else if (!miniplc0::isdigit(ch))
					{
						current_state = DFAState::UNSIGNED_INTEGER10_STATE;	// 10based
					}
					else 
					{
						unreadLast();
						return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrInvalidInteger));	
					}
				}
				else if (miniplc0::isdigit(ch)) // 读到的字符是数字
					current_state = DFAState::UNSIGNED_INTEGER10_STATE; // 10based
				else if (miniplc0::isalpha(ch)) // 读到的字符是英文字母
					current_state = DFAState::IDENTIFIER_STATE; // 切换到标识符的状态
				else {
					switch (ch) {
					case '=': // 如果读到的字符是`=`，则切换到等于号的状态
						current_state = DFAState::EQUAL_SIGN_STATE;
						break;
					case '<':
						current_state = DFAState::SMALLER_STATE;
						break;
					case '>':
						current_state = DFAState::BIGGER_STATE;
						break;
					case '!':
						current_state = DFAState::EXCLAMATION_STATE;
						break;
					case '-':
						current_state = DFAState::MINUS_SIGN_STATE;
						break;
					case '+':
						current_state = DFAState::PLUS_SIGN_STATE;
						break;
					case '*':
						current_state = DFAState::MULTIPLICATION_SIGN_STATE;
						break;
					case '/':
						current_state = DFAState::DIVISION_SIGN_STATE;
						break;
					case ';':
						current_state = DFAState::SEMICOLON_STATE;
						break;
					case '(':
						current_state = DFAState::LEFTBRACKET_STATE;
						break;
					case ')':
						current_state = DFAState::RIGHTBRACKET_STATE;
						break;
					case '{':
						current_state = DFAState::LEFTBRACE_STATE;
						break;
					case '}':
						current_state = DFAState::RIGHTBRACE_STATE;
						break;
					case ',':
						current_state = DFAState::COMMA_STATE;
						break;

					// 不接受的字符导致的不合法的状态
					default:
						invalid = true;
						break;
					}
				}
				// 如果读到的字符导致了状态的转移，说明它是一个token的第一个字符
				if (current_state != DFAState::INITIAL_STATE)
					pos = previousPos(); // 记录该字符的的位置为token的开始位置
				// 读到了不合法的字符
				if (invalid) {
					// 回退这个字符
					unreadLast();
					// 返回编译错误：非法的输入
					return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrInvalidInput));
				}
				// 如果读到的字符导致了状态的转移，说明它是一个token的第一个字符
				if (current_state != DFAState::INITIAL_STATE) // ignore white spaces
					ss << ch; // 存储读到的字符
				break;
			}

			// 当前状态是无符号整数(10 based)
			case UNSIGNED_INTEGER10_STATE: {

				// 如果当前已经读到了文件尾，则解析已经读到的字符串为整数
				//     解析成功则返回无符号整数类型的token，否则返回编译错误
				if (!current_char.has_value())
				{
					try
					{
						int32_t tmp = std::stoi(ss.str());
						return std::make_pair(std::make_optional<Token>(TokenType::UNSIGNED_INTEGER, tmp, pos, currentPos()), std::optional<CompilationError>());
					}
					catch(std::invalid_argument&)
					{
						return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrIntegerOverflow));					
					}
					catch(std::out_of_range&)
					{
						return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrIntegerOverflow));
					}
					catch(...)
					{
						return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrIntegerOverflow));
					}
				}
				// 如果读到的字符是数字，则存储读到的字符
				else if (miniplc0::isdigit(current_char.value()))
				{
					ss << current_char.value();
				}
				// 如果读到的是字母，then error
				else if (miniplc0::isalpha(current_char.value()))
				{
					return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrInvalidIdentifier));
				}
				// 如果读到的字符不是上述情况之一，则回退读到的字符，并解析已经读到的字符串为整数
				//     解析成功则返回无符号整数类型的token，否则返回编译错误
				else
				{
					unreadLast();
					try
					{
						int32_t tmp = std::stoi(ss.str());
						return std::make_pair(std::make_optional<Token>(TokenType::UNSIGNED_INTEGER, tmp, pos, currentPos()), std::optional<CompilationError>());
					}
					catch(std::invalid_argument&)
					{
						return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrIntegerOverflow));					
					}
					catch(std::out_of_range&)
					{
						return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrIntegerOverflow));
					}
					catch(...)
					{
						return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrIntegerOverflow));
					}
				}

				break;
				
			}
			// This is hex integer 
			case UNSIGNED_INTEGER16_STATE:{
				if (!current_char.has_value())
				{
					try
					{
						int32_t tmp = std::stoi(ss.str(), 0, 16);
						return std::make_pair(std::make_optional<Token>(TokenType::UNSIGNED_INTEGER, tmp, pos, currentPos()), std::optional<CompilationError>());
					}
					catch(std::invalid_argument&)
					{
						return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrIntegerOverflow));					
					}
					catch(std::out_of_range&)
					{
						return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrIntegerOverflow));
					}
					catch(...)
					{
						return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrIntegerOverflow));
					}
				}
				// 如果读到的字符是数字，则存储读到的字符
				else if (miniplc0::isdigit(current_char.value()))
				{
					ss << current_char.value();
				}
				// 如果读到的是字母，then error
				else if (miniplc0::isalpha(current_char.value()))
				{
					return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrInvalidIdentifier));
				}
				// 如果读到的字符不是上述情况之一，则回退读到的字符，并解析已经读到的字符串为整数
				//     解析成功则返回无符号整数类型的token，否则返回编译错误
				else
				{
					unreadLast();
					try
					{
						int32_t tmp = std::stoi(ss.str(), 0, 16);
						return std::make_pair(std::make_optional<Token>(TokenType::UNSIGNED_INTEGER, tmp, pos, currentPos()), std::optional<CompilationError>());
					}
					catch(std::invalid_argument&)
					{
						return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrIntegerOverflow));					
					}
					catch(std::out_of_range&)
					{
						return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrIntegerOverflow));
					}
					catch(...)
					{
						return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrIntegerOverflow));
					}
				}

				break;


			}
			// Identifier
			case IDENTIFIER_STATE: {
				// 如果当前已经读到了文件尾，则解析已经读到的字符串
				//     如果解析结果是关键字，那么返回对应关键字的token，否则返回标识符的token
				if (!current_char.has_value())
				{

					if (ss.str() == "const")
					{
						return std::make_pair(std::make_optional<Token>(TokenType::CONST, ss.str(), pos, currentPos()), std::optional<CompilationError>());
					}
					else if (ss.str() == "void")
					{
						return std::make_pair(std::make_optional<Token>(TokenType::VOID, ss.str(), pos, currentPos()), std::optional<CompilationError>());
					}
					else if (ss.str() == "int")
					{
						return std::make_pair(std::make_optional<Token>(TokenType::INT, ss.str(), pos, currentPos()), std::optional<CompilationError>());
					}
					else if (ss.str() == "if")
					{
						return std::make_pair(std::make_optional<Token>(TokenType::IF, ss.str(), pos, currentPos()), std::optional<CompilationError>());
					}
					else if (ss.str() == "else")
					{
						return std::make_pair(std::make_optional<Token>(TokenType::ELSE, ss.str(), pos, currentPos()), std::optional<CompilationError>());
					}
					else if (ss.str() == "while")
					{
						return std::make_pair(std::make_optional<Token>(TokenType::WHILE, ss.str(), pos, currentPos()), std::optional<CompilationError>());
					}
					else if (ss.str() == "scan")
					{
						return std::make_pair(std::make_optional<Token>(TokenType::SCAN, ss.str(), pos, currentPos()), std::optional<CompilationError>());
					}
					else if (ss.str() == "print")
					{
						return std::make_pair(std::make_optional<Token>(TokenType::PRINT, ss.str(), pos, currentPos()), std::optional<CompilationError>());
					}
					else if (ss.str() == "char" || ss.str() == "double" || ss.str() == "struct" || ss.str() == "switch" || ss.str() == "case" || ss.str() == "default"  || ss.str() == "for"
						|| ss.str() == "do" || ss.str() == "return" || ss.str() == "break" || ss.str() == "continue")
					{
						return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrInvalidIdentifier));
					}
					else
					{
						return std::make_pair(std::make_optional<Token>(TokenType::IDENTIFIER, ss.str(), pos, currentPos()), std::optional<CompilationError>());
					}

				}
				// 如果读到的是字符或字母，则存储读到的字符
				else if (miniplc0::isalpha(current_char.value()) || miniplc0::isdigit(current_char.value()))
				{
					ss << current_char.value();
				}
				// 如果读到的字符不是上述情况之一，则回退读到的字符，并解析已经读到的字符串
				//     如果解析结果是关键字，那么返回对应关键字的token，否则返回标识符的token
				else
				{
					unreadLast();
					// the same as above
					if (ss.str() == "const")
					{
						return std::make_pair(std::make_optional<Token>(TokenType::CONST, ss.str(), pos, currentPos()), std::optional<CompilationError>());
					}
					else if (ss.str() == "void")
					{
						return std::make_pair(std::make_optional<Token>(TokenType::VOID, ss.str(), pos, currentPos()), std::optional<CompilationError>());
					}
					else if (ss.str() == "int")
					{
						return std::make_pair(std::make_optional<Token>(TokenType::INT, ss.str(), pos, currentPos()), std::optional<CompilationError>());
					}
					else if (ss.str() == "if")
					{
						return std::make_pair(std::make_optional<Token>(TokenType::IF, ss.str(), pos, currentPos()), std::optional<CompilationError>());
					}
					else if (ss.str() == "else")
					{
						return std::make_pair(std::make_optional<Token>(TokenType::ELSE, ss.str(), pos, currentPos()), std::optional<CompilationError>());
					}
					else if (ss.str() == "while")
					{
						return std::make_pair(std::make_optional<Token>(TokenType::WHILE, ss.str(), pos, currentPos()), std::optional<CompilationError>());
					}
					else if (ss.str() == "scan")
					{
						return std::make_pair(std::make_optional<Token>(TokenType::SCAN, ss.str(), pos, currentPos()), std::optional<CompilationError>());
					}
					else if (ss.str() == "print")
					{
						return std::make_pair(std::make_optional<Token>(TokenType::PRINT, ss.str(), pos, currentPos()), std::optional<CompilationError>());
					}
					else if (ss.str() == "return")
					{
						return std::make_pair(std::make_optional<Token>(TokenType::RETURN, ss.str(), pos, currentPos()), std::optional<CompilationError>());
					}
					else if (ss.str() == "char" || ss.str() == "double" || ss.str() == "struct" || ss.str() == "switch" || ss.str() == "case" || ss.str() == "default"  || ss.str() == "for"
						|| ss.str() == "do" || ss.str() == "break" || ss.str() == "continue")
					{
						return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrInvalidIdentifier));
					}
					else
					{
						return std::make_pair(std::make_optional<Token>(TokenType::IDENTIFIER, ss.str(), pos, currentPos()), std::optional<CompilationError>());
					}
				}
				break;
			}

			// 如果当前状态是加号
			case PLUS_SIGN_STATE: {
				// 请思考这里为什么要回退，在其他地方会不会需要
				// Actually, I think every Vt which has only one letter needs to unread, for they won't actually handle current_char. 
				// What they handle here is actually the last current_char (in this case, '+'.), which made the state changed.
				// If we don't do the process of unread, we'll skip the current_char in this loop.
				// (I'm lazy so I can't type in Chinese :)
				// So, +, -, =, (, ), ;, *, / They all need to unread.
				unreadLast(); // Yes, we unread last char even if it's an EOF.	// If it's an EOF, we can still leave it for the next loop.
				return std::make_pair(std::make_optional<Token>(TokenType::PLUS_SIGN, '+', pos, currentPos()), std::optional<CompilationError>());
			}
			// 当前状态为减号的状态
			case MINUS_SIGN_STATE: {
				// 请填空：回退，并返回减号token
				unreadLast();
				return std::make_pair(std::make_optional<Token>(TokenType::MINUS_SIGN, '-', pos, currentPos()), std::optional<CompilationError>());
			}

			// 对于其他的合法状态，进行合适的操作
			// 比如进行解析、返回token、返回编译错误

			// *
			case MULTIPLICATION_SIGN_STATE: {
				unreadLast();
				return std::make_pair(std::make_optional<Token>(TokenType::MULTIPLICATION_SIGN, '*', pos, currentPos()), std::optional<CompilationError>());
			}
			// /
			case DIVISION_SIGN_STATE: {
				unreadLast();
				return std::make_pair(std::make_optional<Token>(TokenType::DIVISION_SIGN, '/', pos, currentPos()), std::optional<CompilationError>());
			}
			// =
			case EQUAL_SIGN_STATE: {
				// =
				if (!current_char.has_value() || current_char.value() != '=')
				{
					unreadLast();
					return std::make_pair(std::make_optional<Token>(TokenType::ASSIGN_SIGN, '=', pos, currentPos()), std::optional<CompilationError>());
				}
				// ==
				else
				{
					return std::make_pair(std::make_optional<Token>(TokenType::EQUAL_SIGN, "==", pos, currentPos()), std::optional<CompilationError>());
				}

			}
			// ;
			case SEMICOLON_STATE: {
				unreadLast();
				return std::make_pair(std::make_optional<Token>(TokenType::SEMICOLON, ';', pos, currentPos()), std::optional<CompilationError>());
			}
			// (
			case LEFTBRACKET_STATE: {
				unreadLast();
				return std::make_pair(std::make_optional<Token>(TokenType::LEFT_BRACKET, '(', pos, currentPos()), std::optional<CompilationError>());
			}
			// )
			case RIGHTBRACKET_STATE: {
				unreadLast();
				return std::make_pair(std::make_optional<Token>(TokenType::RIGHT_BRACKET, ')', pos, currentPos()), std::optional<CompilationError>());
			}
			// {
			case LEFTBRACE_STATE: {
				unreadLast();
				return std::make_pair(std::make_optional<Token>(TokenType::LEFT_BRACE, '{', pos, currentPos()), std::optional<CompilationError>());
			}
			// }
			case RIGHTBRACE_STATE:{
				unreadLast();
				return std::make_pair(std::make_optional<Token>(TokenType::RIGHT_BRACE, '}', pos, currentPos()), std::optional<CompilationError>());
			}
			// >
			case BIGGER_STATE:{
				// >
				if (!current_char.has_value() || current_char.value() != '=')
				{
					unreadLast();
					return std::make_pair(std::make_optional<Token>(TokenType::BIGGER_SIGN, '>', pos, currentPos()), std::optional<CompilationError>());
				}
				// >=
				else
				{
					return std::make_pair(std::make_optional<Token>(TokenType::NOTSMALLER_SIGN, ">=", pos, currentPos()), std::optional<CompilationError>());
				}
				break;
			}
			// <
			case SMALLER_STATE:{
				// <
				if (!current_char.has_value() || current_char.value() != '=')
				{
					unreadLast();
					return std::make_pair(std::make_optional<Token>(TokenType::SMALLER_SIGN, '<', pos, currentPos()), std::optional<CompilationError>());
				}
				// <=
				else
				{
					return std::make_pair(std::make_optional<Token>(TokenType::NOTBIGGER_SIGN, "<=", pos, currentPos()), std::optional<CompilationError>());
				}
				break;
			}
			// !
			case EXCLAMATION_STATE:{
				// the char has to be '=', or it's an error
				if (!current_char.has_value() || current_char.value() != '=')
				{
					unreadLast();
					return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrInvalidNotEqual));					

				}
				// !=
				else
				{
					return std::make_pair(std::make_optional<Token>(TokenType::NOTEQUAL_SIGN, "!=", pos, currentPos()), std::optional<CompilationError>());
				}
				break;
			}
			// ,
			case COMMA_STATE:{
				unreadLast();
				return std::make_pair(std::make_optional<Token>(TokenType::COMMA, ';', pos, currentPos()), std::optional<CompilationError>());
			}

			// 预料之外的状态，如果执行到了这里，说明程序异常
			default:
				DieAndPrint("unhandled state.");
				break;
			}
		}
		// 预料之外的状态，如果执行到了这里，说明程序异常
		return std::make_pair(std::optional<Token>(), std::optional<CompilationError>());
	}

	std::optional<CompilationError> Tokenizer::checkToken(const Token& t) {
		switch (t.GetType()) {
			case IDENTIFIER: {
				auto val = t.GetValueString();
				if (miniplc0::isdigit(val[0]))
					return std::make_optional<CompilationError>(t.GetStartPos().first, t.GetStartPos().second, ErrorCode::ErrInvalidIdentifier);
				break;
			}
		default:
			break;
		}
		return {};
	}

	void Tokenizer::readAll() {
		if (_initialized)
			return;
		for (std::string tp; std::getline(_rdr, tp);)
			_lines_buffer.emplace_back(std::move(tp + "\n"));
		_initialized = true;
		_ptr = std::make_pair<int64_t, int64_t>(0, 0);
		return;
	}

	// Note: We allow this function to return a postion which is out of bound according to the design like std::vector::end().
	std::pair<uint64_t, uint64_t> Tokenizer::nextPos() {
		if (_ptr.first >= _lines_buffer.size())
			DieAndPrint("advance after EOF");
		if (_ptr.second == _lines_buffer[_ptr.first].size() - 1)
			return std::make_pair(_ptr.first + 1, 0);
		else
			return std::make_pair(_ptr.first, _ptr.second + 1);
	}

	std::pair<uint64_t, uint64_t> Tokenizer::currentPos() {
		return _ptr;
	}

	std::pair<uint64_t, uint64_t> Tokenizer::previousPos() {
		if (_ptr.first == 0 && _ptr.second == 0)
			DieAndPrint("previous position from beginning");
		if (_ptr.second == 0)
			return std::make_pair(_ptr.first - 1, _lines_buffer[_ptr.first - 1].size() - 1);
		else
			return std::make_pair(_ptr.first, _ptr.second - 1);
	}

	std::optional<char> Tokenizer::nextChar() {
		if (isEOF())
			return {}; // EOF
		auto result = _lines_buffer[_ptr.first][_ptr.second];
		_ptr = nextPos();
		return result;
	}

	bool Tokenizer::isEOF() {
		return _ptr.first >= _lines_buffer.size();
	}

	// Note: Is it evil to unread a buffer?
	void Tokenizer::unreadLast() {
		_ptr = previousPos();
	}
}