#pragma once

#include "Error.hpp"
#include "Statement.hpp"

#include <map>
#include <memory>
#include <optional>

namespace basic {

class Program {
private:
	std::map<LineID, std::unique_ptr<Statement>> m_statements;

public:
	inline static std::unique_ptr<Program> Create() { return std::make_unique<Program>(); }

	inline RuntimeResult<LineID> GetFirstLine() const {
		if (m_statements.empty())
			return MsgEndOfProgram{};

		return m_statements.begin()->first;
	}
	inline RuntimeResult<LineID> GetNextLine(LineID line) const {
		if (m_statements.empty())
			return MsgEndOfProgram{};

		auto it = m_statements.upper_bound(line);
		if (it == m_statements.end())
			return MsgEndOfProgram{};
		return it->first;
	}
	inline RuntimeResult<void> CheckLine(LineID line) const {
		if (m_statements.empty())
			return MsgEndOfProgram{};

		auto it = m_statements.find(line);
		if (it == m_statements.end())
			return ErrUndefinedLine{.line = line};
		return {};
	}
	inline RuntimeResult<const Statement *> GetStatement(LineID line) const {
		if (m_statements.empty())
			return MsgEndOfProgram{};

		auto it = m_statements.find(line);
		if (it == m_statements.end())
			return ErrUndefinedLine{.line = line};
		return it->second.get();
	}

	inline void InsertStatement(LineID line, std::unique_ptr<Statement> statement) {
		if (statement)
			m_statements[line] = std::move(statement);
	}
	inline void EraseStatement(LineID line) { m_statements.erase(line); }

	inline void Clear() { m_statements.clear(); }

	inline String Format() const {
		String lines;
		for (const auto &it : m_statements)
			lines += std::to_string(it.first) + ' ' + it.second->Format() + '\n';
		return lines;
	}

	inline String FormatAST(const Context *p_state) const {
		String lines;
		for (const auto &it : m_statements)
			lines += std::to_string(it.first) + ' ' + it.second->FormatAST(it.first, p_state);
		return lines;
	}
};

} // namespace basic
