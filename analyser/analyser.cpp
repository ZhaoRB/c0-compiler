#include "analyser.h"

#include <climits>
#include <sstream>

namespace miniplc0 {
	std::pair<std::vector<Instruction>, std::optional<CompilationError>> Analyser::Analyse() {
		auto err = analyseProgram();
		if (err.has_value())
			return std::make_pair(std::vector<Instruction>(), err);
		else
			return std::make_pair(_instructions, std::optional<CompilationError>());
	}

	// <程序> ::= 'begin'<主过程>'end'
	std::optional<CompilationError> Analyser::analyseProgram() {
		// 示例函数，示例如何调用子程序

		// 'begin'
		auto bg = nextToken();
		if (!bg.has_value() || bg.value().GetType() != TokenType::BEGIN)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoBegin);
		//std::cout << "begin is OK" << std::endl;

		// <主过程>
		auto err = analyseMain();
		if (err.has_value())
			return err;
        //std::cout << "main program is OK" << std::endl;

		// 'end'
		auto ed = nextToken();
		if (!ed.has_value() || ed.value().GetType() != TokenType::END)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoEnd);
        //std::cout << "end is OK" << std::endl;
		return {};
	}

	// <主过程> ::= <常量声明><变量声明><语句序列>
	// 需要补全
	std::optional<CompilationError> Analyser::analyseMain() {
		// 完全可以参照 <程序> 编写

		// <常量声明>
        auto errConstant = analyseConstantDeclaration();
        if (errConstant.has_value())
            return errConstant;
		// <变量声明>
        auto errVar = analyseVariableDeclaration();
        if (errVar.has_value())
            return errVar;
		// <语句序列>
        auto errStatement = analyseStatementSequence();
        if (errStatement.has_value())
            return errStatement;
		return {};
	}

	// <常量声明> ::= {<常量声明语句>}
	// <常量声明语句> ::= 'const'<标识符>'='<常表达式>';'
	std::optional<CompilationError> Analyser::analyseConstantDeclaration() {
		// 示例函数，示例如何分析常量声明

		// 常量声明语句可能有 0 或无数个
		while (true) {
			// 预读一个 token，不然不知道是否应该用 <常量声明> 推导
			auto next = nextToken();
            if (!next.has_value())
                return {};
			// 如果是 const 那么说明应该推导 <常量声明> 否则直接返回
			if (next.value().GetType() != TokenType::CONST) {
				unreadToken();
				return {};
			}

			// <常量声明语句>
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
			//zrb：是已经声明过的常量或者变量，如果再声明，返回错误
			if (isDeclared(next.value().GetValueString()))
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);
			//zrb：将声明的常量加入常量的符号表
			addConstant(next.value());

			// '='
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::EQUAL_SIGN)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrConstantNeedValue);

			// <常表达式>
			int32_t val;
			auto err = analyseConstantExpression(val);
			if (err.has_value())
				return err;

			// ';'
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
			// 生成一次 LIT 指令加载常量
			//zrb：LIT 将数据压栈
			//zrb：emplace_back在容器尾部添加一个元素
			_instructions.emplace_back(Operation::LIT, val);
		}
		return {};
	}

	// <变量声明> ::= {<变量声明语句>}
	// <变量声明语句> ::= 'var'<标识符>['='<表达式>]';'
	// 需要补全
	std::optional<CompilationError> Analyser::analyseVariableDeclaration() {
		// 变量声明语句可能有一个或者多个
		// 改成0个或多个

		while (true) {
            // 预读？
            auto next = nextToken();
            if (!next.has_value())
                return {};
            // 'var'
            if (next.value().GetType() != TokenType::VAR) {
                unreadToken();
                return {};
            }
            // <标识符>
            //zrb：std::optional<Token>
            next = nextToken();
            if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
            if (isDeclared(next.value().GetValueString()))
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);

            //加入未初始化的变量表 然后压栈一个0
            addUninitializedVariable(next.value());
            _instructions.emplace_back(LIT,0);
            // 用一个string将标识符的值存起来，STO的时候使用，index查找标识符在栈中的位置
            auto identifierToke = next.value();
            std::string identifier = next.value().GetValueString();
            int32_t index = getIndex(identifier);

            // 变量可能没有初始化，仍然需要一次预读
            // 如果什么都没有，或者既不是 ‘=’ 也不是 ‘；’，报错
            next = nextToken();
            if (!next.has_value() || (next.value().GetType() != SEMICOLON && next.value().GetType() != EQUAL_SIGN))
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);

            // ';'
            // 没初始化的变量，加入没有初始化的变量表，用 LIT 0 占位
            if (next.value().GetType() == TokenType::SEMICOLON)
                continue;
            // '='
            else if (next.value().GetType() == TokenType::EQUAL_SIGN) {
                // <表达式>
                // next = nextToken(); 因为会在子函数中调用nextToken() 所以这里不用
                auto errExpression = analyseExpression();
                if (errExpression.has_value())
                    return errExpression;
                // ;
                next = nextToken();
                if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);

                // 初始化后 将变量从未初始化的符号表中移动出来 添加到已经初始化的符号表中
                // STO赋值
                // 在analyseExpression()中添加 _instruction吗
                _instructions.emplace_back(Operation::STO,index);
                int32_t tmp = _nextTokenIndex;
                _nextTokenIndex = _uninitialized_vars[identifier];
                _uninitialized_vars.erase(identifier);
                addVariable(identifierToke);
                _nextTokenIndex = tmp;
            }
		}
		return {};
	}

	// <语句序列> ::= {<语句>}
	// <语句> :: = <赋值语句> | <输出语句> | <空语句>
	// <赋值语句> :: = <标识符>'='<表达式>';'
	// <输出语句> :: = 'print' '(' <表达式> ')' ';'
	// <空语句> :: = ';'
	// 需要补全
	std::optional<CompilationError> Analyser::analyseStatementSequence() {
		while (true) {
			// 预读
			auto next = nextToken();
			if (!next.has_value())
				return {};
			unreadToken();
			if (next.value().GetType() != TokenType::IDENTIFIER &&
				next.value().GetType() != TokenType::PRINT &&
				next.value().GetType() != TokenType::SEMICOLON) {
				return {};
			}
			// zrb: 这里为什么在这写err才行？？？？？
			std::optional<CompilationError> err;
			// 因为在调用的赋值语句和输出语句中会 nextToken() 所以不需要在这里nextToken
			//next = nextToken();
			switch (next.value().GetType()) {
				// 这里需要你针对不同的预读结果来调用不同的子程序
				// 注意我们没有针对空语句单独声明一个函数，因此可以直接在这里返回
			    case IDENTIFIER:
			        err = analyseAssignmentStatement();
			        if (err.has_value())
                        return err;
			        break;
			    case PRINT:
                    err = analyseOutputStatement();
                    if (err.has_value())
                        return err;
                    break;
                // 如果是分号，指向分号，进入下一次循环
			    case SEMICOLON:
                    next = nextToken();
                    break;
			    default:
				    break;
			}
		}
		return {};
	}

	// <常表达式> ::= [<符号>]<无符号整数>
	// 需要补全
	std::optional<CompilationError> Analyser::analyseConstantExpression(int32_t& out) {
		// out 是常表达式的结果
		// 这里你要分析常表达式并且计算结果
		// 注意以下均为常表达式
		// +1 -1 1
		// 同时要注意是否溢出

		auto next = nextToken();  //是optional<Token>类型，应该是合法的无符号整数 或者是 ‘+’ ‘-’
		TokenType type = next.value().GetType();

		// 如果<常表达式>没有值，或者不是加号，也不是减号，也不是无符号整数
		// 这里应该不需要再判断是不是无符号整数了吧  tokenizer里面已经判断了
		if (!next.has_value() || (type != TokenType::PLUS_SIGN && type != TokenType::MINUS_SIGN && type != TokenType::UNSIGNED_INTEGER))
		    return std::make_optional<CompilationError>(_current_pos,ErrConstantNeedValue);
		// 如果是加号或者减号，再读入一个token
		if (type == TokenType::PLUS_SIGN || type == TokenType::MINUS_SIGN) {
		    next = nextToken();
		    if (!next.has_value() || next.value().GetType() != TokenType::UNSIGNED_INTEGER)
		        return std::make_optional<CompilationError>(_current_pos,ErrConstantNeedValue);
		}
		// 如果是无符号整数，直接到token的处理
        // 将Token类型转换
        std::string s = next.value().GetValueString();
        std::stringstream ss;
        ss.str(s);
        ss >> out;
        if (type == TokenType::MINUS_SIGN)
            out *= -1;
		return {};
	}

	// <表达式> ::= <项>{<加法型运算符><项>}
	std::optional<CompilationError> Analyser::analyseExpression() {
		// <项>
		auto err = analyseItem();
		if (err.has_value())
			return err;

		// {<加法型运算符><项>}
		while (true) {
			// 预读
			auto next = nextToken();
			if (!next.has_value())
				return {};
			auto type = next.value().GetType();
			if (type != TokenType::PLUS_SIGN && type != TokenType::MINUS_SIGN) {
				unreadToken();
				return {};
			}

			// <项>
			err = analyseItem();
			if (err.has_value())
				return err;

			// 根据结果生成指令
			if (type == TokenType::PLUS_SIGN)
				_instructions.emplace_back(Operation::ADD, 0);
			else
				_instructions.emplace_back(Operation::SUB, 0);
		}
		return {};
	}

	// <赋值语句> ::= <标识符>'='<表达式>';'
	// 需要补全
	std::optional<CompilationError> Analyser::analyseAssignmentStatement() {
		// 这里除了语法分析以外还要留意
		// 标识符声明过吗？
		// 标识符是常量吗？
		// 需要生成指令吗？
		auto next = nextToken();
		auto token = next.value();
		auto str = next.value().GetValueString();

		// 读到文件结束 或者不是标识符
		if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
            return std::make_optional<CompilationError>(_current_pos,ErrNeedIdentifier);
		// 是标识符
		// 没有声明过
        if (!isDeclared(str))
            return std::make_optional<CompilationError>(_current_pos,ErrInvalidIdentifier);
        // 声明过 但是是常量
		if (isConstant(str))
		    return std::make_optional<CompilationError>(_current_pos,ErrInvalidIdentifier);

		// 声明过，而且是变量，合法的标识符
		// '='
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != EQUAL_SIGN)
		    return std::make_optional<CompilationError>(_current_pos,ErrNoEqualSign);
		// <表达式>，这时候值应该已经在栈顶了
		auto errExpression = analyseExpression();
		if (errExpression.has_value())
		    return errExpression;
		// ';'
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != SEMICOLON)
		    return std::make_optional<CompilationError>(_current_pos,ErrNoSemicolon);
		// 生成赋值指令
		// 变量换符号表
        // 生成STO指令 赋值， 先用<表达式>将要赋的值放到栈顶，然后STO 将栈顶的值放到栈中变量的位置，栈顶元素出栈
        int32_t x = getIndex(str);
        _instructions.emplace_back(Operation::STO,x);
        // 如果是第一次赋值的话，变换符号表
        if (isUninitializedVariable(str)) {
            int32_t tmp = _nextTokenIndex;
            _nextTokenIndex = x;
            addVariable(token);
            _nextTokenIndex = tmp;
            _uninitialized_vars.erase(str);
        }
		return {};
	}

	// <输出语句> ::= 'print' '(' <表达式> ')' ';'
	std::optional<CompilationError> Analyser::analyseOutputStatement() {
		// 如果之前 <语句序列> 的实现正确，这里第一个 next 一定是 TokenType::PRINT
		auto next = nextToken();

		// '('
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidPrint);

		// <表达式>
		auto err = analyseExpression();
		if (err.has_value())
			return err;

		// ')'
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidPrint);

		// ';'
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);

		// 生成相应的指令 WRT
		_instructions.emplace_back(Operation::WRT, 0);
		return {};
	}

	// <项> :: = <因子>{ <乘法型运算符><因子> }
	// 需要补全
	std::optional<CompilationError> Analyser::analyseItem() {
		// 可以参考 <表达式> 实现

		// <因子>
		auto errFactor = analyseFactor();
		if (errFactor.has_value())
		    return errFactor;
		// { <乘法型运算符><因子> }
		while (true) {
		    //预读
		    auto next = nextToken();
		    if (!next.has_value())
                return {};
		    auto type = next.value().GetType();
		    if (type != TokenType::MULTIPLICATION_SIGN && type != TokenType::DIVISION_SIGN) {
		        unreadToken();
		        return {};
		    }
		    // 因子
		    errFactor = analyseFactor();
		    if (errFactor.has_value())
                return errFactor;

		    // 生成指令
		    if (type == TokenType::MULTIPLICATION_SIGN)
		        _instructions.emplace_back(Operation::MUL,0);
		    if (type == TokenType::DIVISION_SIGN)
                _instructions.emplace_back(Operation::DIV, 0);
		}
		return {};
	}

	// <因子> ::= [<符号>]( <标识符> | <无符号整数> | '('<表达式>')' )
	// 需要补全
	std::optional<CompilationError> Analyser::analyseFactor() {
		// [<符号>]
            auto next = nextToken();
        auto prefix = 1;
        if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
		if (next.value().GetType() == TokenType::PLUS_SIGN)
			prefix = 1;
		else if (next.value().GetType() == TokenType::MINUS_SIGN) {
			prefix = -1;
			_instructions.emplace_back(Operation::LIT, 0);
		}
		else
			unreadToken();

		// 预读
		next = nextToken();
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);

		std::string s = next.value().GetValueString();
		std::stringstream ss;
		ss.str(s);
		std::optional<CompilationError> errExpression;
		int32_t x;
		switch (next.value().GetType()) {
			// 这里和 <语句序列> 类似，需要根据预读结果调用不同的子程序
			// 但是要注意 default 返回的是一个编译错误

			//标识符看符号表里面有没有 有没有初始化
		    case TokenType::IDENTIFIER :
		        // 没定义过
		        if (!isDeclared(s))
                    return std::make_optional<CompilationError>(_current_pos,ErrInvalidIdentifier);
		        // 没初始化过
		        if (isUninitializedVariable(s))
		            return std::make_optional<CompilationError>(_current_pos,ErrNotInitialized);
		        // 如果是标识符，LOD将标识符的值加载到栈顶
		        x = getIndex(s);
		        _instructions.emplace_back(Operation::LOD,x);
                break;
            //无符号整数看是否越界
            case TokenType::UNSIGNED_INTEGER :
                // 如果是无符号整数，要将值放到栈顶
                int32_t a;
                ss >> a;
                _instructions.emplace_back(Operation::LIT,a);
                break;
            //左括号
            case TokenType::LEFT_BRACKET :
                errExpression = analyseExpression();
                if (errExpression.has_value())
                    return errExpression;
                next = nextToken();
                if (!next.has_value() || next.value().GetType() != RIGHT_BRACKET)
                    return std::make_optional<CompilationError>(_current_pos,ErrIncompleteExpression);
                break;
            default:
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
		}

		// 取负
		if (prefix == -1)
			_instructions.emplace_back(Operation::SUB, 0);
		return {};
	}

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

	void Analyser::_add(const Token& tk, std::map<std::string, int32_t>& mp) {
		if (tk.GetType() != TokenType::IDENTIFIER)
			DieAndPrint("only identifier can be added to the table.");
		mp[tk.GetValueString()] = _nextTokenIndex;
		_nextTokenIndex++;
	}

	void Analyser::addVariable(const Token& tk) {
		_add(tk, _vars);
	}

	void Analyser::addConstant(const Token& tk) {
		_add(tk, _consts);
	}

	void Analyser::addUninitializedVariable(const Token& tk) {
		_add(tk, _uninitialized_vars);
	}

	int32_t Analyser::getIndex(const std::string& s) {
		if (_uninitialized_vars.find(s) != _uninitialized_vars.end())
			return _uninitialized_vars[s];
		else if (_vars.find(s) != _vars.end())
			return _vars[s];
		else
			return _consts[s];
	}

	bool Analyser::isDeclared(const std::string& s) {
		return isConstant(s) || isUninitializedVariable(s) || isInitializedVariable(s);
	}

	bool Analyser::isUninitializedVariable(const std::string& s) {
		return _uninitialized_vars.find(s) != _uninitialized_vars.end();
	}
	bool Analyser::isInitializedVariable(const std::string&s) {
		return _vars.find(s) != _vars.end();
	}

	bool Analyser::isConstant(const std::string&s) {
		return _consts.find(s) != _consts.end();
	}
}