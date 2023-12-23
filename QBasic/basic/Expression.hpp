#pragma once

#include <climits>
#include <memory>
#include <span>
#include <variant>

#include "Config.hpp"
#include "Error.hpp"
#include "Token.hpp"

namespace basic {

class Context;

enum class ExpressionType { kOperand, kUnary, kBinary };
enum class ExpressionAsso { kLeft, kRight };

class Expression;

// unary operators only support right associative
#define BASIC_OPERATOR_UNARY(KEY, PRE) \
	inline static constexpr ExpressionType kType = ExpressionType::kUnary; \
	inline static constexpr const char *kSymbol = KEY; \
	inline static constexpr bool IsSymbol(const String &token) { return token == KEY; } \
	inline static constexpr int kPrecedence = PRE; \
	std::unique_ptr<Expression> child;

// operators with same precedence should have the same associative
#define BASIC_OPERATOR_BINARY(KEY, PRE, ASS) \
	inline static constexpr ExpressionType kType = ExpressionType::kBinary; \
	inline static constexpr ExpressionAsso kAssociative = ExpressionAsso::ASS; \
	inline static constexpr const char *kSymbol = KEY; \
	inline static constexpr bool IsSymbol(const String &token) { return token == KEY; } \
	inline static constexpr int kPrecedence = PRE; \
	std::unique_ptr<Expression> left, right;

struct ExprNum {
	inline static constexpr ExpressionType kType = ExpressionType::kOperand;
	inline static bool IsSymbol(const Token &token) { return token.IsDigit(); }
	inline static ExprNum FromToken(const Token &token) { return {token.ToDigit<Int>()}; }

	Int value;
	inline RuntimeResult<Int> Eval(const Context &context) const { return value; }
	inline String Format() const { return std::to_string(value); }
};
struct ExprVar {
	inline static constexpr ExpressionType kType = ExpressionType::kOperand;
	inline static bool IsSymbol(const Token &token) { return token.IsVariable(); }
	inline static ExprVar FromToken(const Token &token) { return {token.GetString()}; }

	String var;
	RuntimeResult<Int> Eval(const Context &context) const;
	inline String Format() const { return var; }
};
struct ExprSub {
	BASIC_OPERATOR_BINARY("-", 0, kLeft)
	inline static RuntimeResult<Int> Eval(Int l, Int r) { return l - r; }
};
struct ExprNeg {
	BASIC_OPERATOR_UNARY("-", 30)
	inline static RuntimeResult<Int> Eval(Int v) { return -v; }
};
struct ExprAdd {
	BASIC_OPERATOR_BINARY("+", 0, kLeft)
	inline static RuntimeResult<Int> Eval(Int l, Int r) { return l + r; }
};
struct ExprPos {
	BASIC_OPERATOR_UNARY("+", 30)
	inline static RuntimeResult<Int> Eval(Int v) { return v; }
};
struct ExprMul {
	BASIC_OPERATOR_BINARY("*", 10, kLeft)
	inline static RuntimeResult<Int> Eval(Int l, Int r) { return l * r; }
};
struct ExprDiv {
	BASIC_OPERATOR_BINARY("/", 10, kLeft)
	RuntimeResult<Int> Eval(Int l, Int r) const;
};
struct ExprMod {
	BASIC_OPERATOR_BINARY("MOD", 10, kLeft)
	RuntimeResult<Int> Eval(Int l, Int r) const;
};
struct ExprExp {
	BASIC_OPERATOR_BINARY("**", 20, kRight)
	RuntimeResult<Int> Eval(Int l, Int r) const;
};

#undef BASIC_OPERATOR_UNARY
#undef BASIC_OPERATOR_BINARY

class Expression {
private:
	using Variant =
	    std::variant<ExprNum, ExprAdd, ExprPos, ExprSub, ExprNeg, ExprMul, ExprDiv, ExprMod, ExprExp, ExprVar>;
	// ExprVar should be placed at last to be the last one to be matched
	Variant m_expr;

public:
	template <typename T> inline Expression(T &&expr) : m_expr{std::forward<T>(expr)} {}

	static ParseResult<std::unique_ptr<Expression>> Parse(std::span<const Token> tokens);

	inline RuntimeResult<Int> Eval(const Context &context) const {
		return std::visit(
		    [&context](const auto &expr) -> RuntimeResult<Int> {
			    using Expr = std::decay_t<decltype(expr)>;
			    if constexpr (Expr::kType == ExpressionType::kOperand)
				    return expr.Eval(context);
			    else if constexpr (Expr::kType == ExpressionType::kUnary) {
				    Int v;
				    BASIC_UNWRAP_ASSIGN(v, expr.child->Eval(context));
				    return expr.Eval(v);
			    } else {
				    Int l, r;
				    BASIC_UNWRAP_ASSIGN(l, expr.left->Eval(context));
				    BASIC_UNWRAP_ASSIGN(r, expr.right->Eval(context));
				    return expr.Eval(l, r);
			    }
		    },
		    m_expr);
	}
	inline String Format() const {
		return std::visit(
		    [](const auto &expr) -> String {
			    using Expr = std::decay_t<decltype(expr)>;
			    if constexpr (Expr::kType == ExpressionType::kOperand)
				    return expr.Format();
			    else if constexpr (Expr::kType == ExpressionType::kUnary) {
				    String cs = expr.child->Format();
				    if (expr.child->GetPrecedence() < Expr::kPrecedence)
					    cs = '(' + cs + ')';
				    return Expr::kSymbol + cs;
			    } else {
				    String ls = expr.left->Format(), rs = expr.right->Format();
				    if constexpr (Expr::kAssociative == ExpressionAsso::kRight) {
					    if (expr.left->GetPrecedence() <= Expr::kPrecedence)
						    ls = '(' + ls + ')';
					    if (expr.right->GetPrecedence() < Expr::kPrecedence)
						    rs = '(' + rs + ')';
				    } else {
					    if (expr.left->GetPrecedence() < Expr::kPrecedence)
						    ls = '(' + ls + ')';
					    if (expr.right->GetPrecedence() <= Expr::kPrecedence)
						    rs = '(' + rs + ')';
				    }
				    return ls + ' ' + Expr::kSymbol + ' ' + rs;
			    }
		    },
		    m_expr);
	}
	inline String FormatAST(const String &align) const {
		return std::visit(
		    [&align](const auto &expr) -> String {
			    using Expr = std::decay_t<decltype(expr)>;
			    if constexpr (Expr::kType == ExpressionType::kOperand)
				    return align + expr.Format() + "\n";
			    else if constexpr (Expr::kType == ExpressionType::kUnary)
				    return align + Expr::kSymbol + "\n" + expr.child->FormatAST(align + kASTFormatAlign);
			    else
				    return align + Expr::kSymbol + "\n" + expr.left->FormatAST(align + kASTFormatAlign) +
				           expr.right->FormatAST(align + kASTFormatAlign);
		    },
		    m_expr);
	}
	inline int GetPrecedence() const {
		return std::visit(
		    [](const auto &expr) {
			    using Expr = std::decay_t<decltype(expr)>;
			    if constexpr (Expr::kType == ExpressionType::kOperand)
				    return INT_MAX;
			    else
				    return Expr::kPrecedence;
		    },
		    m_expr);
	}
	inline ExpressionAsso GetAssociative() const {
		return std::visit(
		    [](const auto &expr) -> ExpressionAsso {
			    using Expr = std::decay_t<decltype(expr)>;
			    if constexpr (Expr::kType == ExpressionType::kBinary)
				    return Expr::kAssociative;
			    else
				    return ExpressionAsso::kRight;
		    },
		    m_expr);
	}
};

} // namespace basic
