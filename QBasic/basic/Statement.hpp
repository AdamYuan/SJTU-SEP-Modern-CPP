#pragma once

#include <memory>
#include <variant>

#include "Config.hpp"
#include "Expression.hpp"

namespace basic {

class Context;
class Program;

struct StmtRem {
	inline static constexpr const char *kKeyWord = "REM";

	String comment;
	static RuntimeResult<void> Run(const Program &program, Context *p_context);
	ParseResult<void> Parse(std::span<const Token> tokens);

	inline String Format() const { return comment; }
	String FormatAST(LineID line, const Context *p_context) const;
};
struct StmtInput {
	inline static constexpr const char *kKeyWord = "INPUT";

	String var;
	RuntimeResult<void> Run(const Program &program, Context *p_context) const;
	ParseResult<void> Parse(std::span<const Token> tokens);

	inline String Format() const { return var; }
	String FormatAST(LineID line, const Context *p_context) const;
};
struct StmtPrint {
	inline static constexpr const char *kKeyWord = "PRINT";

	std::unique_ptr<Expression> expr;
	RuntimeResult<void> Run(const Program &program, Context *p_context) const;
	ParseResult<void> Parse(std::span<const Token> tokens);

	inline String Format() const { return expr->Format(); }
	String FormatAST(LineID line, const Context *p_context) const;
};
struct StmtLet {
	inline static constexpr const char *kKeyWord = "LET";

	String var;
	std::unique_ptr<Expression> expr;
	RuntimeResult<void> Run(const Program &program, Context *p_context) const;
	ParseResult<void> Parse(std::span<const Token> tokens);

	inline String Format() const { return var + " = " + expr->Format(); }
	String FormatAST(LineID line, const Context *p_context) const;
};
struct StmtGoto {
	inline static constexpr const char *kKeyWord = "GOTO";

	LineID line;
	RuntimeResult<void> Run(const Program &program, Context *p_context) const;
	ParseResult<void> Parse(std::span<const Token> tokens);

	inline String Format() const { return std::to_string(line); }
	String FormatAST(LineID line, const Context *p_context) const;
};
struct StmtIf {
	inline static constexpr const char *kKeyWord = "IF";

	std::unique_ptr<Expression> expr_l, expr_r;
	Char cmp;
	LineID line_then;
	RuntimeResult<void> Run(const Program &program, Context *p_context) const;
	ParseResult<void> Parse(std::span<const Token> tokens);

	inline String Format() const {
		return expr_l->Format() + ' ' + cmp + ' ' + expr_r->Format() + " THEN " + std::to_string(line_then);
	}
	String FormatAST(LineID line, const Context *p_context) const;
};
struct StmtEnd {
	inline static constexpr const char *kKeyWord = "END";

	static RuntimeResult<void> Run(const Program &program, Context *p_context);
	static ParseResult<void> Parse(std::span<const Token> tokens);

	inline String Format() const { return ""; }
	static String FormatAST(LineID line, const Context *p_context);
};

class Statement {
private:
	using Variant = std::variant<StmtRem, StmtInput, StmtPrint, StmtLet, StmtGoto, StmtIf, StmtEnd>;
	Variant m_stmt;

public:
	template <typename T> inline Statement(T &&stmt) : m_stmt{std::forward<T>(stmt)} {}
	static ParseResult<std::unique_ptr<Statement>> Parse(std::span<const Token> tokens);

	inline RuntimeResult<void> Run(const Program &program, Context *p_context) const {
		return std::visit([&program, p_context](const auto &stmt) { return stmt.Run(program, p_context); }, m_stmt);
	}
	inline String Format() const {
		return std::visit([](const auto &stmt) { return String(stmt.kKeyWord) + " " + stmt.Format(); }, m_stmt);
	}
	inline String FormatAST(LineID line, const Context *p_context) const {
		return std::visit(
		    [line, p_context](const auto &stmt) {
			    return String(stmt.kKeyWord) + " " + stmt.FormatAST(line, p_context);
		    },
		    m_stmt);
	}
};

} // namespace basic
