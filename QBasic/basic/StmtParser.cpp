#include "Statement.hpp"

#include "Token.hpp"
#include <optional>

namespace basic {

ParseResult<std::unique_ptr<Statement>> Statement::Parse(std::span<const Token> tokens) {
	if (tokens.empty())
		return ErrEmptyStmt{};

	const auto foreach_stmt = [](auto &&func) { VariantIterator<Variant>::Run(func); };

	std::unique_ptr<Statement> stmt_ptr = nullptr;

	foreach_stmt([&](auto &&_stmt) -> bool {
		using Stmt = std::decay_t<decltype(_stmt)>;
		if (tokens[0].IsKeyword(Stmt::kKeyWord)) {
			stmt_ptr = std::make_unique<Statement>(Stmt{});
			return true;
		}
		return {};
	});

	if (stmt_ptr == nullptr)
		return ErrInvalidToken{.stmt_str = Token::DeTokenize(tokens), .token_str = tokens[0].GetString()};

	BASIC_UNWRAP(std::visit([&](auto &&stmt) -> ParseResult<void> { return stmt.Parse(tokens); }, stmt_ptr->m_stmt));
	return std::move(stmt_ptr);
}
// Parsers (tokens.size() >= 1, tokens[0] is its keyword)
ParseResult<void> StmtRem::Parse(std::span<const Token> tokens) {
	this->comment = Token::DeTokenize(tokens.subspan(1));
	return {};
}
ParseResult<void> StmtInput::Parse(std::span<const Token> tokens) {
	if (tokens.size() != 2 || !tokens[1].IsVariable())
		return ErrInvalidVariable{.stmt_str = Token::DeTokenize(tokens),
		                          .var_str = Token::DeTokenize(tokens.subspan(1))};
	this->var = tokens[1].GetString();
	return {};
}
ParseResult<void> StmtPrint::Parse(std::span<const Token> tokens) {
	BASIC_UNWRAP_ASSIGN(this->expr, Expression::Parse(tokens.subspan(1)));
	return {};
}
ParseResult<void> StmtLet::Parse(std::span<const Token> tokens) {
	std::optional<std::size_t> opt_equal_pos;
	for (std::size_t i = 1; i < tokens.size(); ++i)
		if (tokens[i].IsKeyword("=")) {
			opt_equal_pos = i;
			break;
		}
	if (!opt_equal_pos.has_value())
		return ErrMissingToken{.stmt_str = Token::DeTokenize(tokens), .token_str = "="};
	std::size_t equal_pos = opt_equal_pos.value();

	BASIC_UNWRAP_ASSIGN(this->expr, Expression::Parse(tokens.subspan(equal_pos + 1)));

	std::span<const Token> var_tokens = tokens.subspan(1, equal_pos - 1);
	if (var_tokens.size() != 1 || !var_tokens[0].IsVariable())
		return ErrInvalidVariable{.stmt_str = Token::DeTokenize(tokens), .var_str = Token::DeTokenize(var_tokens)};

	this->var = var_tokens[0].GetString();
	return {};
}
ParseResult<void> StmtGoto::Parse(std::span<const Token> tokens) {
	if (tokens.size() != 2 || !tokens[1].IsDigit())
		return ErrInvalidDigit{.stmt_str = Token::DeTokenize(tokens),
		                       .digit_str = Token::DeTokenize(tokens.subspan(1))};
	this->line = tokens[1].ToDigit<LineID>();
	return {};
}
ParseResult<void> StmtIf::Parse(std::span<const Token> tokens) {
	std::size_t cmp_pos;
	{
		std::optional<std::size_t> opt_cmp_pos;
		for (std::size_t i = 1; i < tokens.size(); ++i)
			if (tokens[i].IsKeyword("=") || tokens[i].IsKeyword("<") || tokens[i].IsKeyword(">")) {
				opt_cmp_pos = i;
				break;
			}
		if (!opt_cmp_pos.has_value())
			return ErrMissingToken{.stmt_str = Token::DeTokenize(tokens), .token_str = "<, =, >"};

		cmp_pos = opt_cmp_pos.value();
	}
	this->cmp = tokens[cmp_pos].GetView()[0];

	std::size_t then_pos;
	{
		std::optional<std::size_t> opt_then_pos;
		for (std::size_t i = tokens.size() - 1; i > cmp_pos; --i)
			if (tokens[i].IsKeyword("THEN")) {
				opt_then_pos = i;
				break;
			}
		if (!opt_then_pos.has_value())
			return ErrMissingToken{.stmt_str = Token::DeTokenize(tokens), .token_str = "THEN"};

		then_pos = opt_then_pos.value();
	}

	auto expr_l_tokens = tokens.subspan(1, cmp_pos - 1),
	     expr_r_tokens = tokens.subspan(cmp_pos + 1, then_pos - cmp_pos - 1);

	BASIC_UNWRAP_ASSIGN(this->expr_l, Expression::Parse(expr_l_tokens));
	BASIC_UNWRAP_ASSIGN(this->expr_r, Expression::Parse(expr_r_tokens));

	auto line_tokens = tokens.subspan(then_pos + 1);
	if (line_tokens.size() != 1 || !line_tokens[0].IsDigit())
		return ErrInvalidDigit{.stmt_str = Token::DeTokenize(tokens), .digit_str = Token::DeTokenize(line_tokens)};

	this->line_then = line_tokens[0].ToDigit<LineID>();
	return {};
}
ParseResult<void> StmtEnd::Parse(std::span<const Token> tokens) {
	if (tokens.size() > 1)
		return ErrInvalidToken{.stmt_str = Token::DeTokenize(tokens), .token_str = tokens[1].GetString()};
	return {};
}

} // namespace basic