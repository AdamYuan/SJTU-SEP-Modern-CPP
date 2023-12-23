#pragma once

#include <vector>

#include "Effect.hpp"
#include "Grid.hpp"
#include "Player.hpp"
#include "Random.hpp"

class Game {
private:
	Random m_random;
	uint32_t m_time{};
	Grid m_grid;
	std::vector<Player> m_players;
	std::optional<std::array<Coord, 2>> m_next_solution;
	bool m_over{}, m_show_hint{}, m_paused{};

	std::vector<EffectType> m_available_effects;
	EffectSet m_effect_set;
	EffectBlock m_effect_block{};

	mutable struct {
		bool next_second = false, toggle_pause = false;
	} m_update;

	template <EffectType> friend class Effect;
	template <typename> friend class Serializer;

	void do_update();
	void clear_update();

public:
	inline Game() : m_random{std::random_device{}()} {}
	inline bool IsValid() const { return !m_players.empty() && m_grid.GetWidth() && m_grid.GetHeight(); }
	inline const Grid &GetGrid() const { return m_grid; }
	inline uint32_t GetTime() const { return m_time; }
	inline void NextSecond() const { m_update.next_second = true; }
	inline void TogglePause() const { m_update.toggle_pause = true; }
	inline const std::vector<Player> &GetPlayers() const { return m_players; }
	inline bool IsOver() const { return m_over; }
	inline bool IsPaused() const { return m_paused; }
	inline bool ShowHint() const { return m_show_hint; }
	inline const EffectSet &GetEffectSet() const { return m_effect_set; }
	inline const EffectBlock &GetEffectBlock() const { return m_effect_block; }
	inline const std::vector<EffectType> &GetAvailableEffects() const { return m_available_effects; }
	inline const std::optional<std::array<Coord, 2>> &GetNextSolution() const { return m_next_solution; }

	void Start(uint32_t width, uint32_t height, uint32_t initial_time, Block types, uint32_t player_count,
	           const std::vector<EffectType> &available_effects);
	void Update();
};
