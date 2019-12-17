#pragma once

#include "error/error.h"
#include "instruction/instruction.h"
#include "tokenizer/token.h"
#include "table/constant.h"
#include "table/function.h"
#include "table/symbol.h"

#include <utility>
#include <vector>
#include <optional>
#include <utility>
#include <map>
#include <cstdint>
#include <cstddef> // for std::size_t

namespace miniplc0 {

	class Analyser final {
	private:
		using uint64_t = std::uint64_t;
		using int64_t = std::int64_t;
		using uint32_t = std::uint32_t;
		using int32_t = std::int32_t;
    // 私有属性
    private:
        // 所有token
        std::vector<Token> _tokens;
        // 文件中的位置
        std::pair<uint64_t, uint64_t> _current_pos;
        // 符号表
        std::vector<Symbol> _symbols;

        // 这三个vector是存储最后要输出的信息，并不是程序运行时候所需要的数据结构
        // 指令表，用来构造 .s0 /.o0 文件
        std::vector<Instruction> _instructions;
        // 常量表和符号表
        std::vector<Constant> _constants;
        std::vector<Function> _functions;

        // 这个是指向当前的token
        std::size_t _offset;
        // 下一个 token 在栈的偏移
        int32_t _nextTokenIndex;

	public:
	    // 构造函数
		explicit Analyser(std::vector<Token> v)
		    : _tokens(std::move(v)), _current_pos(0,0), _symbols({}),
            _instructions({}), _constants({}), _functions({}),
            _offset(0), _nextTokenIndex(0)
        {}

		// 唯一接口
		std::pair<std::vector<Instruction>, std::optional<CompilationError>> Analyse();
	private:

	    // 所有的递归子程序
	    std::optional<CompilationError> analyseProgram();

		// Token 缓冲区相关操作

		// 返回下一个 token
		std::optional<Token> nextToken();
		// 回退一个 token
		void unreadToken();

		// 下面是符号表相关操作

		// helper function
		void _add(const Token&, std::map<std::string, int32_t>&);
		// 添加变量、常量、未初始化的变量
		void addVariable(const Token&);
		void addConstant(const Token&);
		void addUninitializedVariable(const Token&);
		// 是否被声明过
		bool isDeclared(const std::string&);
		// 是否是未初始化的变量
		bool isUninitializedVariable(const std::string&);
		// 是否是已初始化的变量
		bool isInitializedVariable(const std::string&);
		// 是否是常量
		bool isConstant(const std::string&);
		// 获得 {变量，常量} 在栈上的偏移
		int32_t getIndex(const std::string&);

	};
}