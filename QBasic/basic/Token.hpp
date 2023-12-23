#pragma once

#include "Config.hpp"
#include <algorithm>
#include <cctype>
#include <memory>
#include <span>
#include <vector>

namespace basic {

class Token {
private:
	std::shared_ptr<Char[]> m_shared_str;
	std::size_t m_begin{}, m_end{};

public:
	inline StringView GetView() const { return {m_shared_str.get() + m_begin, m_shared_str.get() + m_end}; }
	inline String GetString() const { return String{m_shared_str.get() + m_begin, m_shared_str.get() + m_end}; }

	inline bool IsDigit() const {
		StringView view = GetView();
		return std::all_of(view.begin(), view.end(), isdigit);
	}
	template <typename T> inline T ToDigit() const {
		T ret = 0;
		StringView view = GetView();
		for (char c : view)
			ret = ret * 10 + c - '0';
		return ret;
	}
	// TODO: support '_'
	inline bool IsVariable() const {
		StringView view = GetView();
		return isalpha(view.front()) && std::all_of(view.begin() + 1, view.end(), isalnum);
	}
	inline bool IsKeyword(StringView keyword) const { return GetView() == keyword; }

	static std::vector<Token> Tokenize(StringView line);

	inline static String DeTokenize(std::span<const Token> tokens) {
		if (tokens.empty())
			return {};
		return DeTokenize(tokens.front(), tokens.back());
	}
	inline static String DeTokenize(const Token &l, const Token &r) {
		if (l.m_shared_str != r.m_shared_str || l.m_begin >= r.m_end)
			return {};
		return {l.m_shared_str.get() + l.m_begin, l.m_shared_str.get() + r.m_end};
	}
};

} // namespace basic
