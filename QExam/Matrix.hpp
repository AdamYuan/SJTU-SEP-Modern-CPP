//
// Created by adamyuan on 12/23/23.
//

#pragma once

#include <cassert>
#include <vector>

class Matrix {
public:
	struct Coord {
		int x, y;
		inline bool operator==(Coord r) const { return x == r.x && y == r.y; }
		inline bool operator!=(Coord r) const { return x != r.x || y != r.y; }
		inline bool operator<(Coord r) const { return std::tie(x, y) < std::tie(r.x, r.y); }
	};

private:
	int m_width, m_height, m_min_safety;
	std::vector<bool> m_grid;
	std::vector<int> m_safety;
	std::vector<Coord> m_path;

	inline int xy_to_index(int x, int y) const { return y * m_width + x; }
	inline int coord_to_index(Coord c) const { return xy_to_index(c.x, c.y); }

	void solve_safety();
	void solve_path();

public:
	inline Matrix() = default;
	inline Matrix(int width, int height, std::vector<bool> &&grid, int min_safety)
	    : m_width{std::max(width, 0)}, m_height{std::max(height, 0)}, m_grid{std::move(grid)},
	      m_min_safety{min_safety} {
		assert(m_grid.size() == m_width * m_height);
		m_grid.resize(std::max(m_width * m_height, 0));
		m_path.clear();
		if (!m_grid.empty()) {
			solve_safety();
			solve_path();
		}
	}

	template <typename IStream> inline static Matrix Input(IStream &in);

	template <typename Func> inline void ForEachCoord(Func &&func) const {
		for (int y = 0; y < m_height; ++y)
			for (int x = 0; x < m_width; ++x)
				func(Coord{x, y});
	}
	inline bool IsEmpty() const { return m_grid.empty(); }
	inline int GetWidth() const { return m_width; }
	inline int GetHeight() const { return m_height; }
	inline bool IsValidCoord(int x, int y) const { return 0 <= x && x < m_width && 0 <= y && y < m_height; }
	inline bool IsValidCoord(Coord c) const { return IsValidCoord(c.x, c.y); }
	inline bool GetGrid(int x, int y) const { return IsValidCoord(x, y) && m_grid[xy_to_index(x, y)]; }
	inline bool GetGrid(Coord c) const { return GetGrid(c.x, c.y); }
	inline int GetSafety(int x, int y) const { return IsValidCoord(x, y) ? m_safety[xy_to_index(x, y)] : 0; }
	inline int GetSafety(Coord c) const { return GetSafety(c.x, c.y); }
	inline const std::vector<Coord> &GetPath() const { return m_path; }
	inline bool HavePath() const { return !m_path.empty(); }
};

template <typename IStream> Matrix Matrix::Input(IStream &in) {
	int width, height, min_safety;
	std::vector<bool> grid;
	in >> height >> width;
	grid.resize(width * height);
	for (int i = 0; i < grid.size(); ++i) {
		int b = 0;
		in >> b;
		grid[i] = b;
	}
	in >> min_safety;
	return Matrix{width, height, std::move(grid), min_safety};
}
