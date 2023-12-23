#include "Expression.hpp"

#include "Token.hpp"
#include <climits>
#include <optional>
#include <stack>

namespace basic {

ParseResult<std::unique_ptr<Expression>> Expression::Parse(std::span<const Token> tokens) {
	if (tokens.empty())
		return ErrEmptyExpr{.expr_str = Token::DeTokenize(tokens)};

	const auto foreach_expr = [](auto &&func) { VariantIterator<Variant>::Run(func); };

	// preprocess
	std::vector<std::unique_ptr<Expression>> token_exprs(tokens.size());
	{
		// detect '()' tokens
		for (std::size_t i = 1; i < tokens.size(); ++i)
			if (tokens[i - 1].GetView().back() == '(' && tokens[i].GetView().front() == ')')
				return ErrInvalidToken{.expr_str = Token::DeTokenize(tokens), .token_str = "()"};

		// mark all operators, separate from operands and brackets
		std::vector<bool> token_is_operator(tokens.size());
		for (std::size_t i = 0; i < tokens.size(); ++i) {
			foreach_expr([&](auto &&expr) -> bool {
				using Expr = std::decay_t<decltype(expr)>;
				if constexpr (Expr::kType != ExpressionType::kOperand) {
					if (tokens[i].IsKeyword(Expr::kSymbol)) {
						token_is_operator[i] = true;
						return true; // break
					}
				}
				return {};
			});
		}

		// determine whether the operators are binary or unary
		std::vector<bool> token_is_unary_operator(tokens.size());
		for (std::size_t i = 0; i < tokens.size(); ++i) {
			// operator with no operand on left is unary operator
			token_is_unary_operator[i] = i == 0 || token_is_operator[i - 1] || tokens[i - 1].GetView().front() == '(';
		}

		// create token expressions, detect invalid expressions
		for (std::size_t i = 0; i < tokens.size(); ++i) {
			// match expression
			foreach_expr([&](auto &&expr) -> bool {
				using Expr = std::decay_t<decltype(expr)>;
				if constexpr (Expr::kType == ExpressionType::kOperand) {
					if (!token_is_operator[i] && Expr::IsSymbol(tokens[i])) {
						token_exprs[i] = std::make_unique<Expression>(Expr::FromToken(tokens[i]));
						return true; // break
					}
				} else if constexpr (Expr::kType == ExpressionType::kUnary) {
					if (token_is_operator[i] && token_is_unary_operator[i] && tokens[i].IsKeyword(Expr::kSymbol)) {
						token_exprs[i] = std::make_unique<Expression>(Expr{});
						return true; // break
					}
				} else {
					if (token_is_operator[i] && !token_is_unary_operator[i] && tokens[i].IsKeyword(Expr::kSymbol)) {
						token_exprs[i] = std::make_unique<Expression>(Expr{});
						return true; // break
					}
				}
				return {};
			});

			// if not expression, then should be bracket, or it is an invalid token
			auto token_view = tokens[i].GetView();
			if (token_exprs[i] == nullptr && token_view.front() != '(' && token_view.front() != ')')
				return ErrInvalidToken{.expr_str = Token::DeTokenize(tokens), .token_str = tokens[i].GetString()};
		}
	}

	// token_stack stores token expressions without child data, nullptr means left bracket '('
	std::stack<std::unique_ptr<Expression>> token_stack, expr_stack;

	const auto make_new_expr = [&](int precedence, bool asso_equal) -> ParseResult<void> {
		while (!token_stack.empty() && token_stack.top() != nullptr &&
		       (asso_equal ? token_stack.top()->GetPrecedence() >= precedence
		                   : token_stack.top()->GetPrecedence() > precedence)) {
			auto op = std::move(token_stack.top());
			token_stack.pop();

			BASIC_UNWRAP(std::visit(
			    [&](auto &&expr) -> ParseResult<void> {
				    using Expr = std::decay_t<decltype(expr)>;
				    if constexpr (Expr::kType == ExpressionType::kUnary) {
					    if (expr_stack.empty())
						    return ErrNoOperand{.expr_str = Token::DeTokenize(tokens), .operator_str = Expr::kSymbol};
					    expr.child = std::move(expr_stack.top());
					    expr_stack.pop();
				    } else if constexpr (Expr::kType == ExpressionType::kBinary) {
					    if (expr_stack.empty())
						    return ErrNoOperand{.expr_str = Token::DeTokenize(tokens), .operator_str = Expr::kSymbol};
					    expr.right = std::move(expr_stack.top());
					    expr_stack.pop();

					    if (expr_stack.empty())
						    return ErrNoOperand{.expr_str = Token::DeTokenize(tokens), .operator_str = Expr::kSymbol};
					    expr.left = std::move(expr_stack.top());
					    expr_stack.pop();
				    }
				    return {};
			    },
			    op->m_expr));

			expr_stack.push(std::move(op));
		}

		return {};
	};

	for (std::size_t i = 0; i < tokens.size(); ++i) {
		const auto &token = tokens[i];
		auto token_view = token.GetView();
		if (token_view.front() == '(') {
			for (char _ : token_view)
				token_stack.emplace(nullptr);
		} else if (token_view.front() == ')') {
			for (char _ : token_view) {
				BASIC_UNWRAP(make_new_expr(INT_MIN, false));
				if (token_stack.empty())
					return ErrBracketUnmatched{.expr_str = Token::DeTokenize(tokens)};
				token_stack.pop();
			}
		} else {
			auto &token_expr = token_exprs[i];

			BASIC_UNWRAP(
			    make_new_expr(token_expr->GetPrecedence(), token_expr->GetAssociative() == ExpressionAsso::kLeft));
			token_stack.push(std::move(token_expr));
		}
	}

	BASIC_UNWRAP(make_new_expr(INT_MIN, false));

	if (!token_stack.empty()) {
		if (token_stack.top() == nullptr) // there are still '(' in token stack
			return ErrBracketUnmatched{.expr_str = Token::DeTokenize(tokens)};
		// operator in token stack
		return ErrNoOperand{.expr_str = Token::DeTokenize(tokens), .operator_str = token_stack.top()->Format()};
	}

	if (expr_stack.empty())
		return ErrEmptyExpr{.expr_str = Token::DeTokenize(tokens)};

	std::unique_ptr<Expression> expr = std::move(expr_stack.top());
	expr_stack.pop();

	if (!expr_stack.empty())
		return ErrOrphanExpr{.expr_str = Token::DeTokenize(tokens), .orph_expr_str = expr_stack.top()->Format()};

	return expr;
}

} // namespace basic