#include "fmt/core.h"
#include "tokenizer/tokenizer.h"
#include "analyser/analyser.h"

// Error
namespace fmt {
	template<>
	struct formatter<miniplc0::ErrorCode> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const miniplc0::ErrorCode &p, FormatContext &ctx) {
			std::string name;
			switch (p) {
			case miniplc0::ErrNoError:
				name = "No error.";
				break;
			case miniplc0::ErrStreamError:
				name = "Stream error.";
				break;
			case miniplc0::ErrEOF:
				name = "EOF";
				break;
			case miniplc0::ErrInvalidInput:
				name = "The input is invalid.";
				break;
			case miniplc0::ErrInvalidIdentifier:
				name = "Identifier is invalid";
				break;
			case miniplc0::ErrIntegerOverflow:
				name = "The integer is too big(int64_t).";
				break;
			case miniplc0::ErrNeedIdentifier:
				name = "Need an identifier here.";
				break;
			case miniplc0::ErrConstantNeedValue:
				name = "The constant need a value to initialize.";
				break;
			case miniplc0::ErrNoSemicolon:
				name = "Zai? Wei shen me bu xie fen hao.";
				break;
			case miniplc0::ErrInvalidVariableDeclaration:
				name = "The declaration is invalid.";
				break;
			case miniplc0::ErrIncompleteExpression:
				name = "The expression is incomplete.";
				break;
			case miniplc0::ErrNotDeclared:
				name = "The variable or constant must be declared before being used.";
				break;
			case miniplc0::ErrAssignToConstant:
				name = "Trying to assign value to a constant.";
				break;
			case miniplc0::ErrDuplicateDeclaration:
				name = "The variable or constant has been declared.";
				break;
			case miniplc0::ErrNotInitialized:
				name = "The variable has not been initialized.";
				break;
			case miniplc0::ErrInvalidAssignment:
				name = "The assignment statement is invalid.";
				break;
			case miniplc0::ErrInvalidPrint:
				name = "The output statement is invalid.";
				break;
            case miniplc0::ErrNoEqualSign:
                name = "Need an equal sign here";
                break;
            case miniplc0::ErrNoTypeSpecifier:
                name = "Need a type specifier here";
                break;
            case miniplc0::ErrInvalidParameter:
                name = "can not be a parameter here";
                break;
            case miniplc0::ErrNoBigBracket:
                name = "Need left or right big bracket here";
                break;
            case miniplc0::ErrUnexpected:
                name = "An unexpected error";
                break;
            case miniplc0::ErrInvalidStatement:
                name = "this statement is invalid";
                break;
            case miniplc0::ErrHasDeclared:
                name = "the identifier has been declared";
                break;
            case miniplc0::ErrUsedIdentifierName:
                name = "the identifier has been declared";
                break;
            case miniplc0::ErrIdentifierNotDeclare:
                name = "identifier has never been declared";
                break;
            case miniplc0::ErrFunctionNameHasBeenOverride:
                name = "function name has been recovered in this level";
                break;
            case miniplc0::ErrFunctionNotDeclare:
                name = "the function has not been declared";
                break;
            case miniplc0::ErrIncorrectParaNum:
                name = "the paramenter is not expected here";
                break;
            case miniplc0::ErrIncorrectType:
                name = "incorrect type";
                break;
            case miniplc0::ErrIncorrectReturnType:
                name = "incorrect return type";
                break;
            case miniplc0::ErrNoBracket:
                name = "no bracket here";
                break;
            case miniplc0::ErrNoMainFunction:
                name = "no main function in this program";
                break;
            case miniplc0::ErrNoReturnStatement:
                name = "no return statement in this function";
                break;
            }
            return format_to(ctx.out(), name);
        }
	};

	template<>
	struct formatter<miniplc0::CompilationError> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const miniplc0::CompilationError &p, FormatContext &ctx) {
			return format_to(ctx.out(), "Line: {} Column: {} Error: {}", p.GetPos().first, p.GetPos().second, p.GetCode());
		}
	};
}

// TokenType
namespace fmt {
	template<>
	struct formatter<miniplc0::Token> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const miniplc0::Token &p, FormatContext &ctx) {
			return format_to(ctx.out(),
				"Line: {} Column: {} Type: {} Value: {}",
				p.GetStartPos().first, p.GetStartPos().second, p.GetType(), p.GetValueString());
		}
	};

	template<>
	struct formatter<miniplc0::TokenType> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const miniplc0::TokenType &p, FormatContext &ctx) {
			std::string name;
			switch (p) {
                case miniplc0::NULL_TOKEN:
                    name = "NullToken";
                    break;
                case miniplc0::UNSIGNED_INTEGER:
                    name = "UnsignedInteger";
                    break;
                case miniplc0::IDENTIFIER:
                    name = "Identifier";
                    break;
                case miniplc0::BEGIN:
                    name = "Begin";
                    break;
                case miniplc0::END:
                    name = "End";
                    break;
                case miniplc0::VAR:
                    name = "Var";
                    break;
                case miniplc0::CONST:
                    name = "Const";
                    break;
                case miniplc0::PRINT:
                    name = "Print";
                    break;
                case miniplc0::PLUS_SIGN:
                    name = "PlusSign";
                    break;
                case miniplc0::MINUS_SIGN:
                    name = "MinusSign";
                    break;
                case miniplc0::MULTIPLICATION_SIGN:
                    name = "MultiplicationSign";
                    break;
                case miniplc0::DIVISION_SIGN:
                    name = "DivisionSign";
                    break;
                case miniplc0::EQUAL_SIGN:
                    name = "EqualSign";
                    break;
                case miniplc0::SEMICOLON:
                    name = "Semicolon";
                    break;
                case miniplc0::LEFT_BRACKET:
                    name = "LeftBracket";
                    break;
                case miniplc0::RIGHT_BRACKET:
                    name = "RightBracket";
                    break;
                case miniplc0::BIG_LEFT_BRACKET:
                    name = "BigLeftBracket";
                    break;
                case miniplc0::BIG_RIGHT_BRACKET:
                    name = "BigRightBracket";
                    break;
                case miniplc0::LESS_THAN_SIGN:
                    name = "LessThanSign";
                    break;
                case miniplc0::LESS_OR_EQUAL_SIGN:
                    name = "LessOrEqualSign";
                    break;
                case miniplc0::MORE_THAN_SIGN:
                    name = "MoreThanSign";
                    break;
                case miniplc0::MORE_OR_EQUAL_SIGN:
                    name = "MoreOrEqualSign";
                    break;
                case miniplc0::IS_EQUAL_SIGN:
                    name = "IsEqualSign";
                    break;
                case miniplc0::NOT_EQUAL_SIGN:
                    name = "NotEqualSign";
                    break;
                case miniplc0::DECIMAL_UNSIGNED_INTEGER:
                    name = "DecimalUnsignedInteger";
                    break;
                case miniplc0::HEXADECIMAL_UNSIGNED_INTEGER:
                    name = "HexadecimalUnsignedInteger";
                    break;
                case miniplc0::VOID:
                    name = "Void";
                    break;
                case miniplc0::INT:
                    name = "Int";
                    break;
                case miniplc0::CHAR:
                    name = "Char";
                    break;
                case miniplc0::DOUBLE:
                    name = "Double";
                    break;
                case miniplc0::STRUCT:
                    name = "Struct";
                    break;
                case miniplc0::IF:
                    name = "If";
                    break;
                case miniplc0::ELSE:
                    name = "Else";
                    break;
                case miniplc0::SWITCH:
                    name = "Switch";
                    break;
                case miniplc0::CASE:
                    name = "Case";
                    break;
                case miniplc0::DEFAULT:
                    name = "Default";
                    break;
                case miniplc0::WHILE:
                    name = "While";
                    break;
                case miniplc0::FOR:
                    name = "For";
                    break;
                case miniplc0::DO:
                    name = "Do";
                    break;
                case miniplc0::RETURN:
                    name = "Return";
                    break;
                case miniplc0::BREAK:
                    name = "Break";
                    break;
                case miniplc0::CONTINUE:
                    name = "Continue";
                    break;
                case miniplc0::SCAN:
                    name = "Scan";
                    break;
                case miniplc0::COMMA_SIGN:
                    name = "CommaSign";
                    break;
                case miniplc0::SINGLE_QUOTATION_MARKS:
                    name = "SingleQuotationMarks";
                    break;
                case miniplc0::DOUBLE_QUOTATION_MARKS:
                    name = "DoubleQuotationMarks";
                    break;
			}
			return format_to(ctx.out(), name);
		}
	};
}

// Instruction
namespace fmt {
	template<>
	struct formatter<miniplc0::Operation> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const miniplc0::Operation &p, FormatContext &ctx) {
			std::string name;
			switch (p) {

				break;
                case miniplc0::NOP:
                    break;
                case miniplc0::BIPUSH:
                    break;
                case miniplc0::IPUSH:
                    break;
                case miniplc0::POP:
                    break;
                case miniplc0::POP2:
                    break;
                case miniplc0::POPN:
                    break;
                case miniplc0::DUP:
                    break;
                case miniplc0::DUP2:
                    break;
                case miniplc0::LOADC:
                    break;
                case miniplc0::LOADA:
                    break;
                case miniplc0::NEW:
                    break;
                case miniplc0::SNEW:
                    break;
                case miniplc0::ILOAD:
                    break;
                case miniplc0::DLOAD:
                    break;
                case miniplc0::ALOAD:
                    break;
                case miniplc0::IALOAD:
                    break;
                case miniplc0::DALOAD:
                    break;
                case miniplc0::AALOAD:
                    break;
                case miniplc0::ISTORE:
                    break;
                case miniplc0::DSTORE:
                    break;
                case miniplc0::ASTORE:
                    break;
                case miniplc0::IASTORE:
                    break;
                case miniplc0::DASTORE:
                    break;
                case miniplc0::AASTROE:
                    break;
                case miniplc0::IADD:
                    break;
                case miniplc0::DADD:
                    break;
                case miniplc0::ISUB:
                    break;
                case miniplc0::DSUB:
                    break;
                case miniplc0::IMUL:
                    break;
                case miniplc0::DMUL:
                    break;
                case miniplc0::IDIV:
                    break;
                case miniplc0::DDIV:
                    break;
                case miniplc0::INEG:
                    break;
                case miniplc0::DNEG:
                    break;
                case miniplc0::ICMP:
                    break;
                case miniplc0::DCMP:
                    break;
                case miniplc0::I2D:
                    break;
                case miniplc0::D2I:
                    break;
                case miniplc0::I2C:
                    break;
                case miniplc0::JMP:
                    break;
                case miniplc0::JE:
                    break;
                case miniplc0::JNE:
                    break;
                case miniplc0::JL:
                    break;
                case miniplc0::JGE:
                    break;
                case miniplc0::JG:
                    break;
                case miniplc0::JLE:
                    break;
                case miniplc0::CALL:
                    break;
                case miniplc0::RET:
                    break;
                case miniplc0::IRET:
                    break;
                case miniplc0::DRET:
                    break;
                case miniplc0::ARET:
                    break;
                case miniplc0::IPRINT:
                    break;
                case miniplc0::DPRINT:
                    break;
                case miniplc0::CPRINT:
                    break;
                case miniplc0::SPRINT:
                    break;
                case miniplc0::PRINTL:
                    break;
                case miniplc0::ISCAN:
                    break;
                case miniplc0::DSCAN:
                    break;
                case miniplc0::CSCAN:
                    break;
            }
			return format_to(ctx.out(), name);
		}
	};
	template<>
	struct formatter<miniplc0::Instruction> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const miniplc0::Instruction &p, FormatContext &ctx) {
			std::string name;
			switch (p.GetOperation())
			{
			    // return format_to(ctx.out(), "{}", p.GetOperation());
				// return format_to(ctx.out(), "{} {}", p.GetOperation(), p.GetX());
                case miniplc0::NOP:
                    break;
                case miniplc0::BIPUSH:
                    break;
                case miniplc0::IPUSH:
                    break;
                case miniplc0::POP:
                    break;
                case miniplc0::POP2:
                    break;
                case miniplc0::POPN:
                    break;
                case miniplc0::DUP:
                    break;
                case miniplc0::DUP2:
                    break;
                case miniplc0::LOADC:
                    break;
                case miniplc0::LOADA:
                    break;
                case miniplc0::NEW:
                    break;
                case miniplc0::SNEW:
                    break;
                case miniplc0::ILOAD:
                    break;
                case miniplc0::DLOAD:
                    break;
                case miniplc0::ALOAD:
                    break;
                case miniplc0::IALOAD:
                    break;
                case miniplc0::DALOAD:
                    break;
                case miniplc0::AALOAD:
                    break;
                case miniplc0::ISTORE:
                    break;
                case miniplc0::DSTORE:
                    break;
                case miniplc0::ASTORE:
                    break;
                case miniplc0::IASTORE:
                    break;
                case miniplc0::DASTORE:
                    break;
                case miniplc0::AASTROE:
                    break;
                case miniplc0::IADD:
                    break;
                case miniplc0::DADD:
                    break;
                case miniplc0::ISUB:
                    break;
                case miniplc0::DSUB:
                    break;
                case miniplc0::IMUL:
                    break;
                case miniplc0::DMUL:
                    break;
                case miniplc0::IDIV:
                    break;
                case miniplc0::DDIV:
                    break;
                case miniplc0::INEG:
                    break;
                case miniplc0::DNEG:
                    break;
                case miniplc0::ICMP:
                    break;
                case miniplc0::DCMP:
                    break;
                case miniplc0::I2D:
                    break;
                case miniplc0::D2I:
                    break;
                case miniplc0::I2C:
                    break;
                case miniplc0::JMP:
                    break;
                case miniplc0::JE:
                    break;
                case miniplc0::JNE:
                    break;
                case miniplc0::JL:
                    break;
                case miniplc0::JGE:
                    break;
                case miniplc0::JG:
                    break;
                case miniplc0::JLE:
                    break;
                case miniplc0::CALL:
                    break;
                case miniplc0::RET:
                    break;
                case miniplc0::IRET:
                    break;
                case miniplc0::DRET:
                    break;
                case miniplc0::ARET:
                    break;
                case miniplc0::IPRINT:
                    break;
                case miniplc0::DPRINT:
                    break;
                case miniplc0::CPRINT:
                    break;
                case miniplc0::SPRINT:
                    break;
                case miniplc0::PRINTL:
                    break;
                case miniplc0::ISCAN:
                    break;
                case miniplc0::DSCAN:
                    break;
                case miniplc0::CSCAN:
                    break;
            }
			return format_to(ctx.out(), "ILL");
		}
	};
}