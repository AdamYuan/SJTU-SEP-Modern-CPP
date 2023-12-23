#include "Grid.hpp"

#include <algorithm>
#include <random>

void Grid::Initialize(uint32_t width, uint32_t height) {
	m_width = width;
	m_height = height;
	m_blocks.clear();
	m_blocks.resize(m_width * m_height);
}

void Grid::InitializeRandomized(Random *p_random, uint32_t width, uint32_t height, Block types,
                                std::array<Coord, 2> *p_next_solution) {
	Initialize(width, height);

	std::uniform_int_distribution<Block> type_dis{1, types};

	{
		std::size_t i = 0;
		for (; (i | 1) < m_blocks.size(); i += 2)
			m_blocks[i] = m_blocks[i | 1] = type_dis(*p_random);
		if (i < m_blocks.size())
			m_blocks[i] = type_dis(*p_random);
	}
	Shuffle(p_random, p_next_solution);
}

void Grid::Shuffle(Random *p_random, std::array<Coord, 2> *p_next_solution) {
	do {
		std::shuffle(m_blocks.begin(), m_blocks.end(), *p_random);
	} while (!HasSolution(p_next_solution));
}

bool Grid::is_surrounded(Coord c) const {
	return Get(c.x - 1, c.y) && Get(c.x + 1, c.y) && Get(c.x, c.y - 1) && Get(c.x, c.y + 1);
}
bool Grid::is_pos_h_linked(Coord c1, Coord c2) const {
	if (c1.y != c2.y)
		return false;
	if (c1.y == 0 || c1.y == m_height + 1)
		return true;
	if (c1.x > c2.x)
		std::swap(c1.x, c2.x);
	for (uint32_t x = c1.x + 1; x < c2.x; ++x)
		if (Get(x, c1.y))
			return false;
	return true;
}
bool Grid::is_pos_v_linked(Coord c1, Coord c2) const {
	if (c1.x != c2.x)
		return false;
	if (c1.x == 0 || c1.x == m_width + 1)
		return true;
	if (c1.y > c2.y)
		std::swap(c1.y, c2.y);
	for (uint32_t y = c1.y + 1; y < c2.y; ++y)
		if (Get(c1.x, y))
			return false;
	return true;
}
std::pair<uint32_t, uint32_t> Grid::get_pos_h_bound(Coord c) const {
	uint32_t x1 = c.x, x2 = c.x;
	while (x1 >= 1 && Get(x1 - 1, c.y) == 0)
		--x1;
	while (x2 <= m_width && Get(x2 + 1, c.y) == 0)
		++x2;
	return {x1, x2};
}
std::pair<uint32_t, uint32_t> Grid::get_pos_v_bound(Coord c) const {
	uint32_t y1 = c.y, y2 = c.y;
	while (y1 >= 1 && Get(c.x, y1 - 1) == 0)
		--y1;
	while (y2 <= m_height && Get(c.x, y2 + 1) == 0)
		++y2;
	return {y1, y2};
}

bool Grid::IsLinked(Coord c1, Coord c2, std::vector<Coord> *p_joints) const {
	// Check Types
	if (c1 == c2 || Get(c1) == 0 || Get(c1) != Get(c2))
		return false;

	// Check Neighbours
	if (is_surrounded(c1) || is_surrounded(c2))
		return false;

	// 0 Joints
	if (is_pos_h_linked(c1, c2) || is_pos_v_linked(c1, c2)) {
		if (p_joints)
			*p_joints = {c1, c2};
		return true;
	}

	// 1 Joints
	{
		Coord cc = {c1.x, c2.y};
		if (Get(cc) == 0 && is_pos_v_linked(c1, cc) && is_pos_h_linked(cc, c2)) {
			if (p_joints)
				*p_joints = {c1, cc, c2};
			return true;
		}
		cc = {c2.x, c1.y};
		if (Get(cc) == 0 && is_pos_h_linked(c1, cc) && is_pos_v_linked(cc, c2)) {
			if (p_joints)
				*p_joints = {c1, cc, c2};
			return true;
		}
	}

	// 2 Joints
	{
		auto [cx1l, cx1h] = get_pos_h_bound(c1);
		auto [cx2l, cx2h] = get_pos_h_bound(c2);
		uint32_t cxl = std::max(cx1l, cx2l), cxh = std::min(cx1h, cx2h);
		for (uint32_t cx = cxl; cx <= cxh; ++cx) {
			if (cx == c1.x || cx == c2.x)
				continue;
			Coord cc1 = {cx, c1.y}, cc2 = {cx, c2.y};
			if (is_pos_v_linked(cc1, cc2)) {
				if (p_joints)
					*p_joints = {c1, cc1, cc2, c2};
				return true;
			}
		}
		auto [cy1l, cy1h] = get_pos_v_bound(c1);
		auto [cy2l, cy2h] = get_pos_v_bound(c2);
		uint32_t cyl = std::max(cy1l, cy2l), cyh = std::min(cy1h, cy2h);
		for (uint32_t cy = cyl; cy <= cyh; ++cy) {
			if (cy == c1.y || cy == c2.y)
				continue;
			Coord cc1 = {c1.x, cy}, cc2 = {c2.x, cy};
			if (is_pos_h_linked(cc1, cc2)) {
				if (p_joints)
					*p_joints = {c1, cc1, cc2, c2};
				return true;
			}
		}
	}

	return false;
}

bool Grid::HasSolution(std::array<Coord, 2> *p_next_solution) const {
	for (uint32_t y1 = 1; y1 <= m_height; ++y1)
		for (uint32_t x1 = 1; x1 <= m_width; ++x1) {
			if (Get(x1, y1) == 0)
				continue;
			for (uint32_t y2 = y1; y2 <= m_height; ++y2)
				for (uint32_t x2 = 1; x2 <= m_width; ++x2) {
					if (IsLinked({x1, y1}, {x2, y2}, nullptr)) {
						if (p_next_solution) {
							(*p_next_solution)[0] = {x1, y1};
							(*p_next_solution)[1] = {x2, y2};
						}
						return true;
					}
				}
		}
	return false;
}

Coord Grid::GetRandomSpace(Random *p_random, const std::set<Coord> &exclude_coords) const {
	uint32_t empty_count = 0;
	for (uint32_t y = 0; y <= m_height + 1; ++y)
		for (uint32_t x = 0; x <= m_width + 1; ++x)
			if (Get(x, y) == 0 && exclude_coords.find({x, y}) == exclude_coords.end())
				++empty_count;

	uint32_t sample = std::uniform_int_distribution<uint32_t>{0, empty_count - 1}(*p_random);
	for (uint32_t y = 0, i = 0; y <= m_height + 1; ++y)
		for (uint32_t x = 0; x <= m_width + 1; ++x)
			if (Get(x, y) == 0 && exclude_coords.find({x, y}) == exclude_coords.end() && i++ == sample)
				return {x, y};

	assert(false);
	return {0, 0};
}
