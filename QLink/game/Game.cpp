#include "Game.hpp"

void Game::Start(uint32_t width, uint32_t height, uint32_t initial_time, Block types, uint32_t player_count,
                 const std::vector<EffectType> &available_effects) {
	assert(!available_effects.empty());
	assert((width * height) % 2 == 0);

	m_over = false;
	m_show_hint = false;
	m_paused = false;
	m_time = initial_time;
	m_next_solution = std::array<Coord, 2>{};
	m_grid.InitializeRandomized(&m_random, width, height, types, &m_next_solution.value());

	std::set<Coord> used_coords;
	m_players.clear();
	m_players.reserve(player_count);
	for (uint32_t i = 0; i < player_count; ++i) {
		m_players.emplace_back(m_grid.GetRandomSpace(&m_random, used_coords));
		used_coords.insert(m_players.back().GetPosition());
	}

	m_available_effects = available_effects;
	m_effect_block.coord = m_grid.GetRandomSpace(&m_random, used_coords);
	m_effect_block.type = m_available_effects[m_random() % m_available_effects.size()];
	m_effect_set.Clear();

	clear_update();
}

void Game::Update() {
	do_update();
	clear_update();
}

void Game::do_update() {
	if (m_over)
		return;

	// Process Pause
	if (m_update.toggle_pause) {
		m_paused = !m_paused;
	}
	if (m_paused)
		return;

	// Effect Update
	Effect<>::Update(m_update.next_second, this);

	bool check_next_solution = false, create_next_effect = false;

	for (auto &player : m_players) {
		// Reset Empty Activation
		if (player.m_activation.has_value() && m_grid.Get(player.m_activation.value()) == 0)
			player.m_activation.reset();

		// Move, Activation & Link
		{
			Coord new_pos = {~0u, ~0u};
			if (player.m_update.dx > 0 && player.m_position.x <= m_grid.GetWidth())
				new_pos = {player.m_position.x + 1, player.m_position.y};
			else if (player.m_update.dy > 0 && player.m_position.y <= m_grid.GetHeight())
				new_pos = {player.m_position.x, player.m_position.y + 1};
			else if (player.m_update.dx < 0 && player.m_position.x > 0)
				new_pos = {player.m_position.x - 1, player.m_position.y};
			else if (player.m_update.dy < 0 && player.m_position.y > 0)
				new_pos = {player.m_position.x, player.m_position.y - 1};

			if (~new_pos.x) {
				player.m_link_joints.clear();

				if (m_grid[new_pos]) {
					if (player.m_activation.has_value() &&
					    m_grid.IsLinked(player.m_activation.value(), new_pos, &player.m_link_joints)) {
						m_grid.Set(player.m_activation.value(), 0);
						m_grid.Set(new_pos, 0);
						player.m_activation.reset();
						++player.m_score;
						check_next_solution = true;
					} else {
						player.m_activation = new_pos;
						player.m_link_joints = {};
					}
				} else
					player.m_position = new_pos;
			}
		}

		if (!create_next_effect && player.m_position == m_effect_block.coord) {
			create_next_effect = true;
			Effect<>::AddEffect(m_effect_block.type, this, &player);
		}
	}

	for (auto &player : m_players) {
		// Reset Empty Activation
		if (player.m_activation.has_value() && m_grid.Get(player.m_activation.value()) == 0)
			player.m_activation.reset();
	}

	// Create Another Effect (if the current one is picked)
	if (create_next_effect) {
		std::set<Coord> used_coords;
		for (const auto &player : m_players)
			used_coords.insert(player.GetPosition());
		m_effect_block.coord = m_grid.GetRandomSpace(&m_random, used_coords);
		m_effect_block.type = m_available_effects[m_random() % m_available_effects.size()];
	}

	// Update Time
	if (m_update.next_second && m_time != 0)
		--m_time;

	// Check Game Over
	if (m_time == 0)
		m_over = true;
	else if (check_next_solution) {
		m_next_solution = std::array<Coord, 2>{};
		if (!m_grid.HasSolution(&m_next_solution.value())) {
			m_over = true;
			m_next_solution.reset();
		}
	}
}

void Game::clear_update() {
	m_update = {};
	for (auto &player : m_players)
		player.m_update = {};
}
