#include "Effect.hpp"

#include "Game.hpp"
#include "Player.hpp"
#include "Random.hpp"

template <EffectType PushType> void Effect<EffectType::__EFFECT_NUM__>::AddEffect(Game *p_game, Player *p_pick_player) {
	if constexpr (Effect<PushType>::kField == EffectField::kGlobal)
		p_game->m_effect_set.Insert<PushType>(p_game, p_pick_player);
	else if constexpr (Effect<PushType>::kField == EffectField::kLocal)
		p_pick_player->m_effect_set.Insert<PushType>(p_game, p_pick_player);
	else if constexpr (Effect<PushType>::kField == EffectField::kExclude) {
		for (auto &player : p_game->m_players) {
			if (&player == p_pick_player)
				continue;
			player.m_effect_set.Insert<PushType>(p_game, &player);
		}
	}
}
void Effect<EffectType::__EFFECT_NUM__>::AddEffect(EffectType type, Game *p_game, Player *p_pick_player) {
	EffectIterator<>::ForEach([type, p_game, p_pick_player](auto effect) {
		constexpr auto kType = EffectTrait<std::decay_t<decltype(effect)>>::kType;
		if (type == kType)
			AddEffect<kType>(p_game, p_pick_player);
	});
}
void Effect<EffectType::__EFFECT_NUM__>::Update(bool next_second, Game *p_game) {
	for (auto &player : p_game->m_players)
		player.m_effect_set.Update(next_second, p_game, &player);
	p_game->m_effect_set.Update(next_second, p_game, nullptr);
}

void Effect<EffectType::kPlus1S>::OnActive(Game *p_game, Player *p_player) {
	p_game->m_update.next_second = false;
	p_game->m_time += 30;
}

void Effect<EffectType::kShuffle>::OnActive(Game *p_game, Player *p_player) {
	p_game->m_next_solution = std::array<Coord, 2>{};
	p_game->m_grid.Shuffle(&p_game->m_random, &p_game->m_next_solution.value());

	// Move players if overlapped
	std::set<Coord> used_coords;
	for (auto &player : p_game->m_players) {
		player.m_activation.reset();  // De-activate
		player.m_link_joints.clear(); // Clear Joints
		if (p_game->m_grid[player.m_position]) {
			player.m_position = p_game->m_grid.GetRandomSpace(&p_game->m_random, used_coords);
			used_coords.insert(player.m_position);
		}
	}
}

void Effect<EffectType::kHint>::OnActive(Game *p_game, Player *p_player) { p_game->m_show_hint = true; }
void Effect<EffectType::kHint>::OnEnded(Game *p_game, Player *p_player) { p_game->m_show_hint = false; }

void Effect<EffectType::kFreeze>::OnHold(Game *p_game, Player *p_player) {
	p_player->m_update.dx = p_player->m_update.dy = 0;
}

void Effect<EffectType::kDizzy>::OnHold(Game *p_game, Player *p_player) {
	p_player->m_update.dx = -p_player->m_update.dx;
	p_player->m_update.dy = -p_player->m_update.dy;
}
