#include "catch2/catch.hpp"
#include "tokenizer/tokenizer.h"
#include "tokenizer/token.h"
#include "fmt/core.h"

#include <sstream>
#include <vector>

// 下面是示例如何书写测试用例
// I still can't get it......
TEST_CASE("Test hello world.", "basic") {
	/*
	std::string input = 
		"begin\n"
		"	var a = 1;\n"
		"	const b = 1\n"
		"	print(a+b);\n"
		"end\n";
	std::stringstream ss;
	ss.str(input);
	miniplc0::Tokenizer tkz(ss);
	std::vector<miniplc0::Token> output = {
		std::make_optional<Token>(TokenType::BEGIN, "begin", 0, 0),
		std::make_optional<Token>(TokenType::VAR, "var", 1, 1),
		std::make_optional<Token>(TokenType::Identifier, "a", 1, 5),
		std::make_optional<Token>(TokenType::EqualSign, '=', 1, 7),
		std::make_optional<Token>(TokenType::UnsignedInteger, 1, 1, 9),
		std::make_optional<Token>(TokenType::Semicolon, ';', 1, 10),
		std::make_optional<Token>(TokenType::PRINT, "print", 2, 1),
		std::make_optional<Token>(TokenType::LeftBracket, '(', 2, 6),
		std::make_optional<Token>(TokenType::Identifier, "a", 2, 7),
		std::make_optional<Token>(TokenType::RightBracket, ')', 2, 8),
		std::make_optional<Token>(TokenType::Semicolon, ';', 2, 9),
		std::make_optional<Token>(TokenType::END, "end", 3, 0)
	};
	auto result = tkz.AllTokens();
	if (result.second.has_value()) {
		FAIL();
	}
	REQUIRE( (result.first == output) );
	*/
	
}

/*
// try 1 try
TEST_CASE("Test redefine.", "basic")
{
	std::string input = 
	"begin\n"
	"	var a = 1;\n"
	"	var b = 2;\n"
	"	b = 3;\n";
	"	var a = 2;\n"
	"end\n";
	std::stringstream ss;
	ss.str(input);
	miniplc0::Tokenizer tkz(ss);
	std::vector<miniplc0::Token> output = {

	};
	auto result = tkz.AllTokens();
	if (result.second.has_value()) {
		FAIL();
	}
	REQUIRE( (result.first == output) );
}
*/