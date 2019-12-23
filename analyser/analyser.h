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

        // “语义分析”用到的
        // 符号表
        // 编译的时候用来判断是否重复声明，是否存在
        std::vector<Symbol> _constant_symbols;   //常量表
        std::vector<Symbol> _variable_symbols;   //变量表
        std::vector<CompilingFunction> _compilingFunctions;  //函数表
        bool isConstant;
        int _current_level;
        bool isVoid;
        bool isMain;
        bool hasMain;
        bool hasReturn;
        int _offsets; // 每当声明一个新函数的时候让offsets=0
        int functionIndex;

        // “目标代码生成”时使用
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
		    isConstant(false),_current_level(0),isVoid(false),
		    isMain(false),hasMain(false),hasReturn(false),
		    _offsets(0), functionIndex(0),
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
		// 添加
        void addToSymbolList(std::optional<Token> identifier);
        void addToCompilingFunctions(std::optional<Token> identifier, int paraNum, std::string type);
        // 查找
        std::optional<CompilingFunction> findFunction(std::optional<Token> identifier);
        std::optional<Symbol> findIdentifier(std::optional<Token> identifier);
        std::optional<Symbol> findConstantIdentifier(std::optional<Token> identifier);
        // 删除
        void deleteCurrentLevelSymbol();
        // 十进制数转成二进制补码 输出成相应长度的十六进制
        std::string changeToBinary(int operand, int length);


        // 所有的递归子程序
        std::optional<CompilationError> analyseProgram();
        std::optional<CompilationError> analyseVariableDeclaration();
        std::optional<CompilationError> analyseFunctionDeclaration();
        std::optional<CompilationError> analyseInitDeclarationList();
        std::optional<CompilationError> analyseInitDeclaration();
        std::optional<CompilationError> analyseExpression();
        std::optional<CompilationError> analyseMulExpression();
        std::optional<CompilationError> analyseUnaryExpression();
        std::optional<CompilationError> analyseFunctionCall(bool isExpression);
        std::optional<CompilationError> analyseParasList(std::optional<Token> functionType);
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