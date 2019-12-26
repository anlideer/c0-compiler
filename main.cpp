#include "argparse.hpp"
#include "fmt/core.h"

#include "tokenizer/tokenizer.h"
#include "analyser/analyser.h"
#include "fmts.hpp"


  
#include "./vm.h"
#include "./file.h"
#include "./exception.h"
#include "./util/print.hpp"


#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <exception>

std::vector<miniplc0::Token> _tokenize(std::istream& input) {
	miniplc0::Tokenizer tkz(input);
	auto p = tkz.AllTokens();
	if (p.second.has_value()) {
		fmt::print(stderr, "Tokenization error: {}\n", p.second.value());
		exit(2);
	}
	return p.first;
}

void Tokenize(std::istream& input, std::ostream& output) {
	auto v = _tokenize(input);
	for (auto& it : v)
		output << fmt::format("{}\n", it);
	return;
}

void Analyse(std::istream& input, std::ostream& output){
	auto tks = _tokenize(input);
	miniplc0::Analyser analyser(tks);
	auto p = analyser.Analyse();
	if (p.second.has_value()) {
		fmt::print(stderr, "Syntactic analysis error: {}\n", p.second.value());
		exit(2);
	}
	auto v = p.first;
	for (auto& it : v)
		output << fmt::format("{}\n", it);
	return;
}

void AnalyseToBinary(std::istream& input, std::ostream& output) {
	std::ofstream tmpStream;
	tmpStream.open("tmp.s0", std::ios::out);
	if (!tmpStream) {
		fmt::print(stderr, "Fail to open tmp.s0 for writing.\n");
			exit(2);
	}	
	std::ostream* tmpStream2 = &tmpStream;
	Analyse(input, output);


	std::istream* inputtmp2;
	std::ifstream inputtmp;
	inputtmp.open("tmp.s0", std::ios::in);
	if (!inputtmp)
	{
		fmt::print(stderr, "Fail to open tmp.s0 for reading.\n");
			exit(2);		
	}
	inputtmp2 = &inputtmp;
	std::ofstream* outftmp;
	outftmp = *output;

    try {
        File f = File::parse_file_text(&inputtmp2);
        // f.output_text(std::cout);
        f.output_binary(outftmp);
    }
    catch (const std::exception& e) {
        println(std::cerr, e.what());
    }
}

int main(int argc, char** argv) {
	argparse::ArgumentParser program("miniplc0");
	program.add_argument("input")
		.help("speicify the file to be compiled.");
	program.add_argument("-t")
		.default_value(false)
		.implicit_value(true)
		.help("perform tokenization for the input file.");
	program.add_argument("-s")
		.default_value(false)
		.implicit_value(true)
		.help("perform syntactic analysis(in the form of text) for the input file to the output file.");
	program.add_argument("-h")
		.defalut_value(false)
		.implicit_value(true)
		.help("perform syntactc analysis and output is .o0");
	program.add_argument("-o", "--output")
		.required()
		.default_value(std::string("-"))
		.help("specify the output file.");

	try {
		program.parse_args(argc, argv);
	}
	catch (const std::runtime_error& err) {
		fmt::print(stderr, "{}\n\n", err.what());
		program.print_help();
		exit(2);
	}

	auto input_file = program.get<std::string>("input");
	auto output_file = program.get<std::string>("--output");
	std::istream* input;
	std::ostream* output;
	std::ifstream inf;
	std::ofstream outf;
	if (input_file != "-") {
		inf.open(input_file, std::ios::in);
		if (!inf) {
			fmt::print(stderr, "Fail to open {} for reading.\n", input_file);
			exit(2);
		}
		input = &inf;
	}
	else
		input = &std::cin;
	if (output_file != "-") {
		outf.open(output_file, std::ios::out | std::ios::trunc);
		if (!outf) {
			fmt::print(stderr, "Fail to open {} for writing.\n", output_file);
			exit(2);
		}
		output = &outf;
	}
	else
		output = &std::cout;
	if (program["-t"] == true && program["-s"] == true) {
		fmt::print(stderr, "You can only perform tokenization or syntactic analysis at one time.");
		exit(2);
	}
	if (program["-t"] == true) {
		Tokenize(*input, *output);
	}
	else if (program["-s"] == true) {
		Analyse(*input, *output);
	}
	else if (program["-h"] == true)
	{
		AnalyseToBinary(*input, *output);
	}
	else {
		fmt::print(stderr, "You must choose tokenization or syntactic analysis.");
		exit(2);
	}
	return 0;
}