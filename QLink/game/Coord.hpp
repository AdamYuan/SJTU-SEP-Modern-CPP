#pragma once

#include <cinttypes>

struct Coord {
	uint32_t x, y;
	inline bool operator==(Coord r) const { return x == r.x && y == r.y; }
	inline bool operator<(Coord r) const { return x < r.x || (x == r.x && y < r.y); }
};
