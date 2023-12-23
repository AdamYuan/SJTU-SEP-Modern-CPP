#include "Expression.hpp"

#include "Context.hpp"

namespace basic {

RuntimeResult<Int> ExprVar::Eval(const Context &context) const {
	Int v;
	BASIC_UNWRAP_ASSIGN(v, context.ReadVariable(var));
	return v;
}
RuntimeResult<Int> ExprDiv::Eval(Int l, Int r) const {
	if (r == 0)
		return ErrDivByZero{.zero_expr_str = right->Format()};
	return l / r;
}
RuntimeResult<Int> ExprMod::Eval(Int l, Int r) const {
	if (r == 0)
		return ErrDivByZero{.zero_expr_str = right->Format()};
	return (r + (l % r)) % r;
}
inline static Int fast_pow(Int a, Int b) {
	Int res = 1;
	while (b > 0) {
		if (b & 1)
			res *= a;
		a *= a;
		b >>= 1;
	}
	return res;
}
RuntimeResult<Int> ExprExp::Eval(Int l, Int r) const {
	if (r < 0)
		return ErrExpByNeg{.neg_expr_str = right->Format()};
	return fast_pow(l, r);
}

} // namespace basic
