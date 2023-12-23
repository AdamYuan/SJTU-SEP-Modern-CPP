#pragma once

#include "Config.hpp"

#include <stdexcept>
#include <variant>

namespace basic {

#define PARSE_ERROR_HEAD "[PARSE ERROR] "
#define RUNTIME_ERROR_HEAD "[RUNTIME ERROR] "
#define RUNTIME_MSG_HEAD "[RUNTIME INFO] "

// Parse errors
struct ErrInvalidToken {
	String expr_str, stmt_str, token_str;
	inline String Format() const {
		String in = expr_str.empty() ? (stmt_str.empty() ? "" : " in statement \'" + stmt_str + "\'")
		                             : " in expression \'" + expr_str + "\'";
		return PARSE_ERROR_HEAD "Invalid token \'" + token_str + "\'" + in;
	}
};
struct ErrMissingToken {
	String expr_str, stmt_str, token_str;
	inline String Format() const {
		String in = expr_str.empty() ? (stmt_str.empty() ? "" : " in statement \'" + stmt_str + "\'")
		                             : " in expression \'" + expr_str + "\'";
		return PARSE_ERROR_HEAD "Missing token \'" + token_str + "\'" + in;
	}
};
struct ErrNoOperand {
	String expr_str, operator_str;
	inline String Format() const {
		return PARSE_ERROR_HEAD "No operand for \'" + operator_str + "\' in \'" + expr_str + "\'";
	}
};
struct ErrEmptyExpr {
	String expr_str;
	inline String Format() const { return PARSE_ERROR_HEAD "Empty expression \'" + expr_str + "\'"; }
};
struct ErrOrphanExpr {
	String expr_str, orph_expr_str;
	inline String Format() const {
		return PARSE_ERROR_HEAD "Orphan expression \'" + orph_expr_str + "\' in \'" + expr_str + "\'";
	}
};
struct ErrBracketUnmatched {
	String expr_str;
	inline String Format() const { return PARSE_ERROR_HEAD "Unmatched brackets in \'" + expr_str + "\'"; }
};

struct ErrInvalidVariable {
	String expr_str, stmt_str, var_str;
	inline String Format() const {
		String in = expr_str.empty() ? (stmt_str.empty() ? "" : " in statement \'" + stmt_str + "\'")
		                             : " in expression \'" + expr_str + "\'";
		return PARSE_ERROR_HEAD "Invalid variable \'" + var_str + "\'" + in;
	}
};
struct ErrInvalidDigit {
	String expr_str, stmt_str, digit_str;
	inline String Format() const {
		String in = expr_str.empty() ? (stmt_str.empty() ? "" : " in statement \'" + stmt_str + "\'")
		                             : " in expression \'" + expr_str + "\'";
		return PARSE_ERROR_HEAD "Invalid digit \'" + digit_str + "\'" + in;
	}
};
struct ErrEmptyStmt {
	inline String Format() const { return PARSE_ERROR_HEAD "Empty statement"; }
};

// Runtime errors
struct ErrUndefinedVariable {
	String var;
	inline String Format() const { return RUNTIME_ERROR_HEAD "Undefined variable \'" + var + "\'"; }
};
struct ErrUndefinedLine {
	LineID line;
	inline String Format() const { return RUNTIME_ERROR_HEAD "Undefined line \'" + std::to_string(line) + "\'"; }
};
struct ErrDivByZero {
	String zero_expr_str;
	inline String Format() const {
		return RUNTIME_ERROR_HEAD "Divided by zero value expression \'" + zero_expr_str + "\'";
	}
};
struct ErrExpByNeg {
	String neg_expr_str;
	inline String Format() const {
		return RUNTIME_ERROR_HEAD "Exponentiated by negative value expression \'" + neg_expr_str + "\'";
	}
};
struct ErrInvalidInput {
	String input;
	inline String Format() const { return RUNTIME_ERROR_HEAD "Invalid input \'" + input + "\'"; }
};
struct ErrTerminate {
	static inline String Format() { return RUNTIME_ERROR_HEAD "Program terminated by user"; }
};

// messages, not error, but used as error
struct MsgPrint {
	inline String Format() const { return ""; }
};
struct MsgEndOfProgram {
	inline String Format() const { return RUNTIME_MSG_HEAD "Program ended"; }
};
struct MsgRequestInput {
	inline String Format() const { return RUNTIME_MSG_HEAD "Input requested"; }
};

#undef PARSE_ERROR_HEAD
#undef RUNTIME_ERROR_HEAD
#undef RUNTIME_MSG_HEAD

template <typename... Errors> class Error {
private:
	std::variant<Errors...> m_err;

public:
	template <typename T> inline Error(T &&val) : m_err{std::forward<T>(val)} {}
	inline String Format() const {
		return std::visit([](const auto &err) -> String { return err.Format(); }, m_err);
	}
	template <typename Visitor> inline void Visit(Visitor &&visitor) const {
		std::visit(std::forward<Visitor>(visitor), m_err);
	}
};

using ParseError = Error<ErrNoOperand, ErrEmptyExpr, ErrOrphanExpr, ErrBracketUnmatched, ErrInvalidToken,
                         ErrMissingToken, ErrInvalidVariable, ErrInvalidDigit, ErrEmptyStmt>;
using RuntimeError = Error<ErrUndefinedVariable, ErrUndefinedLine, ErrDivByZero, ErrExpByNeg, ErrTerminate,
                           ErrInvalidInput, MsgPrint, MsgEndOfProgram, MsgRequestInput>;

template <typename Type, typename ErrorType> class Result {
private:
	using RType = std::conditional_t<std::is_same_v<Type, void>, std::monostate, Type>;
	std::variant<RType, ErrorType> m_res;

public:
	inline Result() : m_res{std::monostate{}} { static_assert(std::is_same_v<Type, void>); }
	template <typename T> inline Result(T &&val) : m_res{std::forward<T>(val)} {}
	inline bool IsError() const { return m_res.index() == 1; }
	inline bool IsOK() const { return m_res.index() == 0; }
	inline RType PopValue() {
		return std::visit(
		    [](auto &v) -> RType {
			    if constexpr (std::is_same_v<RType, std::decay_t<decltype(v)>>)
				    return std::move(v);
			    else
				    throw std::runtime_error("Result has no value");
		    },
		    m_res);
	}
	inline ErrorType PopError() {
		return std::visit(
		    [](auto &v) -> ErrorType {
			    if constexpr (std::is_same_v<RType, std::decay_t<decltype(v)>>)
				    throw std::runtime_error("Result has no error");
			    else
				    return std::move(v);
		    },
		    m_res);
	}
};

#define BASIC_UNWRAP_ASSIGN(L_VALUE, RESULT) \
	do { \
		auto result = RESULT; \
		if (result.IsError()) \
			return result.PopError(); \
		L_VALUE = result.PopValue(); \
	} while (false)

#define BASIC_UNWRAP(RESULT) \
	do { \
		auto result = RESULT; \
		if (result.IsError()) \
			return result.PopError(); \
	} while (false)

template <typename Type> using ParseResult = Result<Type, ParseError>;
template <typename Type> using RuntimeResult = Result<Type, RuntimeError>;

} // namespace basic
