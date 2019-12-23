#include "analyser.h"

#include <climits>
#include <sstream>
#include <utility>
#include <vector>
#include <algorithm>
#include <string>

namespace miniplc0 {

    /*
     * 对外唯一接口
     */
    std::pair<std::vector<Instruction>, std::optional<CompilationError>> Analyser::Analyse() {
		auto err = analyseProgram();
		if (err.has_value())
			return std::make_pair(std::vector<Instruction>(), err);
		else
			return std::make_pair(_instructions, std::optional<CompilationError>());
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

    // 添加到符号表
    // 常量表和变量表
    void Analyser::addToSymbolList(std::optional<Token> identifier) {
        std::string _name = identifier.value().GetValueString();
        int _level = _current_level;
        miniplc0::Symbol symbol(_name,_level);
        if (isConstant)
            _constant_symbols.push_back(symbol);
        else
            _variable_symbols.push_back(symbol);
    }
    // 运行时的函数表
    void Analyser::addToCompilingFunctions(std::optional<Token> identifier, int paraNum, std::string type) {
        CompilingFunction compilingFunction(identifier.value().GetValueString(),paraNum, std::move(type));
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
            return std::make_optional<Symbol>(_name,_level);
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
                else
                    break;
            }
        }
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

        // 添加到符号表
        // 等待实现
        // constant表可能会添加元素
        // 生成加载全局变量的指令

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
            }
            else {
                if (type != TokenType::EQUAL_SIGN)
                    unreadToken();
                else {
                    auto err = analyseExpression();
                    if (err.has_value())
                        return err;
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
            }
            else {
                unreadToken();
                break;
            }
        }
        // 生成相应操作？

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
        if (type == TokenType::PLUS_SIGN) {
            int a = 1; //zhanwei
        }
        else if (type == TokenType::MINUS_SIGN) {
            int a = 1;  // zhanwei
        }
        else
            unreadToken();

	    // primary-expression 部分
	    next = nextToken();
	    type = next.value().GetType();
	    if (type == TokenType::IDENTIFIER) {
	        // 生成指令
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
                }
	        }
	    }
	    else if (type == TokenType::DECIMAL_UNSIGNED_INTEGER || type == TokenType::HEXADECIMAL_UNSIGNED_INTEGER) {
	        // 直接压栈
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

	    // 如果取负数的话 还要进行相关操作

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
        auto err = analyseExpression();
        if (err.has_value())
            return err;
        paraNum++;
        while (true) {
            next = nextToken();
            auto type = next.value().GetType();
            if (type != TokenType::COMMA_SIGN) {
                unreadToken();
                break;
            }
            err = analyseExpression();
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
	    return {};
	}

	/*函数声明的头部*/
    //    <function-definition> ::=<type-specifier><identifier><parameter-clause><compound-statement>
    //    <parameter-clause> ::='(' [<parameter-declaration-list>] ')'
    std::optional<CompilationError> Analyser::analyseFunctionDeclaration() {
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

        next = nextToken();
        type = next.value().GetType();
        if (!next.has_value() || type != TokenType::LEFT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos,ErrNoBracket);
        _current_level++; // 参数表的level要+1

        // 判断有无参数
        next = nextToken();
        type = next.value().GetType();
        // 没参数 直接加入函数表
        if (type == TokenType::RIGHT_BRACKET) {
            unreadToken();
            auto functionName = identifier.value().GetValueString();
            CompilingFunction compilingFunction(functionName,0, functionType.value().GetValueString());
            _compilingFunctions.push_back(compilingFunction);
        }
        // 有参数 在paraList里面添加到符号表
        else {
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
            else
                break;
        }
        // 添加到函数表
        CompilingFunction compilingFunction(identifier.value().GetValueString(),paraNum,functionType.value().GetValueString());
        _compilingFunctions.push_back(compilingFunction);

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
             std::make_optional<CompilationError>(_current_pos,ErrInvalidStatement);
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
            // 不同操作
            err = analyseExpression();
            if (err.has_value())
                return err;
        }
        // 如果没有关系运算符的话 通过这个expression来判断 true or false
        return {};
    }
    //    <condition-statement> ::='if' '(' <condition> ')' <statement> ['else' <statement>]
    std::optional<CompilationError> Analyser::analyseConditionStatement() {
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
        next = nextToken();
        if (next.value().GetType() == TokenType::ELSE) {
            err = analyseStatement();
            if (err.has_value())
                return err;
        }
        return {};
    }
    //    <loop-statement> ::='while' '(' <condition> ')' <statement>
    std::optional<CompilationError> Analyser::analyseLoopStatement() {
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
            auto symbol = findIdentifier(identifier);
            if (!symbol.has_value())
                return std::make_optional<CompilationError>(_current_pos,ErrIdentifierNotDeclare);
            symbol = findConstantIdentifier(identifier);
            if (symbol.has_value())
                return std::make_optional<CompilationError>(_current_pos,ErrAssignToConstant);
            // 有的话 给赋值
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
        while (true) {
            auto next = nextToken();
            if (next.value().GetType() == TokenType::COMMA_SIGN) {
                err = analyseExpression();
                if (err.has_value())
                    return err;
            }
            else
                break;
        }
        return {};
    }
    //    <assignment-expression> ::=<identifier><assignment-operator><expression>
    std::optional<CompilationError> Analyser::analyseAssignmentExpression() {
        auto identifier = nextToken();
        // 有且不能是常量
        auto symbol = findIdentifier(identifier);
        if (!symbol.has_value())
            return std::make_optional<CompilationError>(_current_pos,ErrIdentifierNotDeclare);
        symbol = findConstantIdentifier(identifier);
        if (symbol.has_value())
            return std::make_optional<CompilationError>(_current_pos,ErrAssignToConstant);
        auto next = nextToken();
        auto err = analyseExpression();
        if (err.has_value())
            return err;
        return {};
    }
}