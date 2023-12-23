#pragma once

#include <array>
#include <cassert>
#include <cinttypes>
#include <set>
#include <vector>

#include "Block.hpp"
#include "Coord.hpp"
#include "Random.hpp"

class Grid {
private:
	uint32_t m_width, m_height;
	std::vector<Block> m_blocks;

	inline bool is_surrounded(Coord c) const;
	inline bool is_pos_h_linked(Coord c1, Coord c2) const;
	inline bool is_pos_v_linked(Coord c1, Coord c2) const;
	inline std::pair<uint32_t, uint32_t> get_pos_h_bound(Coord c) const;
	inline std::pair<uint32_t, uint32_t> get_pos_v_bound(Coord c) const;

	template <typename> friend class Serializer;

public:
	inline Block Get(uint32_t x, uint32_t y) const {
		return 1 <= x && x <= m_width && 1 <= y && y <= m_height ? m_blocks[(y - 1) * m_width + x - 1] : 0;
	}
	inline Block Get(Coord c) const { return Get(c.x, c.y); }
	inline void Set(uint32_t x, uint32_t y, Block b) {
		if (1 <= x && x <= m_width && 1 <= y && y <= m_height)
			m_blocks[(y - 1) * m_width + x - 1] = b;
	}
	inline void Set(Coord c, Block b) { Set(c.x, c.y, b); }
	inline Block operator[](Coord c) const { return Get(c); }

	inline uint32_t GetWidth() const { return m_width; }
	inline uint32_t GetHeight() const { return m_height; }

	void Initialize(uint32_t width, uint32_t height);
	void InitializeRandomized(Random *p_random, uint32_t width, uint32_t height, Block types,
	                          std::array<Coord, 2> *p_next_solution);
	void Shuffle(Random *p_random, std::array<Coord, 2> *p_next_solution);
	bool IsLinked(Coord c1, Coord c2, std::vector<Coord> *p_joints) const;
	bool HasSolution(std::array<Coord, 2> *p_next_solution) const;
	Coord GetRandomSpace(Random *p_random, const std::set<Coord> &exclude_coords) const;
};
