#include "argparse.hpp"
#include "fmt/core.h"

#include "tokenizer/tokenizer.h"
#include "analyser/analyser.h"
#include "instruction/instruction.h"
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
	if (p.first.second.has_value()) {
        fmt::print(stderr, "Syntactic analysis error: {}\n", p.first.second.value());
        exit(2);
    }
	// 输出常量表和函数表
//	auto v = p.first;
//	for (auto& it : v)
//		output << fmt::format("{}\n", it);
//	// 测试语义分析
//	std::cout << "OK!" << std::endl;
}

std::string oprToString(miniplc0::Operation opr) {
    std::string s;
    switch (opr) {
        case miniplc0::NOP:
            s = "nop";
            break;
        case miniplc0::BIPUSH:
            s = "bipush";
            break;
        case miniplc0::IPUSH:
            s = "ipush";
            break;
        case miniplc0::POP:
            s = "pop";
            break;
        case miniplc0::POP2:
            s = "pop2";
            break;
        case miniplc0::POPN:
            s = "popn";
            break;
        case miniplc0::LOADC:
            s = "loadc";
            break;
        case miniplc0::LOADA:
            s = "loada";
            break;
        case miniplc0::NEW:
            s = "new";
            break;
        case miniplc0::SNEW:
            s = "snew";
            break;
        case miniplc0::ILOAD:
            s = "iload";
            break;
        case miniplc0::ISTORE:
            s = "istore";
            break;
        case miniplc0::IADD:
            s = "iadd";
            break;
        case miniplc0::ISUB:
            s = "isub";
            break;
        case miniplc0::IMUL:
            s = "imul";
            break;
        case miniplc0::IDIV:
            s = "idiv";
            break;
        case miniplc0::INEG:
            s = "ineg";
            break;
        case miniplc0::ICMP:
            s = "icmp";
            break;
        case miniplc0::JMP:
            s = "jmp";
            break;
        case miniplc0::JE:
            s = "je";
            break;
        case miniplc0::JNE:
            s = "jne";
            break;
        case miniplc0::JL:
            s = "jl";
            break;
        case miniplc0::JGE:
            s = "jge";
            break;
        case miniplc0::JG:
            s = "jg";
            break;
        case miniplc0::JLE:
            s = "jle";
            break;
        case miniplc0::CALL:
            s = "call";
            break;
        case miniplc0::RET:
            s = "ret";
            break;
        case miniplc0::IRET:
            s = "iret";
            break;
        case miniplc0::IPRINT:
            s = "iprint";
            break;
        case miniplc0::ISCAN:
            s = "iscan";
            break;
    }
    return s;
}

// 汇编
void translateToAssemblingFile(std::istream& input, std::ostream& output) {
    auto tks = _tokenize(input);
    miniplc0::Analyser analyser(tks);
    auto p = analyser.Analyse();
    if (p.first.second.has_value()) {
        fmt::print(stderr, "Syntactic analysis error: {}\n", p.first.second.value());
        exit(2);
    }

    auto v = p.first.first; //vector<instruction>
    auto s = p.second.first; //constant
    auto f = p.second.second; //function
    // 输出常量表
    output << ".constants:" << std::endl;
    int n = s.size();
    for (int i=0;i<n;i++)
        output << i << "  " << s[i].type << "  " << "\"" << s[i].value << "\"" << std::endl;

    int funNum = 0;
    for (auto instruction : v) {
        if (instruction.getOffsetNum() == 0) {
            // 输出.start
            if (funNum == 0)
                output << ".start:" << std::endl;
            // 第一次输出.functions
            else if (funNum == 1) {
                output << ".functions:" << std::endl;
                n = f.size();
                for (int i=0;i<n;i++)
                    output << i << "  " << f[i].getIndex() << "  " << f[i].getNum() << "  " << 1 << std::endl;
                output << ".F" << funNum-1 << ":" << std::endl;
            }
            else
                output << ".F" << funNum-1 << ":" << std::endl;
            funNum++;
        }
        output << instruction.getOffsetNum() << "    " << oprToString(instruction.getOpr()) << "  ";
        int operandNum = instruction.getOperand().size();
        if (operandNum == 1)
            output << instruction.getOperand()[0];
        else if (operandNum == 2)
            output << instruction.getOperand()[0] << ",  " << instruction.getOperand()[1];
        output << std::endl;
    }
}

// 二进制
void translateToBinaryFile(std::istream& input, std::ostream& output) {
    auto tks = _tokenize(input);
    miniplc0::Analyser analyser(tks);
    auto p = analyser.Analyse();
    if (p.first.second.has_value()) {
        fmt::print(stderr, "Syntactic analysis error: {}\n", p.first.second.value());
        exit(2);
    }
    // 输出常量表和函数表
    auto v = p.first.first; //instruction
    auto s = p.second.first; //constant
    auto f = p.second.second; //function
    char magic[] = {4,3,3,0,3,10,2,9};
    char version[] = {0,0,0,0,0,0,0,1};


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

	if (program["-s"] == true && program["-c"] == true) {
		fmt::print(stderr, "You can only translate source code into an assembly file or a binary object file at one time.");
		exit(2);
	}

	// 转换程序入口
	if (program["-s"] == true) {
	    // 生成汇编代码
		translateToAssemblingFile(*input,*output);
	}
	else if (program["-c"] == true) {
	    // 生成二进制文件
		translateToBinaryFile(*input, *output);
	}

	else {
		fmt::print(stderr, "You must choose translate to an assembly file or a binary object file.");
		exit(2);
	}
	return 0;
}