#pragma once

#include <memory>
#include <queue>
#include <unordered_map>

#include "Config.hpp"
#include "Error.hpp"
#include "Program.hpp"

namespace basic {

class Context {
private:
	std::unordered_map<String, Int> m_variables;
	LineID m_line = -1;

	std::queue<String> m_inputs;
	String m_outputs;
	bool m_terminated = false;

	mutable std::unordered_map<String, Count> m_variable_stats;
	mutable std::unordered_map<LineID, Count> m_line_stats, m_branch_stats;

public:
	inline static RuntimeResult<std::unique_ptr<Context>> Create(const Program &program) {
		LineID first_line;
		BASIC_UNWRAP_ASSIGN(first_line, program.GetFirstLine());

		auto ret = std::make_unique<Context>();
		BASIC_UNWRAP(ret->GotoLine(program, first_line));

		return ret;
	}

	inline RuntimeResult<Int> ReadVariable(const String &var) const {
		auto it = m_variables.find(var);
		if (it == m_variables.end())
			return ErrUndefinedVariable{.var = var};
		++m_variable_stats[var];
		return it->second;
	}
	inline void SetVariable(const String &var, Int val) { m_variables[var] = val; }

	inline LineID GetLine() const { return m_line; }

	inline RuntimeResult<void> GotoLine(const Program &program, LineID line) {
		BASIC_UNWRAP(program.CheckLine(line));
		m_line = line;
		++m_line_stats[line];
		return {};
	}
	inline RuntimeResult<void> NextLine(const Program &program) {
		LineID next_line;
		BASIC_UNWRAP_ASSIGN(next_line, program.GetNextLine(m_line));
		return GotoLine(program, next_line);
	}
	inline RuntimeResult<void> GotoBranchLine(const Program &program, LineID line) {
		++m_branch_stats[m_line];
		return GotoLine(program, line);
	}

	inline void PushInput(StringView string) { m_inputs.emplace(string); }
	inline RuntimeResult<String> PopInput() {
		if (m_inputs.empty())
			return MsgRequestInput{};
		String ret = std::move(m_inputs.front());
		m_inputs.pop();
		return ret;
	}
	inline bool HaveInput() const { return !m_inputs.empty(); }

	inline void PushOutput(StringView string) {
		m_outputs += string;
		m_outputs += '\n';
	}
	inline String PopOutputs() {
		String ret = std::move(m_outputs);
		if (!ret.empty() && ret.back() == '\n')
			ret.pop_back();
		m_outputs.clear();
		return ret;
	}

	inline void Terminate() { m_terminated = true; }
	inline bool IsTerminated() const { return m_terminated; }

	inline Count GetVariableStat(const String &var) const { return m_variable_stats[var]; }
	inline Count GetLineStat(LineID line) const { return m_line_stats[line]; }
	inline Count GetBranchStat(LineID line) const { return m_branch_stats[line]; }
};

} // namespace basic
