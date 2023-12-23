#pragma once

#include "Coord.hpp"
#include "Effect.hpp"

#include <algorithm>
#include <cinttypes>
#include <optional>
#include <vector>

class Game;

class Player {
private:
	Coord m_position;
	std::optional<Coord> m_activation;
	std::vector<Coord> m_link_joints;
	EffectSet m_effect_set;
	uint32_t m_score;

	mutable struct {
		int32_t dx = 0, dy = 0;
	} m_update;

	friend class Game;
	template <EffectType> friend class Effect;
	template <typename> friend class Serializer;

public:
	inline Player() = default;
	inline explicit Player(Coord position) : m_position{position}, m_score{0} {}

	inline void Move(int32_t dx, int32_t dy) const { m_update.dx += dx, m_update.dy += dy; }

	inline Coord GetPosition() const { return m_position; }
	inline const EffectSet &GetEffectSet() const { return m_effect_set; }
	inline const std::optional<Coord> &GetActivation() const { return m_activation; }
	inline const std::vector<Coord> &GetLinkJoints() const { return m_link_joints; }
	inline uint32_t GetScore() const { return m_score; }
};
