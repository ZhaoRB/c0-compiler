#include "argparse.hpp"
#include "fmt/core.h"

#include "tokenizer/tokenizer.h"
#include "analyser/analyser.h"
#include "instruction/instruction.h"
#include "fmts.hpp"
#include "c0-vm/file.h"
#include "c0-vm/vm.h"
#include "c0-vm/exception.h"
#include "c0-vm/util/print.hpp"

#include <iostream>
#include <fstream>
#include <cstdlib>
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
}

void Analyse(std::istream& input, std::ostream& output) {
    auto tks = _tokenize(input);
    miniplc0::Analyser analyser(tks);
    auto p = analyser.Analyse();
    if (p.first.second.has_value()) {
        fmt::print(stderr, "Syntactic analysis error: {}\n", p.first.second.value());
        exit(2);
    }
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
        case miniplc0::CPRINT:
            s = "cprint";
            break;
        case miniplc0::PRINTL:
            s = "printl";
            break;
    }
    return s;
}

std::vector<char> changeToBinary(int operand, int length) {
    // 都是正数 转换成16进制就行
    std::vector<char> bytes;
    for (int i=0;i<length;i++)
        bytes.push_back(0);
    for(int i=length-1;i>=0;i--){
        bytes[i] = (char)(operand>>8*(length-i-1));
    }
    return bytes;
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
            // 这个地方有问题 如果没有全局变量 .start应该没有内容
            // 输出.start
            if (funNum == 0)
                output << ".start:" << std::endl;
            // 第一次输出.functions
            else if (funNum == 1) {
                if (instruction.getBinaryOpr().empty())
                    continue;
                output << ".functions:" << std::endl;
                n = f.size();
                for (int i=0;i<n;i++)
                    output << i << "  " << f[i].getIndex() << "  " << f[i].getNum() << "  " << 1 << std::endl;
                output << ".F" << funNum-1 << ":" << std::endl;
            }
            else {
                if (instruction.getBinaryOpr().empty())
                    continue;
                output << ".F" << funNum-1 << ":" << std::endl;
            }
            funNum++;
        }
        if (instruction.getBinaryOpr().empty())
            continue;
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
    output << (char)0x43 << (char)0x30 << (char)0x3a << (char)0x29;  //magic
    output << (char)0x00 << (char)0x00 << (char)0x00 << (char)0x01;  //version
    // constantcount
    int constant_count = s.size();
    std::vector<char> _constant_count = changeToBinary(constant_count,2);
    for (auto c : _constant_count)
        output << c;
    // constant_info
    for (auto info : s) {
        output << (char)0;  // type = S
        // string info
        // string info length
        int length = info.value.size();
        std::vector<char> binary_length = changeToBinary(length,2);
        for (char ch : binary_length)
            output << ch;
        // string info value
        for (char ch : info.value)
            output << ch;
    }

    // 以上没问题

    // start code info
    // instruction_count
    int index = 0;
    int n = v.size();
    for (int i=0;i<n;i++) {
        if (i != 0 && v[i].getOffsetNum() == 0) {
            index = i;
            break;
        }
    }
    std::vector<char> instruction_count = changeToBinary(index,2);
    for (auto c : instruction_count)
        output << c;
    // instuction info
    for (int i=0; i<index; i++) {
        auto tmp_instruction = v[i];
        for (auto c1 : tmp_instruction.getBinaryOpr())
            output << c1;
        for (auto bytes : tmp_instruction.getBinaryOperand()) {
            for (auto ch : bytes) {
                output << ch;
            }
        }
    }

    // function count
    int function_count = f.size();
    std::vector<char> _function_count = changeToBinary(function_count,2);
    for (auto c : _function_count)
        output << c;
    // function info
    n = f.size();
    for (int i=0; i<n; i++) {
        // name_index
        std::vector<char> name_index = changeToBinary(i,2);
        for (auto c1 : name_index)
            output << c1;
        // para_size
        std::vector<char> params_size = changeToBinary(f[i].getNum(),2);
        for (auto c : params_size)
            output << c;
        // level
        output << (char)0;
        output << (char)1;
        // instuction count
        int fun_instruction_count = 0;
        for(int i=index; i<n; i++) {
            if (i != 0 && v[i].getOffsetNum() == 0) {
                break;
            }
            fun_instruction_count++;
        }
        std::vector<char> _fun_instruction_count = changeToBinary(fun_instruction_count,2);
        for (auto c : _fun_instruction_count)
            output << c;
        // instruction info
        for (int i=index;i<n;i++) {
            if (i != 0 && v[i].getOffsetNum() == 0) {
                index = i;
                break;
            }
            for (auto c : v[i].getBinaryOpr())
                output << c;
            for (auto bytes : v[i].getBinaryOperand()) {
                for (auto ch : bytes)
                    output << ch;
            }
        }
    }
}

void assemble_text(std::ifstream* in, std::ofstream* out, bool run = false) {
    try {
        File f = File::parse_file_text(*in);
        // f.output_text(std::cout);
        f.output_binary(*out);
        if (run) {
            auto avm = std::move(vm::VM::make_vm(f));
            avm->start();
        }
    }
    catch (const std::exception& e) {
        println(std::cerr, e.what());
    }
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
	    // 将第一个的输出作为第二个的输入
        std::stringstream ss;
        std::string tmp = output_file;
        ss << output_file << ".zrb";
        if (program["-c"] == true)
        {
            input_file = ss.str();
            tmp = ss.str();
        }
		outf.open(tmp, std::ios::out | std::ios::trunc);
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
        translateToAssemblingFile(*input,*output);
        inf.close();
        outf.close();
        {
            std::ifstream* input;
            std::ostream* output;
            std::ifstream inf;
            std::ofstream outf;

            inf.open(input_file, std::ios::in);
            if (!inf) {
                exit(2);
            }
            input = &inf;
            outf.open(output_file, std::ios::out | std::ios::trunc|std::ios::binary);
            if (!outf) {
                inf.close();
                exit(2);
            }
            output = &outf;
            assemble_text(input, dynamic_cast<std::ofstream*>(output), false);
            inf.close();
            outf.close();
        }
        return 0;
	}

	else {
		fmt::print(stderr, "You must choose translate to an assembly file or a binary object file.");
		exit(2);
	}
	return 0;
}