#include "Statement.hpp"

#include "Context.hpp"

namespace basic {

// Runners
RuntimeResult<void> StmtRem::Run(const Program &program, Context *p_context) {
	BASIC_UNWRAP(p_context->NextLine(program));
	return {};
}
RuntimeResult<void> StmtInput::Run(const Program &program, Context *p_context) const {
	String input;
	BASIC_UNWRAP_ASSIGN(input, p_context->PopInput());

	std::vector<Token> tokens = Token::Tokenize(input);
	if (tokens.empty() || tokens.size() > 2)
		return ErrInvalidInput{input};

	Int value;
	if (tokens.size() == 1) {
		if (!tokens[0].IsDigit())
			return ErrInvalidInput{input};
		value = tokens[0].ToDigit<Int>();
	} else {
		if (!tokens[1].IsDigit())
			return ErrInvalidInput{input};
		value = tokens[1].ToDigit<Int>();

		StringView symbol = tokens[0].GetView();
		if (symbol == "-")
			value = -value;
		else if (symbol != "+")
			return ErrInvalidInput{input};
	}

	p_context->SetVariable(var, value);

	BASIC_UNWRAP(p_context->NextLine(program));
	return {};
}
RuntimeResult<void> StmtPrint::Run(const Program &program, Context *p_context) const {
	Int val;
	BASIC_UNWRAP_ASSIGN(val, this->expr->Eval(*p_context));
	p_context->PushOutput(std::to_string(val));
	BASIC_UNWRAP(p_context->NextLine(program));
	return MsgPrint{};
}
RuntimeResult<void> StmtLet::Run(const Program &program, Context *p_context) const {
	Int value;
	BASIC_UNWRAP_ASSIGN(value, expr->Eval(*p_context));
	p_context->SetVariable(var, value);
	BASIC_UNWRAP(p_context->NextLine(program));
	return {};
}
RuntimeResult<void> StmtGoto::Run(const Program &program, Context *p_context) const {
	BASIC_UNWRAP(p_context->GotoLine(program, line));
	return {};
}
RuntimeResult<void> StmtIf::Run(const Program &program, Context *p_context) const {
	Int value_l, value_r;
	BASIC_UNWRAP_ASSIGN(value_l, expr_l->Eval(*p_context));
	BASIC_UNWRAP_ASSIGN(value_r, expr_r->Eval(*p_context));
	bool branch = false;
	if (cmp == '<')
		branch = value_l < value_r;
	else if (cmp == '=')
		branch = value_l == value_r;
	else
		branch = value_l > value_r;

	BASIC_UNWRAP(branch ? p_context->GotoBranchLine(program, line_then) : p_context->NextLine(program));
	return {};
}
RuntimeResult<void> StmtEnd::Run(const Program &program, Context *p_context) { return MsgEndOfProgram{}; }

// Format AST
#define AST_STMT_END (p_context ? "[execute:" + std::to_string(p_context->GetLineStat(line)) + "]" : "")
#define AST_VAR_END (p_context ? "[use:" + std::to_string(p_context->GetVariableStat(this->var)) + "]" : "")
String StmtRem::FormatAST(LineID line, const Context *p_context) const {
	return AST_STMT_END + "\n" + (this->comment.empty() ? "" : kASTFormatAlign + this->comment + "\n");
}
String StmtInput::FormatAST(LineID line, const Context *p_context) const {
	return AST_STMT_END + "\n" + kASTFormatAlign + this->var + " " + AST_VAR_END + "\n";
}
String StmtPrint::FormatAST(LineID line, const Context *p_context) const {
	return AST_STMT_END + "\n" + this->expr->FormatAST(kASTFormatAlign);
}
String StmtLet::FormatAST(LineID line, const Context *p_context) const {
	return "= " + AST_STMT_END + "\n" + kASTFormatAlign + this->var + " " + AST_VAR_END + "\n" +
	       this->expr->FormatAST(kASTFormatAlign);
}
String StmtGoto::FormatAST(LineID, const Context *p_context) const {
	return AST_STMT_END + "\n" + kASTFormatAlign + std::to_string(this->line) + "\n";
}
String StmtIf::FormatAST(LineID line, const Context *p_context) const {
	String line_end_str;
	if (p_context) {
		Count true_cnt = p_context->GetBranchStat(line);
		Count false_cnt = p_context->GetLineStat(line) - true_cnt;
		line_end_str = "[true:" + std::to_string(true_cnt) + "] [false:" + std::to_string(false_cnt) + "]";
	}
	return "THEN " + line_end_str + "\n" + this->expr_l->FormatAST(kASTFormatAlign) + kASTFormatAlign + this->cmp +
	       "\n" + this->expr_r->FormatAST(kASTFormatAlign) + kASTFormatAlign + std::to_string(line_then) + "\n";
}
String StmtEnd::FormatAST(LineID line, const Context *p_context) { return AST_STMT_END + "\n"; }

} // namespace basic
