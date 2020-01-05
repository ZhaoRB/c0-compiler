#include "analyser.h"
#include "instruction/instruction.h"

#include <climits>
#include <sstream>
#include <utility>
#include <vector>
#include <algorithm>
#include <string>

namespace miniplc0 {

    using byte = unsigned char;
    /*
     * 对外唯一接口
     */
	std::pair<
		std::pair<std::vector<Instruction>, std::optional<CompilationError>>,
		std::pair<std::vector<Constant>, std::vector<CompilingFunction>>> Analyser::Analyse() {
        auto err = analyseProgram();
        if (err.has_value()) {
            auto pair1 = std::make_pair(std::vector<Instruction>(), err);
            auto pair2 = std::make_pair(std::vector<Constant>(),std::vector<CompilingFunction>());
            return std::make_pair(pair1,pair2);
        }
        else {
            auto pair1 = std::make_pair(_instructions, std::optional<CompilationError>());
            auto pair2 = std::make_pair(_constants,_compilingFunctions);
            return std::make_pair(pair1,pair2);
        }
    }


    /*
     * Token缓冲区操作
     */
    std::optional<Token> Analyser::nextToken() {
        if (_offset == _tokens.size())
            return {};
        // 考虑到 _tokens[0..._offset-1] 已经被分析过了
        // 所以我们选择 _tokens[0..._offset-1] 的 EndPos 作为当前位置
        _current_pos = _tokens[_offset].GetEndPos();
        return _tokens[_offset++];
    }
    void Analyser::unreadToken() {
        if (_offset == 0)
            DieAndPrint("analyser unreads token from the begining.");
        _current_pos = _tokens[_offset - 1].GetEndPos();
        _offset--;
    }

    /*
     * 工具函数
     */
    bool Analyser::isRelationalOperator(TokenType t) {
        return t == TokenType::IS_EQUAL_SIGN || t == TokenType::NOT_EQUAL_SIGN ||
                t == TokenType::LESS_THAN_SIGN|| t == TokenType::LESS_OR_EQUAL_SIGN ||
                t == TokenType::MORE_THAN_SIGN ||t == TokenType::MORE_OR_EQUAL_SIGN;
    }
    std::vector<byte> Analyser::changeToBinary(int operand, int length) {
        // 都是正数 转换成16进制就行
        std::vector<byte> bytes;
        for (int i=0;i<length;i++)
            bytes.push_back(0);
        for(int i=length-1;i>=0;i--){
            bytes[i] = (byte)(operand>>8*(length-i-1));
        }
        return bytes;
    }

    // 添加到符号表
    // 常量表和变量表
    void Analyser::addToSymbolList(std::optional<Token> identifier) {
        std::string _name = identifier.value().GetValueString();
        int _level = _current_level;
        miniplc0::Symbol symbol(_name,_level,_offsets);
        _offsets++;
        if (isConstant)
            _constant_symbols.push_back(symbol);
        else
            _variable_symbols.push_back(symbol);
    }
    // 运行时的函数表
    void Analyser::addToCompilingFunctions(std::optional<Token> identifier, int paraNum, std::string type) {
        CompilingFunction compilingFunction(identifier.value().GetValueString(),paraNum, std::move(type), functionIndex);
        _compilingFunctions.push_back(compilingFunction);
    }

    // 查找符号表： 声明时查重 ，使用时看有没有
    // 查常量表和表量表
    std::optional<Symbol> Analyser::findIdentifier(std::optional<Token> identifier) {
        // 从后往前找 也就是说先从当前的作用域查找
        // 常量表和变量表从后往前查找 如果都找到 要那个level高的
        // 没找到的话 返回一个空的
        auto name = identifier.value().GetValueString();
        int levelConstant = -1;
        int levelVariable = -1;
        int indexConstant = -1;
        int indexVariable = -1;
        int n = _constant_symbols.size();
        if (n != 0) {
            for (int i=n-1; i>=0; i--) {
                std::string _name = _constant_symbols[i].getName();
                if (name == _name) {
                    levelConstant = _constant_symbols[i].getLevel();
                    indexConstant = i;
                    break;
                }
            }
        }
        n = _variable_symbols.size();
        if (n != 0) {
            for (int i=n-1; i>=0; i--) {
                std::string _name = _variable_symbols[i].getName();
                if (name == _name) {
                    levelVariable = _variable_symbols[i].getLevel();
                    indexVariable = i;
                    break;
                }
            }
        }

        if (levelConstant == -1 && levelVariable == -1)
            return {};
        else if (levelConstant != -1 && levelVariable != -1) {
            if (levelConstant > levelVariable)
                return std::make_optional<Symbol>(_constant_symbols[indexConstant]);
            else
                return std::make_optional<Symbol>(_variable_symbols[indexVariable]);
        }
        else if (levelConstant != -1)
            return std::make_optional<Symbol>(_constant_symbols[indexConstant]);
        else
            return std::make_optional<Symbol>(_variable_symbols[indexVariable]);
    }
    std::optional<Symbol> Analyser::findConstantIdentifier(std::optional<Token> identifier) {
        auto name = identifier.value().GetValueString();
        int indexConstant = -1;
        int n = _constant_symbols.size();
        if (n == 0)
            return {};
        else {
            for (int i=n-1; i>=0; i--) {
                std::string _name = _constant_symbols[i].getName();
                if (name == _name) {
                    indexConstant = i;
                    break;
                }
            }
        }
        if (indexConstant == -1)
            return {};
        else {
            std::string _name = _constant_symbols[indexConstant].getName();
            int _level = _constant_symbols[indexConstant].getLevel();
            return std::make_optional<Symbol>(_constant_symbols[indexConstant]);
        }
        // return std::make_optional<Symbol>(_constant_symbols[indexConstant]);
        return {};
    }
    std::optional<Symbol> Analyser::findVariableIdentifier(std::optional<Token> identifier) {
        auto name = identifier.value().GetValueString();
        int indexConstant = -1;
        int n = _variable_symbols.size();
        if (n == 0)
            return {};
        else {
            for (int i=n-1; i>=0; i--) {
                std::string _name = _variable_symbols[i].getName();
                if (name == _name) {
                    indexConstant = i;
                    break;
                }
            }
        }
        if (indexConstant == -1)
            return {};
        else {
            std::string _name = _variable_symbols[indexConstant].getName();
            int _level = _variable_symbols[indexConstant].getLevel();
            return std::make_optional<Symbol>(_variable_symbols[indexConstant]);
        }
        // return std::make_optional<Symbol>(_constant_symbols[indexConstant]);
        return {};
    }
    // 查函数表
    std::optional<CompilingFunction> Analyser::findFunction(std::optional<Token> identifier) {
        auto name = identifier.value().GetValueString();
        for (auto & _compilingFunction : _compilingFunctions) {
            auto _name = _compilingFunction.getName();
            if (name == _name)
                return std::make_optional<CompilingFunction>(_compilingFunction);
        }
        // 没找到 返回空
        return {};
    }

    // 遇到右大括号 删除此level的常量和变量
    void Analyser::deleteCurrentLevelSymbol() {
        int n = _constant_symbols.size();
        if (n != 0) {
            for (int i=n-1; i>=0; i--) {
                if (_constant_symbols[i].getLevel() < _current_level)
                    break;
                _constant_symbols.pop_back();
            }
        }
        n = _variable_symbols.size();
        if (n != 0) {
            for (int i=n-1; i>=0; i--) {
                if (_variable_symbols[i].getLevel() < _current_level)
                    break;
                _variable_symbols.pop_back();
            }
        }
    }


	/*
	 * 所有的递归子程序
	 */
	// <c0-program> ::= {<variable-declaration>} {<function_declaration>}
	std::optional<CompilationError> Analyser::analyseProgram() {
	    // 变量声明语句是0个或多个
        while (true) {
            auto next = nextToken();
            auto type = next.value().GetType();
            // 没有main函数 返回错误
            if (!next.has_value())
                return std::make_optional<CompilationError>(_current_pos,ErrNoMainFunction);
            else {
                if (type == TokenType::CONST) {
                    unreadToken(); //退回const
                    auto err = analyseVariableDeclaration();
                    if (err.has_value())
                        return err;
                }
                else if (type == TokenType::INT) {
                    next = nextToken();
                    next = nextToken();
                    type = next.value().GetType();
                    if (type == TokenType::LEFT_BRACKET) {
                        unreadToken();
                        unreadToken();
                        unreadToken(); //退回int
                        break;
                    }
                    else {
                        unreadToken();
                        unreadToken();
                        unreadToken();
                        auto err = analyseVariableDeclaration();
                        if (err.has_value())
                            return err;
                    }
                }
                else if (type == TokenType::VOID) {
                    unreadToken();
                    break;
                }
                else
                    break;
            }
        }
        // 判断有没有全局变量
        // 生成一个instruction 用offset = -1标志分界
        Instruction divide;
        divide.setOffsetNum(0);
        _instructions.push_back(divide);
        // 函数声明语句是0个或多个
        while (true) {

            auto next = nextToken();
            if (!next.has_value()) {
                if (!hasMain)
                    return std::make_optional<CompilationError>(_current_pos,ErrNoMainFunction);
                else
                    return {};
            }
            else {
                unreadToken();
                auto err = analyseFunctionDeclaration();
                if (err.has_value())
                    return err;
            }
        }
    }

    /* 全局变量声明部分 */
    //    <variable-declaration>
    //    <variable-declaration> ::= [<const-qualifier>]<type-specifier><init-declarator-list>';'
    std::optional<CompilationError> Analyser::analyseVariableDeclaration() {
        auto next = nextToken();
        auto type = next.value().GetType();
        // CONST类型的变量必须被显示初始化
        if (type == TokenType::CONST) {
            isConstant = true;
            next = nextToken();
            type = next.value().GetType();
            if (type == TokenType::INT) {
                // <init declaration list>
                auto err = analyseInitDeclarationList();
                if (err.has_value())
                    return err;
            }
            else
                return std::make_optional<CompilationError>(_current_pos,ErrorCode::ErrNoTypeSpecifier);
            next = nextToken();
            if (next.value().GetType() != TokenType::SEMICOLON)
                return std::make_optional<CompilationError>(_current_pos,ErrorCode::ErrNoSemicolon);
        }
        else {
            if (type == TokenType::INT) {
                isConstant = false;
                // init declaration list
                auto err = analyseInitDeclarationList();
                if (err.has_value())
                    return err;
            }
            else
                return std::make_optional<CompilationError>(_current_pos,ErrorCode::ErrNoTypeSpecifier);
            next = nextToken();
            if (next.value().GetType() != TokenType::SEMICOLON)
                return std::make_optional<CompilationError>(_current_pos,ErrorCode::ErrNoSemicolon);
        }
        return {};
	}

    //    <init-declarator-list> ::= <init-declarator>{','<init-declarator>}
	std::optional<CompilationError> Analyser::analyseInitDeclarationList() {
	    auto err = analyseInitDeclaration();
	    if (err.has_value())
	        return err;
	    while (true) {
	        auto next = nextToken();
	        auto type = next.value().GetType();
	        if (type == TokenType::SEMICOLON) {
	            unreadToken();
                break;
	        }
	        else if (type == TokenType::COMMA_SIGN) {
                err = analyseInitDeclaration();
                if (err.has_value())
                    return err;
	        }
	        else
	            return std::make_optional<CompilationError>(_current_pos,ErrNoSemicolon);
	    }
	    return {};
	}
    //    <init-declarator> ::= <identifier>[<initializer>]
    //    <initializer> ::='='<expression>
    std::optional<CompilationError> Analyser::analyseInitDeclaration() {
        auto next = nextToken();
        auto type = next.value().GetType();
        auto identifier = next;
        if (!next.has_value() || type != TokenType::IDENTIFIER)
            return std::make_optional<CompilationError>(_current_pos,ErrorCode::ErrNeedIdentifier);
        else {
            // 检查同level是否已经被声明过
            // 检查 常量表 变量表
            std::string name = identifier.value().GetValueString();
            auto symbol = findIdentifier(identifier);
            // auto symbol = findConstantIdentifier(identifier);
            if (symbol.has_value() && symbol.value().getLevel() == _current_level)
                return std::make_optional<CompilationError>(_current_pos,ErrHasDeclared);

            next = nextToken();
            type = next.value().GetType();
            // 如果是常量 必须要初始化
            if (isConstant) {
                if (type != TokenType::EQUAL_SIGN)
                    return std::make_optional<CompilationError>(_current_pos,ErrConstantNeedValue);
                auto err = analyseExpression();
                if (err.has_value())
                    return err;
                // 在这里初始化赋值 需要吗？
            }
            else {
                // 未初始化不用管 也不用给默认值
                // 为初始化需要给分配空间
                if (type != TokenType::EQUAL_SIGN) {
                    unreadToken();
                    // 分配空间
                    std::vector<int> operand;
                    std::vector<std::vector<byte>> binary_operand;
                    std::vector<byte> binary_opr;
                    binary_opr.push_back(0x0c);
                    //binary_opr.push_back(12);
                    operand.push_back(1);
                    std::vector<byte> bytes = changeToBinary(1,4);
                    binary_operand.push_back(bytes);
                    Instruction instruction(Operation::SNEW,binary_opr,operand,binary_operand,opr_offset++);
                    _instructions.push_back(instruction);
                }
                else {
                    auto err = analyseExpression();
                    if (err.has_value())
                        return err;
                    // 初始化赋值 不需要吧
                }
            }
        }
        // 存入符号表
        addToSymbolList(identifier);
        return {};
    }

    /* expression 部分*/
    //    <expression> ::=<additive-expression>
    //    <additive-expression> ::=<multiplicative-expression>{<additive-operator><multiplicative-expression>}
    std::optional<CompilationError> Analyser::analyseExpression() {
        auto err = analyseMulExpression();
        if (err.has_value())
            return err;
        while (true) {
            auto next = nextToken();
            auto type = next.value().GetType();
            if (type == TokenType::PLUS_SIGN || type == TokenType::MINUS_SIGN) {
                err = analyseMulExpression();
                if (err.has_value())
                    return err;
                // 生成指令
                // 加减
                if (type == TokenType::PLUS_SIGN) {
                    std::vector<int> operand;
                    std::vector<std::vector<byte>> binary_operand;
                    std::vector<byte> binary_opr;
                    binary_opr.push_back(0x30);
                    //binary_opr.push_back(0);
                    Instruction instruction(Operation::IADD,binary_opr,operand,binary_operand,opr_offset++);
                    _instructions.push_back(instruction);
                }
                else {
                    std::vector<int> operand;
                    std::vector<std::vector<byte>> binary_operand;
                    std::vector<byte> binary_opr;
                    binary_opr.push_back(0x34);
                    //binary_opr.push_back(4);
                    Instruction instruction(Operation::ISUB,binary_opr,operand,binary_operand,opr_offset++);
                    _instructions.push_back(instruction);
                }
            }
            else {
                unreadToken();
                break;
            }
        }
        return {};
    }

    //    <multiplicative-expression> ::=<unary-expression>{<multiplicative-operator><unary-expression>}
    std::optional<CompilationError> Analyser::analyseMulExpression() {
        auto err = analyseUnaryExpression();
        if (err.has_value())
            return err;
        while (true) {
            auto next = nextToken();
            auto type = next.value().GetType();
            if (type == TokenType::MULTIPLICATION_SIGN || type == TokenType::DIVISION_SIGN) {
                err = analyseUnaryExpression();
                if (err.has_value())
                    return err;
                // 生成指令
                // 乘除
                if (type == TokenType::MULTIPLICATION_SIGN) {
                    std::vector<int> operand;
                    std::vector<std::vector<byte>> binary_operand;
                    std::vector<byte> binary_opr;
                    binary_opr.push_back(0x38);
                    // binary_opr.push_back(8);
                    Instruction instruction(Operation::IMUL,binary_opr,operand,binary_operand, opr_offset++);
                    _instructions.push_back(instruction);
                }
                else {
                    std::vector<int> operand;
                    std::vector<std::vector<byte>> binary_operand;
                    std::vector<byte> binary_opr;
                    binary_opr.push_back(0x3c);
                    // binary_opr.push_back(12);
                    Instruction instruction(Operation::IDIV,binary_opr,operand,binary_operand, opr_offset++);
                    _instructions.push_back(instruction);
                }
            }
            else {
                unreadToken();
                break;
            }
        }
        // 生成乘除指令
        return {};
    }

    //    <unary-expression> ::=[<unary-operator>]<primary-expression>
    //    <primary-expression> ::='('<expression>')'|<identifier>|<integer-literal>|<function-call>
    std::optional<CompilationError> Analyser::analyseUnaryExpression() {
	    // 预读 处理前面可能有的正负号
        auto next = nextToken();
        auto type = next.value().GetType();
        bool isNegative = false;
        if (type != TokenType::PLUS_SIGN && type != TokenType::MINUS_SIGN) {
            unreadToken();
        }
        else if (type == TokenType::MINUS_SIGN) {
            isNegative = true;
//            // 压进来一个 -1
//            std::vector<int> operand;
//            std::vector<std::string> binary_operand;
//            operand.push_back(-1);
//            binary_operand.emplace_back("ff ff ff ff");
//            Instruction instruction(Operation::IPUSH,"02",operand,binary_operand);
//            _instructions.push_back(instruction);
        }


	    // primary-expression 部分
	    next = nextToken();
	    type = next.value().GetType();
	    if (type == TokenType::IDENTIFIER) {
	        next = nextToken();
	        if (next.value().GetType() == TokenType::LEFT_BRACKET) {
	            unreadToken();
	            unreadToken();
	            auto err = analyseFunctionCall(true);
	            if (err.has_value())
	                return err;
	        }
	        // 是 identifier，判断在不在常量表或者变量表里面
	        else {
                unreadToken();
                unreadToken();
                auto identifier = nextToken(); //这个next是标识符
                // 查找
                auto result = findIdentifier(identifier);
                if (!result.has_value())
                    return std::make_optional<CompilationError>(_current_pos,ErrIdentifierNotDeclare);
                else {
                    // 不判断是否初始化了
                    // 从栈中取出identifier到栈顶
                    // loada 和 iload
                    int identifier_level = result.value().getLevel();
                    int level_diff = 1 - identifier_level;
                    int stackOffset = result.value().getOffset();
                    std::vector<int> operand;
                    std::vector<std::vector<byte>> binary_operand;
                    std::vector<byte> binary_opr;
                    binary_opr.push_back(0x0a);
                    //binary_opr.push_back(10);
                    operand.push_back(level_diff);
                    operand.push_back(stackOffset);
                    std::vector<byte> v1 = changeToBinary(level_diff,2); // 2字节
                    std::vector<byte> v2 = changeToBinary(stackOffset, 4); // 4字节
                    binary_operand.push_back(v1);
                    binary_operand.push_back(v2);
                    Instruction instruction(Operation::LOADA,binary_opr,operand,binary_operand, opr_offset++);
                    _instructions.push_back(instruction);
                    // iload
                    std::vector<int> operand2;
                    std::vector<std::vector<byte>> binary_operand2;
                    std::vector<byte> binary_opr2;
                    binary_opr2.push_back(0x10);
                    // binary_opr2.push_back(0);
                    Instruction instruction2(Operation::ILOAD,binary_opr2,operand2,binary_operand2,opr_offset++);
                    _instructions.push_back(instruction2);
                }
	        }
	    }
	    else if (type == TokenType::DECIMAL_UNSIGNED_INTEGER || type == TokenType::HEXADECIMAL_UNSIGNED_INTEGER) {
	        // 直接压栈
	        std::string s = next.value().GetValueString();
	        std::stringstream ss;
	        ss << s;
	        int value;
	        ss >> value;
	        std::vector<byte> binary_value = changeToBinary(value,4);
            std::vector<int> operand;
            std::vector<std::vector<byte >> binary_operand;
            operand.push_back(value);
            binary_operand.push_back(binary_value);
            std::vector<byte> binary_opr;
            binary_opr.push_back(0x02);
            // binary_opr.push_back(2);
            Instruction instruction(Operation::IPUSH,binary_opr,operand,binary_operand ,opr_offset++);
            _instructions.push_back(instruction);
	    }
	    else if (type == TokenType::LEFT_BRACKET) {
	        auto err = analyseExpression();
	        if (err.has_value())
	            return err;
	        next = nextToken();
	        if (next.value().GetType() != TokenType::RIGHT_BRACKET)
                return std::make_optional<CompilationError>(_current_pos,ErrNoBracket);
	    }
	    else
	        return std::make_optional<CompilationError>(_current_pos,ErrIncompleteExpression);

	    // 如果取负数的话 ineg取负数指令
	    if (isNegative) {
            std::vector<int> operand;
            std::vector<std::vector<byte>> binary_operand;
            std::vector<byte> binary_opr;
            binary_opr.push_back(0x40);
            // binary_opr.push_back(0);
            Instruction instruction(Operation::INEG,binary_opr,operand,binary_operand ,opr_offset++);
            _instructions.push_back(instruction);
	    }

        return {};
    }

    //    <function-call> ::=<identifier> '(' [<expression-list>] ')'
    //    <expression-list> ::=<expression>{','<expression>}
    std::optional<CompilationError> Analyser::analyseFunctionCall(bool isExpression) {
        // 因为肯定是确认了Identifier和（ 才能进来，所以跳过
        // 判断有没有这个函数 这个函数是否满足要求
        auto identifier = nextToken(); // 这个是identifier
        // 这个level里面有这样的变量
        auto result = findIdentifier(identifier);
        // 要检查变量表 看当前同等级中有没有同名的变量 有的话，出错了
        if (result.has_value() && _current_level == result.value().getLevel())
            return std::make_optional<CompilationError>(_current_pos,ErrFunctionNameHasBeenOverride);
        // 查函数表
        int number; // 参数个数
        auto oneFunction = findFunction(identifier);
        if (!oneFunction.has_value())
            return std::make_optional<CompilationError>(_current_pos,ErrFunctionNotDeclare);
        else {
            // 如果时从expression进来的 那么不可以是void返回类型
            if (oneFunction.value().getType() == "void" && isExpression)
                return std::make_optional<CompilationError>(_current_pos,ErrIncorrectType);
            number = oneFunction.value().getNum();
        }

        // expression-list
        // 检查类型啊
        int paraNum = 0;
        auto next = nextToken();
        next = nextToken();
        if (next.value().GetType() != TokenType::RIGHT_BRACKET) {
            unreadToken();
            auto err = analyseExpression();
            if (err.has_value())
                return err;
            paraNum++;
        }
        else
            unreadToken();
        while (true) {
            next = nextToken();
            auto type = next.value().GetType();
            if (type != TokenType::COMMA_SIGN) {
                unreadToken();
                break;
            }
            auto err = analyseExpression();
            if (err.has_value())
                return err;
            paraNum++;
        }
        // 判断参数个数是否正确
        if (number != paraNum)
            return std::make_optional<CompilationError>(_current_pos,ErrIncorrectParaNum);

        next = nextToken();
        auto type = next.value().GetType();
        if (!next.has_value() || type != TokenType::RIGHT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos,ErrNoBracket);

        // 生成指令
        // call指令
        std::vector<int> operand;
        std::vector<std::vector<byte>> binary_operand;
        std::vector<byte> binary_opr;
        binary_opr.push_back(0x80);
        // binary_opr.push_back(0);
        operand.push_back(oneFunction.value().getIndex());
        std::vector<byte> s = changeToBinary(oneFunction.value().getIndex(),4);
        binary_operand.push_back(s);
        Instruction instruction(Operation::CALL,binary_opr,operand,binary_operand,opr_offset++);
        _instructions.push_back(instruction);

	    return {};
	}

	/*函数声明的头部*/
    //    <function-definition> ::=<type-specifier><identifier><parameter-clause><compound-statement>
    //    <parameter-clause> ::='(' [<parameter-declaration-list>] ')'
    std::optional<CompilationError> Analyser::analyseFunctionDeclaration() {
        opr_offset = 0;     //第几条指令
        _offsets = 0;       //loada使用，在栈帧中的什么位置
        auto next = nextToken();
        auto type = next.value().GetType();
        if (type != TokenType::INT && type != TokenType::VOID)
            return std::make_optional<CompilationError>(_current_pos,ErrNoTypeSpecifier);
        auto functionType = next; // 填表的时候使用
        isVoid = functionType.value().GetType() == TokenType::VOID;

        next = nextToken();
        type = next.value().GetType();
        if (!next.has_value() || type != TokenType::IDENTIFIER)
            return std::make_optional<CompilationError>(_current_pos,ErrNeedIdentifier);
        if (next.value().GetValueString() == "main")
            hasMain = true;

        // 查重
        // 查符号表
        auto identifier = next;  // 存下来 填表的时候用
        auto result = findIdentifier(identifier);
        if (result.has_value())
            return std::make_optional<CompilationError>(_current_pos,ErrUsedIdentifierName);
        // 查函数表
        auto resultFunc = findFunction(identifier);
        if (resultFunc.has_value())
            return std::make_optional<CompilationError>(_current_pos,ErrUsedIdentifierName);

        // /将函数名添加到运行时的常量表
        Constant constant('S',identifier.value().GetValueString());
        _constants.push_back(constant);

        next = nextToken();
        type = next.value().GetType();
        if (!next.has_value() || type != TokenType::LEFT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos,ErrNoBracket);
        _current_level++; // 参数表的level要+1


        _offsets = 0;

        // 判断有无参数
        next = nextToken();
        type = next.value().GetType();
        // 没参数 直接加入函数表
        if (type == TokenType::RIGHT_BRACKET) {
            unreadToken();
            auto functionName = identifier.value().GetValueString();
            CompilingFunction compilingFunction(functionName,0, functionType.value().GetValueString(),functionIndex);
            _compilingFunctions.push_back(compilingFunction);
            functionIndex++;
        }
        // 有参数 在paraList里面添加到符号表
        else {
            unreadToken();
            auto err = analyseParasList(functionType);
            if (err.has_value())
                return err;
        }
        next = nextToken();
        type = next.value().GetType();
        if (!next.has_value() || type != TokenType::RIGHT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos,ErrNoBracket);
        _current_level--;

        // 函数体
        hasReturn = false;
        auto err = analyseCompoundStatement();
        if (err.has_value())
            return err;
        if (!hasReturn)
            return std::make_optional<CompilationError>(_current_pos,ErrNoReturnStatement);

        // 如果没有return 也要ret
        Operation opr;
        std::vector<byte> binary_opr;
        std::vector<int> operand;
        std::vector<std::vector<byte>> binary_operand;
        if (isVoid) {
            opr = Operation::RET;
            binary_opr.push_back(0x88);
            //binary_opr.push_back(8);
        }
        else {
            opr = Operation::IRET;
            binary_opr.push_back(0x89);
            //binary_opr.push_back(9);
        }
        Instruction instruction(opr,binary_opr,operand,binary_operand ,opr_offset++);
        _instructions.push_back(instruction);

        return {};
    }

    //    <parameter-declaration-list> ::=<parameter-declaration>{','<parameter-declaration>}
    //    <parameter-declaration> ::=[<const-qualifier>]<type-specifier><identifier>
    std::optional<CompilationError> Analyser::analyseParasList(std::optional<Token> functionType) {
        // 取出identifier 函数名
        unreadToken();
        unreadToken();
        auto identifier = nextToken();
        auto next = nextToken();

        auto err = analyseParasDeclaration();
        if (err.has_value())
            return err;
        int paraNum = 1;
        while (true) {
            next = nextToken();
            auto type = next.value().GetType();
            if (type == TokenType::COMMA_SIGN) {
                err = analyseParasDeclaration();
                if (err.has_value())
                    return err;
                paraNum++;
            }
            else {
                unreadToken();
                break;
            }
        }
        // 添加到函数表
        CompilingFunction compilingFunction(identifier.value().GetValueString(),paraNum,functionType.value().GetValueString(),functionIndex);
        _compilingFunctions.push_back(compilingFunction);
        functionIndex++;

        return {};
    }
    std::optional<CompilationError> Analyser::analyseParasDeclaration() {
        // 声明的时候将参数添加到符号表
        // 进行语义分析 不用管运行时的行为
        auto next = nextToken();
        auto type = next.value().GetType();
        if (!next.has_value())
            return std::make_optional<CompilationError>(_current_pos,ErrInvalidParameter);
        if (type != TokenType::CONST && type != TokenType::INT)
            return std::make_optional<CompilationError>(_current_pos,ErrInvalidParameter);
        if (type == TokenType::CONST) {
            // 常量表？
            isConstant = true;
            next = nextToken();
            type = next.value().GetType();
            if (!next.has_value() || type != TokenType::INT)
                return std::make_optional<CompilationError>(_current_pos,ErrInvalidParameter);
            next = nextToken();
            if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
                return std::make_optional<CompilationError>(_current_pos,ErrInvalidParameter);
            auto identifier = next;
            // 查重
            auto symbol = findIdentifier(identifier);
            if (symbol.has_value() && symbol.value().getLevel() == _current_level)
                return std::make_optional<CompilationError>(_current_pos,ErrHasDeclared);
            // 加入常量表
            addToSymbolList(identifier);

        }
        else {
            isConstant = false;
            next = nextToken();
            auto identifier = next;
            if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
                return std::make_optional<CompilationError>(_current_pos,ErrInvalidParameter);
            // 查重
            auto symbol = findIdentifier(identifier);
            if (symbol.has_value() && symbol.value().getLevel() == _current_level)
                return std::make_optional<CompilationError>(_current_pos,ErrHasDeclared);
            // 加入变量表
            addToSymbolList(identifier);
        }
        return {};
    }

    /* 函数体 */
    //    <compound-statement> ::='{' {<variable-declaration>} <statement-seq> '}'
    std::optional<CompilationError> Analyser::analyseCompoundStatement() {
        auto next = nextToken();
        auto type = next.value().GetType();
        if (!next.has_value() || type != TokenType::BIG_LEFT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos,ErrNoBigBracket);
        _current_level++;
        while(true) {
            next = nextToken();
            type = next.value().GetType();
            if (type == TokenType::INT || type == TokenType::CONST) {
                unreadToken();
                auto err = analyseVariableDeclaration();
                if (err.has_value())
                    return err;
            }
            else {
                unreadToken();
                break;
            }
        }
        auto err = analyseStatementSeq();
        if (err.has_value())
            return err;
        next = nextToken();
        type = next.value().GetType();
        if (!next.has_value() || type != TokenType::BIG_RIGHT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos,ErrNoBigBracket);
        // 删除这个level的常量和变量
        deleteCurrentLevelSymbol();
        _current_level--;
        return {};
    }
    //    <statement-seq> ::={<statement>}
    std::optional<CompilationError> Analyser::analyseStatementSeq() {
        while (true) {
            // 预读 判断是否结束了
            auto next = nextToken();
            auto type = next.value().GetType();
            if (type == TokenType::BIG_RIGHT_BRACKET) {
                unreadToken();
                break;
            }
            unreadToken();
            auto err = analyseStatement();
            if (err.has_value())
                return err;
        }
        return {};
    }
    //    <statement> ::='{' <statement-seq> '}'|<condition-statement>|<loop-statement>|<jump-statement>
    //             |<print-statement>|<scan-statement>|<assignment-expression>';'|<function-call>';'|';'
    std::optional<CompilationError> Analyser::analyseStatement() {
         auto next = nextToken();
         auto type = next.value().GetType();
         if (type == TokenType::BIG_LEFT_BRACKET) {
             _current_level++;
             auto err = analyseStatementSeq();
             if (err.has_value())
                 return err;
             next = nextToken();
             type = next.value().GetType();
             if (!next.has_value() || type != TokenType::BIG_RIGHT_BRACKET)
                 return std::make_optional<CompilationError>(_current_pos,ErrNoBigBracket);
             // 删除
             deleteCurrentLevelSymbol();
             _current_level--;
         }
         else if (type == TokenType::IF) {
             unreadToken();
             auto err = analyseConditionStatement();
             if (err.has_value())
                 return err;
         }
         else if (type == TokenType::WHILE) {
             unreadToken();
             auto err = analyseLoopStatement();
             if (err.has_value())
                 return err;
         }
         else if (type == TokenType::RETURN) {
             unreadToken();
             auto err = analyseJumpStatement();
             if (err.has_value())
                 return err;
         }
         else if (type == TokenType::SCAN) {
             unreadToken();
             auto err = analyseScanStatement();
             if (err.has_value())
                 return err;
         }
         else if (type == TokenType::PRINT) {
             unreadToken();
             auto err = analysePrintStatement();
             if (err.has_value())
                 return err;
         }
         else if (type == TokenType::IDENTIFIER) {
             next = nextToken();
             type = next.value().GetType();
             if (type == TokenType::EQUAL_SIGN) {
                 unreadToken();
                 unreadToken();
                 auto err = analyseAssignmentExpression();
                 if (err.has_value())
                     return err;
             }
             else if (type == TokenType::LEFT_BRACKET) {
                 unreadToken();
                 unreadToken();
                 auto err = analyseFunctionCall(false);
                 if (err.has_value())
                     return err;
             }
             else
                 std::make_optional<CompilationError>(_current_pos,ErrUnexpected);
             next = nextToken();
             if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
                 return std::make_optional<CompilationError>(_current_pos,ErrNoSemicolon);
         }
         else if (type == TokenType::SEMICOLON);
         else
             return std::make_optional<CompilationError>(_current_pos,ErrInvalidStatement);
        return {};
    }

    //    <condition> ::=<expression>[<relational-operator><expression>]
    std::optional<CompilationError> Analyser::analyseCondition() {
        auto err = analyseExpression();
        if (err.has_value())
            return err;
        auto next = nextToken();
        auto type = next.value().GetType();
        if (isRelationalOperator(type)) {
            err = analyseExpression();
            if (err.has_value())
                return err;
            // 比较操作 cmp
            std::vector<int> operand;
            std::vector<std::vector<byte>> binary_operand;
            std::vector<byte> binary_opr;
            binary_opr.push_back(0x44);
            // binary_opr.push_back(4);
            Instruction instruction(Operation::ICMP,binary_opr,operand,binary_operand ,opr_offset++);
            _instructions.push_back(instruction);
            // 根据具体是什么符号 跳转 不满足if条件的时候跳转 也就是相反的时候
            Operation opr;
            std::vector<byte> _binary_opr;
            switch (type) {
                case TokenType::IS_EQUAL_SIGN :
                    _binary_opr.push_back(0x72);
                    opr = Operation::JNE;
                    break;
                case TokenType::NOT_EQUAL_SIGN :
                    _binary_opr.push_back(0x71);
                    opr = Operation::JE;
                    break;
                case TokenType::LESS_THAN_SIGN :
                    _binary_opr.push_back(0x74);
                    opr = Operation::JGE;
                    break;
                case TokenType::LESS_OR_EQUAL_SIGN :
                    _binary_opr.push_back(0x75);
                    opr = Operation::JG;
                    break;
                case TokenType::MORE_THAN_SIGN :
                    _binary_opr.push_back(0x76);
                    opr = Operation::JLE;
                    break;
                case TokenType::MORE_OR_EQUAL_SIGN :
                    _binary_opr.push_back(0x73);
                    opr = Operation::JL;
                    break;
                default:
                    opr = Operation::JMP;
                    break;
            }
            std::vector<int> _operand;
            std::vector<std::vector<byte>> _binary_operand;
            Instruction instruction1(opr,_binary_opr,_operand,_binary_operand,opr_offset++);
            _instructions.push_back(instruction1);

            // 之后还要回填这个offset
        }
        else {
            unreadToken();
            // 如果没有关系运算符的话 通过这个expression来判断 true or false
            // 如果value不是0 跳转 jne
            std::vector<int> operand;
            std::vector<std::vector<byte>> binary_operand;
            std::vector<byte> binary_opr;
            binary_opr.push_back(0x71);
            // binary_opr.push_back(2);
            Instruction instruction(Operation::JE,binary_opr,operand,binary_operand ,opr_offset++);
            _instructions.push_back(instruction);

            // 要回填这个offset
        }
        return {};
    }
    //    <condition-statement> ::='if' '(' <condition> ')' <statement> ['else' <statement>]
    std::optional<CompilationError> Analyser::analyseConditionStatement() {
        isLoop = false;
        auto next = nextToken();
        next = nextToken();
        auto type = next.value().GetType();
        if (!next.has_value() || type != TokenType::LEFT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos,ErrNoBracket);
        auto err = analyseCondition();
        if (err.has_value())
            return err;
        next = nextToken();
        if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos,ErrNoBracket);
        err = analyseStatement();
        if (err.has_value())
            return err;

        // 回填
        // else 或者其他指令
        // 将当前的偏移 +1 回填到跳转指令的offset
        int n = _instructions.size();
        for (int i=n-1;i>=0;i--) {
            Instruction tmp = _instructions[i];
            if (tmp.getOpr() == Operation::JE || tmp.getOpr() == Operation::JNE || tmp.getOpr() == Operation::JL
                || tmp.getOpr() == Operation::JLE || tmp.getOpr() == Operation::JGE || tmp.getOpr() == Operation::JG)
            {
                if (tmp.getOperand().size() == 0) {
                    next = nextToken();
                    if (next.value().GetType() == TokenType::ELSE) {
                        _instructions[i].addOperand(opr_offset+1);
                        _instructions[i].addBinaryOperand(changeToBinary(opr_offset+1,2));
                    }
                    else {
                        _instructions[i].addOperand(opr_offset);
                        _instructions[i].addBinaryOperand(changeToBinary(opr_offset,2));
                    }
                    unreadToken();
                    break;
                }
            }
        }

        next = nextToken();
        if (next.value().GetType() == TokenType::ELSE) {
            // 如果有else的话 生成跳转指令 跳转到 else 的statement之后的一句
            // 等待回填
            std::vector<int> operand;
            std::vector<std::vector<byte>> binary_operand;
            std::vector<byte> binary_opr;
            binary_opr.push_back(0x70);
            // binary_opr.push_back(0);
            Instruction instruction(Operation::JMP,binary_opr,operand,binary_operand ,opr_offset++);
            _instructions.push_back(instruction);

            err = analyseStatement();
            if (err.has_value())
                return err;

            // 分析完了 回填
            n = _instructions.size();
            for (int i=n-1;i>=0;i--) {
                Instruction tmp = _instructions[i];
                if (tmp.getOpr() == Operation::JMP) {
                    if (tmp.getOperand().size() == 0) {
                        _instructions[i].addOperand(opr_offset);
                        _instructions[i].addBinaryOperand(changeToBinary(opr_offset,2));
                        break;
                    }
                }
            }
        }
        else {
            unreadToken();
        }
        return {};
    }
    //    <loop-statement> ::='while' '(' <condition> ')' <statement>
    std::optional<CompilationError> Analyser::analyseLoopStatement() {
        isLoop = true;
        auto next = nextToken();
        next = nextToken();
        auto type = next.value().GetType();
        if (!next.has_value() || type != TokenType::LEFT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos,ErrNoBracket);

        int while_offset = opr_offset + 1;

        auto err = analyseCondition();
        if (err.has_value())
            return err;
        next = nextToken();
        if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos,ErrNoBracket);
        err = analyseStatement();
        if (err.has_value())
            return err;

        // 回填这个地方的offset
        int n = _instructions.size();
        for (int i=n-1;i>=0;i--) {
            Instruction tmp = _instructions[i];
            if (tmp.getOpr() == Operation::JE || tmp.getOpr() == Operation::JNE || tmp.getOpr() == Operation::JL
                || tmp.getOpr() == Operation::JLE || tmp.getOpr() == Operation::JGE || tmp.getOpr() == Operation::JG)
            {
                if (tmp.getOperand().size() == 0) {
                    _instructions[i].addOperand(opr_offset+1);
                    _instructions[i].addBinaryOperand(changeToBinary(opr_offset+1,2));
                    break;
                }
            }
        }
        // 跳回原来的condition 语句
        std::vector<int> operand;
        std::vector<std::vector<byte>> binary_operand;
        std::vector<byte> binary_opr;
        binary_opr.push_back(0x70);
        // binary_opr.push_back(0);
        operand.push_back(while_offset-1);
        binary_operand.push_back(changeToBinary(while_offset-1,2));
        Instruction instruction(Operation::JMP,binary_opr,operand,binary_operand ,opr_offset++);
        _instructions.push_back(instruction);

        return {};
    }
    //    <jump-statement> ::= <return-statement>
    //    <return-statement> ::= 'return' [<expression>] ';'
    std::optional<CompilationError> Analyser::analyseJumpStatement() {
        auto next = nextToken();
        next = nextToken();
        auto type = next.value().GetType();
        // 如果不是void 那肯定是int类型 就一定要有返回值
        if (!isVoid) {
            if (type == TokenType::SEMICOLON)
                return std::make_optional<CompilationError>(_current_pos,ErrIncorrectReturnType);
            unreadToken();
            auto err = analyseExpression();
            if (err.has_value())
                return err;
            next = nextToken();
        }
        if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
            return std::make_optional<CompilationError>(_current_pos,ErrNoSemicolon);
        hasReturn = true;

        // ret 或者 iret
        Operation opr;
        std::vector<byte> binary_opr;
        std::vector<int> operand;
        std::vector<std::vector<byte>> binary_operand;
        if (isVoid) {
            opr = Operation::RET;
            binary_opr.push_back(0x88);
            // binary_opr.push_back(8);
        }
        else {
            opr = Operation::IRET;
            binary_opr.push_back(0x89);
            // binary_opr.push_back(9);
        }
        Instruction instruction(opr,binary_opr,operand,binary_operand ,opr_offset++);
        _instructions.push_back(instruction);

        return {};
    }
    //    <scan-statement>  ::= 'scan' '(' <identifier> ')' ';'
    std::optional<CompilationError> Analyser::analyseScanStatement() {
        auto next = nextToken();
        next = nextToken();
        auto type = next.value().GetType();
        if (!next.has_value() || type != TokenType::LEFT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos,ErrNoBracket);
        next = nextToken();
        type = next.value().GetType();
        auto identifier = next;
        if (type == TokenType::IDENTIFIER) {
            // 查看有没有
            // 不能是常量
            auto symbol = findVariableIdentifier(identifier);
            if (!symbol.has_value())
                return std::make_optional<CompilationError>(_current_pos,ErrIdentifierNotDeclare);
//            auto symbol1 = findConstantIdentifier(identifier);
//            if (symbol1.has_value())
//                return std::make_optional<CompilationError>(_current_pos,ErrAssignToConstant);


            // 有的话 给赋值
            // 先把identifier的地址加载过来 loada
            int level_diff = 1 - symbol.value().getLevel();
            int stack_offset = symbol.value().getOffset();
            std::vector<int> operand1;
            std::vector<std::vector<byte>> binary_operand1;
            std::vector<byte> binary_opr1;
            binary_opr1.push_back(0x0a);
            // binary_opr1.push_back(10);
            operand1.push_back(level_diff);
            operand1.push_back(stack_offset);
            binary_operand1.push_back(changeToBinary(level_diff,2));
            binary_operand1.push_back(changeToBinary(stack_offset,4));
            Instruction instruction1(Operation::LOADA,binary_opr1,operand1,binary_operand1 ,opr_offset++);
            _instructions.push_back(instruction1);

            // 先生成iscan指令
            std::vector<int> operand;
            std::vector<std::vector<byte>> binary_operand;
            std::vector<byte> binary_opr;
            binary_opr.push_back(0xb0);
            // binary_opr.push_back(0);
            Instruction instruction(Operation::ISCAN,binary_opr,operand,binary_operand ,opr_offset++);
            _instructions.push_back(instruction);

            // 储存给变量
            std::vector<int> operand2;
            std::vector<std::vector<byte>> binary_operand2;
            std::vector<byte> binary_opr2;
            binary_opr2.push_back(0x20);
            // binary_opr2.push_back(0);
            Instruction instruction2(Operation::ISTORE,binary_opr2,operand2,binary_operand2 ,opr_offset++);
            _instructions.push_back(instruction2);

        }
        else
            return std::make_optional<CompilationError>(_current_pos,ErrNeedIdentifier);
        next = nextToken();
        type = next.value().GetType();
        if (!next.has_value() || type != TokenType::RIGHT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos,ErrNoBracket);
        next = nextToken();
        type = next.value().GetType();
        if (!next.has_value() || type != TokenType::SEMICOLON)
            return std::make_optional<CompilationError>(_current_pos,ErrNoSemicolon);
        return {};
    }
    //    <print-statement> ::= 'print' '(' [<printable-list>] ')' ';'
    std::optional<CompilationError> Analyser::analysePrintStatement() {
        auto next = nextToken();
        next = nextToken();
        auto type = next.value().GetType();
        if (!next.has_value() || type != TokenType::LEFT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos,ErrNoBracket);
        next = nextToken();
        if (next.value().GetType() != TokenType::RIGHT_BRACKET) {
            unreadToken();
            auto err = analysePrintableList();
            if (err.has_value())
                return err;
            next = nextToken();
        }
        if (!next.has_value() || next.value().GetType() != RIGHT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos,ErrNoBracket);
        next = nextToken();
        type = next.value().GetType();
        if (!next.has_value() || type != TokenType::SEMICOLON)
            return std::make_optional<CompilationError>(_current_pos,ErrNoSemicolon);
        return {};
    }
    //    <printable-list>  ::= <printable> {',' <printable>}
    //    <printable> ::= <expression>
    std::optional<CompilationError> Analyser::analysePrintableList() {
        auto err = analyseExpression();
        if (err.has_value())
            return err;
        // 输出指令
        std::vector<int> operand;
        std::vector<std::vector<byte>> binary_operand;
        std::vector<byte> binary_opr;
        // binary_opr.push_back(10);
        binary_opr.push_back(0xa0);
        Instruction instruction(Operation::IPRINT,binary_opr,operand,binary_operand ,opr_offset++);
        _instructions.push_back(instruction);

        while (true) {
            auto next = nextToken();
            if (next.value().GetType() == TokenType::COMMA_SIGN) {
                // 遇到一个逗号 输出一个空格
                // bipush 32
                std::vector<byte> binary_value = changeToBinary(32,4);
                std::vector<int> operand10;
                std::vector<std::vector<byte>> binary_operand10;
                std::vector<byte> binary_opr10;
                binary_opr10.push_back(0x01);
                operand10.push_back(32);
                binary_operand10.push_back(binary_value);
                Instruction instruction10(Operation::BIPUSH,binary_opr10,operand10,binary_operand10 ,opr_offset++);
                _instructions.push_back(instruction10);
                // cprint
                std::vector<int> operand20;
                std::vector<std::vector<byte>> binary_operand20;
                std::vector<byte> binary_opr20;
                binary_opr20.push_back(0xa2);
                Instruction instruction20(Operation::CPRINT,binary_opr20,operand20,binary_operand20 ,opr_offset++);
                _instructions.push_back(instruction20);

                err = analyseExpression();
                if (err.has_value())
                    return err;
                // 输出指令
                std::vector<int> operand1;
                std::vector<std::vector<byte>> binary_operand1;
                std::vector<byte> binary_opr1;
                // binary_opr1.push_back(10);
                binary_opr1.push_back(0xa0);
                Instruction instruction1(Operation::IPRINT,binary_opr1,operand1,binary_operand1 ,opr_offset++);
                _instructions.push_back(instruction1);
            }
            else{
                unreadToken();
                break;
            }
        }
        // 所有输出完了之后 输出一个换行
        std::vector<int> operand30;
        std::vector<std::vector<byte>> binary_operand30;
        std::vector<byte> binary_opr30;
        binary_opr30.push_back(0xaf);
        Instruction instruction30(Operation::PRINTL,binary_opr30,operand30,binary_operand30 ,opr_offset++);
        _instructions.push_back(instruction30);

        return {};
    }
    //    <assignment-expression> ::=<identifier><assignment-operator><expression>
    std::optional<CompilationError> Analyser::analyseAssignmentExpression() {
        auto identifier = nextToken();
        // 有且不能是常量
        auto symbol = findVariableIdentifier(identifier);
        if (!symbol.has_value())
            return std::make_optional<CompilationError>(_current_pos,ErrIdentifierNotDeclare);
//        auto symbol1 = findConstantIdentifier(identifier);
//        if (symbol1.has_value())
//            return std::make_optional<CompilationError>(_current_pos,ErrAssignToConstant);

        // 将要被赋值的identifier的地址拿出来
        int level_diff = 1 - symbol.value().getLevel();
        int stack_offset = symbol.value().getOffset();
        std::vector<int> operand1;
        std::vector<std::vector<byte>> binary_operand1;
        std::vector<byte> binary_opr1;
        binary_opr1.push_back(0x0a);
        // binary_opr1.push_back(10);
        operand1.push_back(level_diff);
        operand1.push_back(stack_offset);
        binary_operand1.push_back(changeToBinary(level_diff,2));
        binary_operand1.push_back(changeToBinary(stack_offset,4));
        Instruction instruction1(Operation::LOADA,binary_opr1,operand1,binary_operand1 ,opr_offset++);
        _instructions.push_back(instruction1);

        // expression 就将 value放到了栈顶
        auto next = nextToken();
        auto err = analyseExpression();
        if (err.has_value())
            return err;

        // istore 存储值就 OK
        std::vector<int> operand2;
        std::vector<std::vector<byte>> binary_operand2;
        std::vector<byte> binary_opr2;
        binary_opr2.push_back(0x20);
        // binary_opr2.push_back(0);
        Instruction instruction2(Operation::ISTORE,binary_opr2,operand2,binary_operand2 ,opr_offset++);
        _instructions.push_back(instruction2);

        return {};
    }
}