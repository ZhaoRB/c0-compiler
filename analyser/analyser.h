#pragma once

#include "error/error.h"
#include "instruction/instruction.h"
#include "tokenizer/token.h"
#include "table/constant.h"
#include "table/function.h"
#include "table/symbol.h"
#include "table/compilingFunction.h"

#include <utility>
#include <vector>
#include <optional>
#include <utility>
#include <map>
#include <cstdint>
#include <cstddef> // for std::size_t
#include <algorithm>
#include <stack>
#include <sstream>

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
        std::vector<Symbol> _constant_symbols;   //常量表
        std::vector<Symbol> _variable_symbols;   //变量表
        bool isConstant;
        int _current_level;
        // 一个运行时的函数表 存储函数名和参数类型
        std::vector<CompilingFunction> _compilingFunctions;

        //用于计算expession的值
        std::stack<int> _calculate_stack;

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
		    : _tokens(std::move(v)), _current_pos(0,0),
		    _constant_symbols({}),_variable_symbols({}),
		    isConstant(false),_current_level(0),
            _instructions({}), _constants({}), _functions({}),
            _offset(0), _nextTokenIndex(0) {}
		// 唯一接口
		std::pair<std::vector<Instruction>, std::optional<CompilationError>> Analyse();
	private:
	    /* Token 缓冲区相关操作 */
		// 返回下一个 token
		std::optional<Token> nextToken();
		// 回退一个 token
		void unreadToken();

		/* 工具函数 */
		// bool isTypeSpecifier(TokenType t);
		static bool isRelationalOperator(TokenType t);
        std::optional<CompilationError> checkDeclare(std::optional<Token> identifier);
        std::optional<CompilingFunction> findFunction(std::optional<Token> identifier);
        void addToSymbolList(std::optional<Token> identifier);
        void calculate(TokenType tk);
        std::optional<Symbol> findIdentifier(const std::string& name);

        // 所有的递static 归子程序
        std::optional<CompilationError> analyseProgram();
        std::optional<CompilationError> analyseVariableDeclaration();
        std::optional<CompilationError> analyseFunctionDeclaration();
        std::optional<CompilationError> analyseInitDeclarationList();
        std::optional<CompilationError> analyseInitDeclaration();
        std::optional<CompilationError> analyseExpression();
        std::optional<CompilationError> analyseMulExpression();
        std::optional<CompilationError> analyseUnaryExpression();
        std::optional<CompilationError> analyseFunctionCall();
        std::optional<CompilationError> analyseParasList();
        std::optional<CompilationError> analyseCompoundStatement();
        std::optional<CompilationError> analyseParasDeclaration();
        std::optional<CompilationError> analyseStatementSeq();
        std::optional<CompilationError> analyseStatement();
        std::optional<CompilationError> analyseCondition();
        std::optional<CompilationError> analyseConditionStatement();
        std::optional<CompilationError> analyseLoopStatement();
        std::optional<CompilationError> analyseJumpStatement();
        std::optional<CompilationError> analysePrintStatement();
        std::optional<CompilationError> analysePrintableList();
        std::optional<CompilationError> analyseScanStatement();
        std::optional<CompilationError> analyseAssignmentExpression();
	};
}