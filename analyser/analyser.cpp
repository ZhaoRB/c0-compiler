#include "analyser.h"

#include <climits>
#include <sstream>

namespace miniplc0 {

    // 对外唯一接口
    std::pair<std::vector<Instruction>, std::optional<CompilationError>> Analyser::Analyse() {
		auto err = analyseProgram();
		if (err.has_value())
			return std::make_pair(std::vector<Instruction>(), err);
		else
			return std::make_pair(_instructions, std::optional<CompilationError>());
	}

    // Token缓冲区操作
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

    /*工具函数*/
    bool Analyser::isRelationalOperator(TokenType t) {
        return t == TokenType::IS_EQUAL_SIGN || t == TokenType::NOT_EQUAL_SIGN ||
                t == TokenType::LESS_THAN_SIGN|| t == TokenType::LESS_OR_EQUAL_SIGN ||
                t == TokenType::MORE_THAN_SIGN ||t == TokenType::MORE_OR_EQUAL_SIGN;
    }

	/* 所有的递归子程序 */
	// <c0-program> ::= {<variable-declaration>} {<function_declaration>}
	std::optional<CompilationError> Analyser::analyseProgram() {
	    // 变量声明语句是0个或多个
        while (true) {
            auto next = nextToken();
            auto type = next.value().GetType();
            // 文件为空, 正常返回
            if (!next.has_value())
                return {};
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
                    if (type == TokenType::EQUAL_SIGN) {
                        unreadToken();
                        unreadToken();
                        unreadToken(); //退回int
                        break;
                    }
                    else {
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
            if (!next.has_value())
                return {};
            else {
                unreadToken();
                auto err = analyseFunctionDeclaration();
                if (err.has_value())
                    return err;
            }
        }
    }

    //    <variable-declaration>
    //    <variable-declaration> ::= [<const-qualifier>]<type-specifier><init-declarator-list>';'
    std::optional<CompilationError> Analyser::analyseVariableDeclaration() {
        auto next = nextToken();
        auto type = next.value().GetType();
        // CONST类型的变量必须被显示初始化
        if (type == TokenType::CONST) {
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
    //    <init-declarator> ::= <identifier>[<initializer>]
    //    <initializer> ::='='<expression>
	std::optional<CompilationError> Analyser::analyseInitDeclarationList() {
	    auto err = analyseInitDeclaration();
	    if (err.has_value())
	        return err;
	    while (true) {
	        auto next = nextToken();
	        auto type = next.value().GetType();
	        if (type == TokenType::SEMICOLON)
                break;
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
    std::optional<CompilationError> Analyser::analyseInitDeclaration() {
        auto next = nextToken();
        auto type = next.value().GetType();
        if (!next.has_value() || type != TokenType::IDENTIFIER)
            return std::make_optional<CompilationError>(_current_pos,ErrorCode::ErrNeedIdentifier);
        else {
            next = nextToken();
            type = next.value().GetType();
            if (type == TokenType::EQUAL_SIGN) {
                auto err = analyseExpression();
                if (err.has_value())
                    return err;
            }
            else
                unreadToken();
        }
        return {};
    }

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
            }
            else
                break;
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
            }
            else
                break;
        }
        // 生成乘除指令
        return {};
    }

    //    <unary-expression> ::=[<unary-operator>]<primary-expression>
    //    <primary-expression> ::='('<expression>')'|<identifier>|<integer-literal>|<function-call>
    std::optional<CompilationError> Analyser::analyseUnaryExpression() {
	    auto next = nextToken();
	    auto type = next.value().GetType();
	    if (type != TokenType::EQUAL_SIGN && type != TokenType::MINUS_SIGN)
	        unreadToken();
	    next = nextToken();
	    type = next.value().GetType();
	    if (type == TokenType::IDENTIFIER) {
	        // 生成指令
	        next = nextToken();
	        if (next.value().GetType() == TokenType::LEFT_BRACKET) {
	            unreadToken();
	            unreadToken();
	            auto err = analyseFunctionCall();
	            if (err.has_value())
	                return err;
	        }
	        else
	            unreadToken();
	    }
	    else if (type == TokenType::DECIMAL_UNSIGNED_INTEGER || type == TokenType::HEXADECIMAL_UNSIGNED_INTEGER) {
	        // 生成指令
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
        return {};
    }

    //    <function-call> ::=<identifier> '(' [<expression-list>] ')'
    //    <expression-list> ::=<expression>{','<expression>}
    std::optional<CompilationError> Analyser::analyseFunctionCall() {
        // 因为肯定是确认了Identifier和（ 才能进来，所以跳过
        auto next = nextToken();
        next = nextToken();
        auto err = analyseExpression();
        if (err.has_value())
            return err;
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
        }
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
        next = nextToken();
        type = next.value().GetType();
        if (!next.has_value() || type == TokenType::IDENTIFIER)
            return std::make_optional<CompilationError>(_current_pos,ErrNeedIdentifier);
        next = nextToken();
        type = next.value().GetType();
        if (!next.has_value() || type == TokenType::LEFT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos,ErrNoBracket);
        // 判断有无参数
        next = nextToken();
        type = next.value().GetType();
        if (type == TokenType::RIGHT_BRACKET)
            unreadToken();
        else {
            auto err = analyseParasList();
            if (err.has_value())
                return err;
        }
        next = nextToken();
        type = next.value().GetType();
        if (!next.has_value() || type == TokenType::RIGHT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos,ErrNoBracket);
        auto err = analyseCompoundStatement();
        if (err.has_value())
            return err;

        // 添加到函数表
        return {};
    }

    //    <parameter-declaration-list> ::=<parameter-declaration>{','<parameter-declaration>}
    //    <parameter-declaration> ::=[<const-qualifier>]<type-specifier><identifier>
    std::optional<CompilationError> Analyser::analyseParasList() {
        auto err = analyseParasDeclaration();
        if (err.has_value())
            return err;
        while (true) {
            auto next = nextToken();
            auto type = next.value().GetType();
            if (type == TokenType::COMMA_SIGN) {
                err = analyseParasDeclaration();
                if (err.has_value())
                    return err;
            }
            else
                break;
        }
        return {};
    }
    std::optional<CompilationError> Analyser::analyseParasDeclaration() {
        auto next = nextToken();
        auto type = next.value().GetType();
        if (!next.has_value())
            return std::make_optional<CompilationError>(_current_pos,ErrInvalidParameter);
        if (type != TokenType::CONST && type != TokenType::INT)
            return std::make_optional<CompilationError>(_current_pos,ErrInvalidParameter);
        if (type == TokenType::CONST) {
            // 常量表？
            next = nextToken();
            type = next.value().GetType();
            if (!next.has_value() || type != TokenType::INT)
                return std::make_optional<CompilationError>(_current_pos,ErrInvalidParameter);
            next = nextToken();
            if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
                return std::make_optional<CompilationError>(_current_pos,ErrInvalidParameter);
        }
        else {
            // 加入变量表
            if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
                return std::make_optional<CompilationError>(_current_pos,ErrInvalidParameter);
        }
        return {};
    }

    //    <compound-statement> ::='{' {<variable-declaration>} <statement-seq> '}'
    std::optional<CompilationError> Analyser::analyseCompoundStatement() {
        auto next = nextToken();
        auto type = next.value().GetType();
        if (!next.has_value() || type != TokenType::BIG_LEFT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos,ErrNoBigBracket);
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
        return {};
    }
    //    <statement-seq> ::={<statement>}
    std::optional<CompilationError> Analyser::analyseStatementSeq() {
        while (true) {
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
             auto err = analyseStatementSeq();
             if (err.has_value())
                 return err;
             next = nextToken();
             type = next.value().GetType();
             if (!next.has_value() || type != TokenType::BIG_RIGHT_BRACKET)
                 return std::make_optional<CompilationError>(_current_pos,ErrNoBigBracket);
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
                 auto err = analyseFunctionCall();
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
        if (type != TokenType::SEMICOLON) {
            auto err = analyseExpression();
            if (err.has_value())
                return err;
            next = nextToken();
        }
        if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
            return std::make_optional<CompilationError>(_current_pos,ErrNoSemicolon);
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
        if (type == TokenType::IDENTIFIER) {
            // xiang guan pan duan
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
        auto next = nextToken();
        next = nextToken();
        auto err = analyseExpression();
        if (err.has_value())
            return err;
        return {};
    }
}