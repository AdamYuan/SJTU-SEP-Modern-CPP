#include "Matrix.hpp"

#include <algorithm>
#include <climits>
#include <optional>
#include <queue>

void Matrix::solve_safety() {
	m_safety.clear();
	m_safety.resize(m_grid.size());
	for (int i = 0; i < m_safety.size(); ++i)
		m_safety[i] = m_grid[i] ? 0 : INT_MAX;

	std::queue<Coord> que;
	ForEachCoord([this, &que](Coord c) {
		if (GetGrid(c))
			que.push(c);
	});

	while (!que.empty()) {
		Coord c = que.front();
		int s = GetSafety(c);
		que.pop();

		Coord n{};
#define PROP \
	if (IsValidCoord(n)) { \
		int n_idx = coord_to_index(n); \
		if (m_safety[n_idx] > s + 1) { \
			m_safety[n_idx] = s + 1; \
			que.push(n); \
		} \
	}
		n = {c.x - 1, c.y};
		PROP;
		n = {c.x + 1, c.y};
		PROP;
		n = {c.x, c.y - 1};
		PROP;
		n = {c.x, c.y + 1};
		PROP;
#undef PROP
	}
}

void Matrix::solve_path() {
	m_path.clear();

	if (m_safety.front() < m_min_safety)
		return;

	std::vector<std::optional<Coord>> prev(m_safety.size(), std::nullopt);
	std::queue<Coord> que;

	que.push({0, 0});
	prev.front() = {0, 0};

	while (!que.empty()) {
		Coord c = que.front();
		que.pop();

		Coord n{};

#define PROP \
	if (IsValidCoord(n)) { \
		int n_idx = coord_to_index(n); \
		if (m_safety[n_idx] >= m_min_safety && !prev[n_idx]) { \
			prev[n_idx] = c; \
			que.push(n); \
		} \
	}
		n = {c.x - 1, c.y};
		PROP;
		n = {c.x + 1, c.y};
		PROP;
		n = {c.x, c.y - 1};
		PROP;
		n = {c.x, c.y + 1};
		PROP;
#undef PROP
	}
	if (prev.back()) {
		// Path Found
		Coord c = {m_width - 1, m_height - 1}, p = prev.back().value();
		for (;;) {
			m_path.push_back(c);
			if (p == c)
				break;
			c = p;
			p = prev[coord_to_index(c)].value();
		}
		std::reverse(m_path.begin(), m_path.end());
	}
}
