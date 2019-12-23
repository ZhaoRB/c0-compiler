#include "argparse.hpp"
#include "fmt/core.h"

#include "tokenizer/tokenizer.h"
#include "analyser/analyser.h"
#include "fmts.hpp"

#include <iostream>
#include <fstream>

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
	// 测试语义分析
	std::cout << "OK!" << std::endl;
}

void translateToAssemblingFile(std::istream& input, std::ostream& output) {
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
}

void translateToBinaryFile(std::istream& input, std::ostream& output) {
    std::cout << "waiting to be implemented" << std::endl;
}

int main(int argc, char** argv) {
	argparse::ArgumentParser program("cc0");
    program.add_argument("input")
            .help("speicify the file to be compiled.");
    program.add_argument("-s")
            .default_value(false)
            .implicit_value(true)
            .help("Translate the input c0 source code into an assembly file.");
    program.add_argument("-c")
            .default_value(false)
            .implicit_value(true)
            .help("Translate the input c0 source code into a binary object file.");
    program.add_argument("-o", "--output")
            .required()
            .default_value(std::string("-"))
            .help("specify the output file.");

    // 用于测试的接口
    program.add_argument("-t")
            .default_value(false)
            .implicit_value(true)
            .help("Tokenizer.");
    program.add_argument("-l")
            .default_value(false)
            .implicit_value(true)
            .help("Translate the input c0 source code into an assembly file.");

	try {
		program.parse_args(argc, argv);
	}
	catch (const std::runtime_error& err) {
		// fmt::print(stderr, "{}\n\n", err.what());
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

	// 改成输出到 output.o0
	else
		output = &std::cout;

//	if (program["-s"] == true && program["-c"] == true) {
//		fmt::print(stderr, "You can only translate source code into an assembly file or a binary object file at one time.");
//		exit(2);
//	}

	// 转换程序入口
//	if (program["-s"] == true) {
//	    // 生成汇编代码
//		translateToAssemblingFile(*input,*output);
//	}
//	else if (program["-c"] == true) {
//	    // 生成二进制文件
//		translateToBinaryFile(*input, *output);
//	}
	/**
	 * 用于测试的接口
	 */
    if (program["-t"] == true) {
        // 测试词法分析
        Tokenize(*input,*output);
    }
    else if (program["-l"] == true) {
        // 测试语义分析
        Analyse(*input,*output);
    }


	else {
		fmt::print(stderr, "You must choose translate to an assembly file or a binary object file.");
		exit(2);
	}
	return 0;
}