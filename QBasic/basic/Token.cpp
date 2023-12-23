#include "Token.hpp"

namespace basic {

std::vector<Token> Token::Tokenize(StringView line) {
	std::shared_ptr<Char[]> shared_str = std::make_shared<Char[]>(line.length());
	std::copy(line.begin(), line.end(), shared_str.get());

	std::vector<Token> tokens;

	for (std::size_t i = 0; i < line.length();) {
		if (isspace(line[i])) {
			++i;
			continue;
		}

		std::size_t j = i + 1;
		for (; j < line.length(); ++j) {
			if (isspace(line[j]))
				break;

			bool merge = isalnum(line[j - 1]) && isalnum(line[j]) || line[j - 1] == line[j];
			if (!merge)
				break;
		}

		Token token;
		token.m_shared_str = shared_str;
		token.m_begin = i;
		token.m_end = j;
		tokens.push_back(std::move(token));

		i = j;
	}

	return std::move(tokens);
}

} // namespace basic