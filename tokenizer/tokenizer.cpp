#include "tokenizer/tokenizer.h"

#include <cctype>
#include <sstream>

namespace miniplc0 {

	std::pair<std::optional<Token>, std::optional<CompilationError>> Tokenizer::NextToken() {
		if (!_initialized)
			readAll();
		if (_rdr.bad())
			return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrorCode::ErrStreamError));
		if (isEOF())
			return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrorCode::ErrEOF));
		auto p = nextToken();
		if (p.second.has_value())
			return std::make_pair(p.first, p.second);
		auto err = checkToken(p.first.value());
		if (err.has_value())
			return std::make_pair(p.first, err.value());
		return std::make_pair(p.first, std::optional<CompilationError>());
	}

	std::pair<std::vector<Token>, std::optional<CompilationError>> Tokenizer::AllTokens() {
		std::vector<Token> result;
		while (true) {
			auto p = NextToken();
			if (p.second.has_value()) {
				if (p.second.value().GetCode() == ErrorCode::ErrEOF)
					return std::make_pair(result, std::optional<CompilationError>());
				else
					return std::make_pair(std::vector<Token>(), p.second);
			}
			result.emplace_back(p.first.value());
		}
	}

	//补全这个nextToken函数

	// 注意：这里的返回值中 Token 和 CompilationError 只能返回一个，不能同时返回。
	std::pair<std::optional<Token>, std::optional<CompilationError>> Tokenizer::nextToken() {
		// 用于存储已经读到的组成当前token字符
		std::stringstream ss;
		// 分析token的结果，作为此函数的返回值
		std::pair<std::optional<Token>, std::optional<CompilationError>> result;
		// <行号，列号>，表示当前token的第一个字符在源代码中的位置
		std::pair<int64_t, int64_t> pos;
		// 记录当前自动机的状态，进入此函数时是初始状态
		DFAState current_state = DFAState::INITIAL_STATE;
		// 这是一个死循环，除非主动跳出
		// 每一次执行while内的代码，都可能导致状态的变更
		while (true) {
			// 读一个字符，请注意auto推导得出的类型是std::optional<char>
			// 这里其实有两种写法
			// 1. 每次循环前立即读入一个 char
			// 2. 只有在可能会转移的状态读入一个 char
			// 因为我们实现了 unread，为了省事我们选择第一种
			auto current_char = nextChar();
			// 针对当前的状态进行不同的操作
			switch (current_state) {

				// 初始状态
				// 这个 case 我们给出了核心逻辑，但是后面的 case 不用照搬。
			case INITIAL_STATE: {
				// 已经读到了文件尾
				if (!current_char.has_value())
					// 返回一个空的token，和编译错误ErrEOF：遇到了文件尾
					return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrEOF));

				// 获取读到的字符的值，注意auto推导出的类型是char
				auto ch = current_char.value();
				// 标记是否读到了不合法的字符，初始化为否
				auto invalid = false;

				// 使用了自己封装的判断字符类型的函数，定义于 tokenizer/utils.hpp
				// see https://en.cppreference.com/w/cpp/string/byte/isblank
				if (miniplc0::isspace(ch)) // 读到的字符是空白字符（空格、换行、制表符等）
					current_state = DFAState::INITIAL_STATE; // 保留当前状态为初始状态，此处直接break也是可以的
				else if (!miniplc0::isprint(ch)) // control codes and backspace,其他不合法字符
					invalid = true;
				// 读到的字符是数字
				else if (miniplc0::isdigit(ch)) {
				    if (ch == '0')
				        current_state = DFAState::UNSIGNED_INTEGER_STATE;
				    // 是 1-9 的话肯定是十进制
				    else
				        current_state = DFAState::DECIMAL_UNSIGNED_INTEGER_STATE;
                }
				else if (miniplc0::isalpha(ch)) // 读到的字符是英文字母
					current_state = DFAState::IDENTIFIER_STATE; // 切换到标识符的状态
				else {
					switch (ch) {
					case '=':
						current_state = DFAState::EQUAL_SIGN_STATE;
						break;
					case '-':
						current_state = DFAState::MINUS_SIGN_STATE;
						break;
					case '+':
						current_state = DFAState::PLUS_SIGN_STATE;
						break;
					case '*':
						current_state = DFAState::MULTIPLICATION_SIGN_STATE;
						break;
					case '/':
						current_state = DFAState::DIVISION_SIGN_STATE;
						break;
                    case ';':
                        current_state = DFAState::SEMICOLON_STATE;
                        break;
                    case '(':
                        current_state = DFAState::LEFTBRACKET_STATE;
                        break;
                    case ')':
                        current_state = DFAState::RIGHTBRACKET_STATE;
                        break;
                    case '<':
                        current_state = DFAState::LESSTHAN_SIGN_STATE;
                        break;
                    case '>':
                        current_state = DFAState::MORETHAN_SIGN_STATE;
                        break;
                    case '{':
                        current_state = DFAState::BIG_LEFTBRACKET_STATE;
                        break;
                    case '}':
                        current_state = DFAState::BIG_RIGHTBRACKET_STATE;
                        break;
                    case '!':
                        current_state = DFAState::MAYBE_NOTEQUAL_SIGN_STATE;
                        break;
                    case '\'':
                        current_state = DFAState::SINGLE_QUOTATION_MARKS_STATE;
                        break;
                    case '\"':
                        current_state = DFAState::DOUBLE_QUOTATION_MARKS_STATE;
                        break;
                    case ',':
                        current_state = DFAState::COMMA_SIGN_STATE;
                        break;
					// 不接受的字符导致的不合法的状态
					default:
						invalid = true;
						break;
					}
				}
				// 如果读到的字符导致了状态的转移，说明它是一个token的第一个字符
				if (current_state != DFAState::INITIAL_STATE)
					pos = previousPos(); // 记录该字符的的位置为token的开始位置
				// 读到了不合法的字符
				if (invalid) {
					// 回退这个字符
					unreadLast();
					// 返回编译错误：非法的输入
					return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrInvalidInput));
				}
				// 如果读到的字符导致了状态的转移，说明它是一个token的第一个字符
				if (current_state != DFAState::INITIAL_STATE) // ignore white spaces
					ss << ch; // 存储读到的字符
				break;
			}

            case UNSIGNED_INTEGER_STATE: {
                if (!current_char.has_value()) {
                    return std::make_pair(std::make_optional<Token>(TokenType::DECIMAL_UNSIGNED_INTEGER,0,pos,currentPos()),std::optional<CompilationError>());
                }
                else {
                    char ch = current_char.value();
                    if (ch == 'x' || ch == 'X') {
                        ss << ch;
                        current_state = DFAState::HEXADECIMAL_UNSIGNED_INTEGER_STATE;
                    }
                        // 要不要回退？
                    else if (isdigit(ch))
                        return std::make_pair(std::optional<Token>(),std::make_optional<CompilationError>(pos,ErrorCode::ErrInvalidInput));
                    else {
                        unreadLast();
                        current_state = DFAState::DECIMAL_UNSIGNED_INTEGER_STATE;
                    }
                }
                break;
            }
			// 当前状态是十进制无符号整数
			case DECIMAL_UNSIGNED_INTEGER_STATE: {
				// 如果当前已经读到了文件尾，则解析已经读到的字符串为整数
				//     解析成功则返回无符号整数类型的token，否则返回编译错误
                if (!current_char.has_value()) {
                    int a;
                    //解析成功
                    if ( ss >> a ) {
                        auto returnValue = std::make_optional<Token>(TokenType::DECIMAL_UNSIGNED_INTEGER ,a,pos,previousPos());
                        result.first = returnValue;
                    }
                    else
                        result.second = std::make_optional<CompilationError>(pos,ErrorCode::ErrIntegerOverflow);
                    return result;
                }
                // 如果读到的字符是数字，则存储读到的字符
                // 如果读到的字符不是上述情况之一，则回退读到的字符，并解析已经读到的字符串为整数
                // 解析成功则返回无符号整数类型的token，否则返回编译错误
                else {
                    auto ch = current_char.value();
                    if (miniplc0::isdigit(ch)) {
                        ss << ch;
                    }
                    else {
                        unreadLast();
                        int a;
                        if ( ss >> a )
                            result.first = std::make_optional<Token>(TokenType::DECIMAL_UNSIGNED_INTEGER,a,pos,previousPos());
                        else
                            result.second = std::make_optional<CompilationError>(pos,ErrorCode::ErrIntegerOverflow);
                        return result;
                    }
                }
				break;
			}
			// 当前状态是十六进制无符号整数
            case HEXADECIMAL_UNSIGNED_INTEGER_STATE: {
                std::string s;
                if (!current_char.has_value()) {
                    // int a;
                    ss >> s;
                    long long ans = convertToDecimal(s);
                    //解析成功
                    if ( ans >= -2147483648 && ans <= 0x7fffffff ) {
                        int a = (int)ans;
                        auto returnValue = std::make_optional<Token>(TokenType::HEXADECIMAL_UNSIGNED_INTEGER ,a,pos,previousPos());
                        result.first = returnValue;
                    }
                    else
                        result.second = std::make_optional<CompilationError>(pos,ErrorCode::ErrIntegerOverflow);
                    return result;
                }
                else {
                    auto ch = current_char.value();
                    if (miniplc0::isdigit(ch)) {
                        ss << ch;
                    }
                    else if ((ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F')) {
                        ss << ch;
                    }
                    else {
                        unreadLast();
                        // int a;
                        ss >> s;
                        long long ans = convertToDecimal(s);
                        if ( ans >= -2147483648 && ans <= 0x7fffffff ) {
                            // a = (int)ans;
                            int a = (int)ans;
                            auto returnValue = std::make_optional<Token>(TokenType::HEXADECIMAL_UNSIGNED_INTEGER ,a,pos,previousPos());
                            result.first = returnValue;
                        }
                        else
                            result.second = std::make_optional<CompilationError>(pos,ErrorCode::ErrIntegerOverflow);
                        return result;
                    }
                }
                break;
            }

			case IDENTIFIER_STATE: {
				// 如果当前已经读到了文件尾，则解析已经读到的字符串
				//     如果解析结果是关键字，那么返回对应关键字的token，否则返回标识符的token
				if (!current_char.has_value()) {
				    std::string s;
				    ss >> s;
				    TokenType tmp;
				    if ( s == "const")
				        tmp = TokenType::CONST;
				    else if ( s == "void" )
				        tmp = TokenType::VOID;
				    else if (s == "int" )
				        tmp = TokenType::INT;
				    else if (s == "char" )
				        tmp = TokenType::CHAR;
				    else if (s == "double")
				        tmp = TokenType::DOUBLE;
                    else if ( s == "struct" )
                        tmp = TokenType::STRUCT;
                    else if (s == "if" )
                        tmp = TokenType::IF;
                    else if (s == "else" )
                        tmp = TokenType::ELSE;
                    else if (s == "switch")
                        tmp = TokenType::SWITCH;
                    else if ( s == "case" )
                        tmp = TokenType::CASE;
                    else if (s == "default" )
                        tmp = TokenType::DEFAULT;
                    else if (s == "while" )
                        tmp = TokenType::WHILE;
                    else if (s == "for")
                        tmp = TokenType::FOR;
                    else if ( s == "do" )
                        tmp = TokenType::DO;
                    else if (s == "return" )
                        tmp = TokenType::RETURN;
                    else if (s == "break" )
                        tmp = TokenType::BREAK;
                    else if (s == "continue")
                        tmp = TokenType::CONTINUE;
                    else if (s == "print")
                        tmp = TokenType::PRINT;
                    else if (s == "scan")
                        tmp = TokenType::SCAN;
				    else
				        tmp = TokenType::IDENTIFIER;
				    result.first = std::make_optional<Token>(tmp,s,pos,previousPos());
				    return result;
				}
				// 如果读到的是字符或字母，则存储读到的字符
				// 如果读到的字符不是上述情况之一，则回退读到的字符，并解析已经读到的字符串
				//     如果解析结果是关键字，那么返回对应关键字的token，否则返回标识符的token
				else {
                    auto ch = current_char.value();
                    if (miniplc0::isalpha(ch) || miniplc0::isdigit(ch)) {
                        ss << ch;
                    }
                    else {
                        unreadLast();
                        std::string s;
                        ss >> s;
                        TokenType tmp;
                        if ( s == "const")
                            tmp = TokenType::CONST;
                        else if ( s == "void" )
                            tmp = TokenType::VOID;
                        else if (s == "int" )
                            tmp = TokenType::INT;
                        else if (s == "char" )
                            tmp = TokenType::CHAR;
                        else if (s == "double")
                            tmp = TokenType::DOUBLE;
                        else if ( s == "struct" )
                            tmp = TokenType::STRUCT;
                        else if (s == "if" )
                            tmp = TokenType::IF;
                        else if (s == "else" )
                            tmp = TokenType::ELSE;
                        else if (s == "switch")
                            tmp = TokenType::SWITCH;
                        else if ( s == "case" )
                            tmp = TokenType::CASE;
                        else if (s == "default" )
                            tmp = TokenType::DEFAULT;
                        else if (s == "while" )
                            tmp = TokenType::WHILE;
                        else if (s == "for")
                            tmp = TokenType::FOR;
                        else if ( s == "do" )
                            tmp = TokenType::DO;
                        else if (s == "return" )
                            tmp = TokenType::RETURN;
                        else if (s == "break" )
                            tmp = TokenType::BREAK;
                        else if (s == "continue")
                            tmp = TokenType::CONTINUE;
                        else if (s == "print")
                            tmp = TokenType::PRINT;
                        else if (s == "scan")
                            tmp = TokenType::SCAN;
                        else
                            tmp = TokenType::IDENTIFIER;
                        result.first = std::make_optional<Token>(tmp,s,pos,previousPos());
                        return result;
                    }
				}
				break;
			}

			// 如果当前状态是加号
			case PLUS_SIGN_STATE: {
				// 回退
				char ch = current_char.value();
//				if (miniplc0::isdigit(ch)) {
//				    if (ch == '0') {
//				        ss << ch;
//                        current_state = DFAState::UNSIGNED_INTEGER_STATE;
//				    }
//                    else {
//                        ss << ch;
//                        current_state = DFAState::DECIMAL_UNSIGNED_INTEGER_STATE;
//                    }
//                    break;
//				}
				unreadLast(); // Yes, we unread last char even if it's an EOF.
				return std::make_pair(std::make_optional<Token>(TokenType::PLUS_SIGN, '+', pos, currentPos()), std::optional<CompilationError>());
			}
			// 当前状态为减号的状态
			case MINUS_SIGN_STATE: {
				// 回退，并返回减号token
                char ch = current_char.value();
//                if (miniplc0::isdigit(ch)) {
//                    if (ch == '0') {
//                        ss << ch;
//                        current_state = DFAState::UNSIGNED_INTEGER_STATE;
//                    }
//                    else {
//                        ss << ch;
//                        current_state = DFAState::DECIMAL_UNSIGNED_INTEGER_STATE;
//                    }
//                    break;
//                }
                unreadLast(); // Yes, we unread last char even if it's an EOF.
                return std::make_pair(std::make_optional<Token>(TokenType::MINUS_SIGN, '-', pos, currentPos()), std::optional<CompilationError>());
			}
            case DIVISION_SIGN_STATE: {
                char c = current_char.value();
                if (c == '/') {
                    char end = nextChar().value();
                    while (true) {
                        if (end == 0x0a || end == 0x0d)
                            break;
                        end = nextChar().value();
                    }
                    current_state = DFAState::INITIAL_STATE;
                    ss >> c;
                    break;
                }
                else if (c == '*') {
                    char end1 = nextChar().value();
                    char end2 = nextChar().value();
                    while (true) {
                        if (end1 == '*' && end2 == '/')
                            break;
                        end1 = end2;
                        auto endzrb = nextChar();
                        // 到文件尾了
                        if (!endzrb.has_value()) {
                            return std::make_pair(std::optional<Token>(),std::make_optional<CompilationError>(pos,ErrorCode::ErrComent));
                        }
                        end2 = endzrb.value();
                    }
                    current_state = DFAState::INITIAL_STATE;
                    ss >> c;
                    break;
                }
                else {
                    unreadLast();
                    return std::make_pair(std::make_optional<Token>(TokenType::DIVISION_SIGN, '/', pos, currentPos()), std::optional<CompilationError>());
                }
            }
            case MULTIPLICATION_SIGN_STATE: {
                unreadLast();
                return std::make_pair(std::make_optional<Token>(TokenType::MULTIPLICATION_SIGN, '*', pos, currentPos()), std::optional<CompilationError>());
            }
            case EQUAL_SIGN_STATE: {
                char ch = current_char.value();
                if (ch == '=')
                    current_state = DFAState::ISEQUAL_SIGN_STATE;
                else {
                    unreadLast();
                    return std::make_pair(std::make_optional<Token>(TokenType::EQUAL_SIGN, '=', pos, currentPos()),
                                          std::optional<CompilationError>());
                }
                break;
            }
            case ISEQUAL_SIGN_STATE: {
                unreadLast();
                return std::make_pair(std::make_optional<Token>(TokenType::IS_EQUAL_SIGN, "==", pos, currentPos()),
                                      std::optional<CompilationError>());
            }
            case SEMICOLON_STATE: {
                unreadLast();
                return std::make_pair(std::make_optional<Token>(TokenType::SEMICOLON, ';', pos, currentPos()), std::optional<CompilationError>());
            }
            case LEFTBRACKET_STATE: {
                unreadLast();
                return std::make_pair(std::make_optional<Token>(TokenType::LEFT_BRACKET, '(', pos, currentPos()), std::optional<CompilationError>());
            }
            case RIGHTBRACKET_STATE: {
                unreadLast();
                return std::make_pair(std::make_optional<Token>(TokenType::RIGHT_BRACKET, ')', pos, currentPos()), std::optional<CompilationError>());
            }
            case BIG_LEFTBRACKET_STATE: {
                unreadLast();
                return std::make_pair(std::make_optional<Token>(TokenType::BIG_LEFT_BRACKET,'{',pos,currentPos()),std::optional<CompilationError>());
            }
            case BIG_RIGHTBRACKET_STATE: {
                unreadLast();
                return std::make_pair(std::make_optional<Token>(TokenType::BIG_RIGHT_BRACKET,'}',pos,currentPos()),std::optional<CompilationError>());
            }
            // <
            case LESSTHAN_SIGN_STATE: {
                char ch = current_char.value();
                if (ch == '=')
                    current_state = DFAState::LESSTHAN_AND_EQUAL_SIGN_STATE;
                else {
                    unreadLast();
                    return std::make_pair(std::make_optional<Token>(TokenType::LESS_THAN_SIGN,'<',pos,currentPos()),std::optional<CompilationError>());
                }
                break;
            }
            // <=
            case LESSTHAN_AND_EQUAL_SIGN_STATE: {
                unreadLast();
                return std::make_pair(std::make_optional<Token>(TokenType::LESS_OR_EQUAL_SIGN,"<=",pos,currentPos()),std::optional<CompilationError>());
            }
            // >
            case MORETHAN_SIGN_STATE: {
                char ch = current_char.value();
                if (ch == '=')
                    current_state = DFAState::MORETHAN_AND_EQUAL_SIGN_STATE;
                else {
                    unreadLast();
                    return std::make_pair(std::make_optional<Token>(TokenType::MORE_THAN_SIGN,'>',pos,currentPos()),std::optional<CompilationError>());
                }
                break;
            }
            // >=
            case MORETHAN_AND_EQUAL_SIGN_STATE: {
                unreadLast();
                return std::make_pair(std::make_optional<Token>(TokenType::MORE_OR_EQUAL_SIGN,">=",pos,currentPos()),std::optional<CompilationError>());
            }
            // 可能是 !=
            case MAYBE_NOTEQUAL_SIGN_STATE: {
                char ch = current_char.value();
                if (ch == '=')
                    current_state = DFAState::NOTEQUAL_SIGN_STATE;
                else
                    return std::make_pair(std::optional<Token>(),std::make_optional<CompilationError>(pos,ErrorCode::ErrInvalidInput));
                break;
            }
            // !=
            case NOTEQUAL_SIGN_STATE: {
                unreadLast();
                return std::make_pair(std::make_optional<Token>(TokenType::NOT_EQUAL_SIGN,"!=",pos,currentPos()),std::optional<CompilationError>());
            }
            // '
            case SINGLE_QUOTATION_MARKS_STATE: {
                unreadLast();
                return std::make_pair(std::make_optional<Token>(TokenType::SINGLE_QUOTATION_MARKS,'\'',pos,currentPos()),std::optional<CompilationError>());
            }
            // "
            case DOUBLE_QUOTATION_MARKS_STATE: {
                unreadLast();
                return std::make_pair(std::make_optional<Token>(TokenType::DOUBLE_QUOTATION_MARKS,'\"',pos,currentPos()),std::optional<CompilationError>());
            }
            // ,
            case COMMA_SIGN_STATE: {
                unreadLast();
                return std::make_pair(std::make_optional<Token>(TokenType::COMMA_SIGN,',',pos,currentPos()),std::optional<CompilationError>());
            }
            // 预料之外的状态，如果执行到了这里，说明程序异常
			default:
				DieAndPrint("unhandled state.");
				break;
			}
		}
		// 预料之外的状态，如果执行到了这里，说明程序异常
		return std::make_pair(std::optional<Token>(), std::optional<CompilationError>());
	}

	std::optional<CompilationError> Tokenizer::checkToken(const Token& t) {
		switch (t.GetType()) {
			case IDENTIFIER: {
				auto val = t.GetValueString();
				if (miniplc0::isdigit(val[0]))
					return std::make_optional<CompilationError>(t.GetStartPos().first, t.GetStartPos().second, ErrorCode::ErrInvalidIdentifier);
				break;
			}
            default:
                break;
		}
		return {};
	}

	void Tokenizer::readAll() {
		if (_initialized)
			return;
		for (std::string tp; std::getline(_rdr, tp);)
			_lines_buffer.emplace_back(std::move(tp + "\n"));
		_initialized = true;
		_ptr = std::make_pair<int64_t, int64_t>(0, 0);
		return;
	}

	// Note: We allow this function to return a postion which is out of bound according to the design like std::vector::end().
	std::pair<uint64_t, uint64_t> Tokenizer::nextPos() {
		if (_ptr.first >= _lines_buffer.size())
			DieAndPrint("advance after EOF");
		if (_ptr.second == _lines_buffer[_ptr.first].size() - 1)
			return std::make_pair(_ptr.first + 1, 0);
		else
			return std::make_pair(_ptr.first, _ptr.second + 1);
	}

	std::pair<uint64_t, uint64_t> Tokenizer::currentPos() {
		return _ptr;
	}

	std::pair<uint64_t, uint64_t> Tokenizer::previousPos() {
		if (_ptr.first == 0 && _ptr.second == 0)
			DieAndPrint("previous position from beginning");
		if (_ptr.second == 0)
			return std::make_pair(_ptr.first - 1, _lines_buffer[_ptr.first - 1].size() - 1);
		else
			return std::make_pair(_ptr.first, _ptr.second - 1);
	}

	std::optional<char> Tokenizer::nextChar() {
		if (isEOF())
			return {}; // EOF
		auto result = _lines_buffer[_ptr.first][_ptr.second];
		_ptr = nextPos();
		return result;
	}

	bool Tokenizer::isEOF() {
		return _ptr.first >= _lines_buffer.size();
	}

	// Note: Is it evil to unread a buffer?
	void Tokenizer::unreadLast() {
		_ptr = previousPos();
	}

	// 处理十六进制数
    long long Tokenizer::convertToDecimal(std::string s) {
        int n = s.size();
        long long ans = 0;
        int i;
        if (s[0] == '+' || s[0] == '-')
            i = 3;
        else
            i = 2;
        for (; i < n ; i++ ) {
            long long temp;
            if (s[i] >= '0' && s[i] <= '9')
                temp = s[i]-'0';
            else if (s[i] >= 'a' && s[i] <= 'f')
                temp = s[i] - 'a' + 10;
            else
                temp = s[i] - 'A' + 10;
            ans = ans * 16 + temp;
        }
        if (s[0] == '-')
            ans *= -1;
        return ans;
    }
}