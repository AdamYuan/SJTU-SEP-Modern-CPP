#pragma once

#include <array>
#include <cinttypes>
#include <memory>
#include <tuple>

#include "Coord.hpp"

class Player;
class Game;

enum class EffectType { kPlus1S, kShuffle, kHint, kFreeze, kDizzy, __EFFECT_NUM__ };
enum class EffectField { kGlobal, kLocal, kExclude };

struct EffectBlock {
	Coord coord;
	EffectType type;
};

template <EffectType Type = EffectType::__EFFECT_NUM__> struct Effect;

template <> struct Effect<EffectType::__EFFECT_NUM__> {
private:
	template <EffectType PushType> static void AddEffect(Game *p_game, Player *p_pick_player);

public:
	static void AddEffect(EffectType type, Game *p_game, Player *p_pick_player);
	static void Update(bool next_second, Game *p_game);
};
template <> struct Effect<EffectType::kPlus1S> {
	inline static constexpr uint32_t kDuration = 0;
	inline static constexpr EffectField kField = EffectField::kGlobal;
	static void OnActive(Game *p_game, Player *p_player);
};
template <> struct Effect<EffectType::kShuffle> {
	inline static constexpr uint32_t kDuration = 0;
	inline static constexpr EffectField kField = EffectField::kGlobal;
	static void OnActive(Game *p_game, Player *p_player);
};
template <> struct Effect<EffectType::kHint> {
	inline static constexpr uint32_t kDuration = 10;
	inline static constexpr EffectField kField = EffectField::kGlobal;
	static void OnActive(Game *p_game, Player *p_player);
	static void OnHold(Game *p_game, Player *p_player) {}
	static void OnEnded(Game *p_game, Player *p_player);
};
template <> struct Effect<EffectType::kFreeze> {
	inline static constexpr uint32_t kDuration = 3;
	inline static constexpr EffectField kField = EffectField::kExclude;
	static void OnActive(Game *p_game, Player *p_player) {}
	static void OnHold(Game *p_game, Player *p_player);
	static void OnEnded(Game *p_game, Player *p_player) {}
};
template <> struct Effect<EffectType::kDizzy> {
	inline static constexpr uint32_t kDuration = 10;
	inline static constexpr EffectField kField = EffectField::kExclude;
	static void OnActive(Game *p_game, Player *p_player) {}
	static void OnHold(Game *p_game, Player *p_player);
	static void OnEnded(Game *p_game, Player *p_player) {}
};

template <EffectType Type = static_cast<EffectType>(0)> struct EffectIterator {
	template <typename Func> static inline void ForEach(Func &&func) {
		func(Effect<Type>{});
		EffectIterator<static_cast<EffectType>(static_cast<int>(Type) + 1)>::ForEach(std::forward<Func>(func));
	}
};
template <> struct EffectIterator<EffectType::__EFFECT_NUM__> {
	template <typename Func> static inline void ForEach(Func &&func) {}
};

template <typename> struct EffectTrait;
template <EffectType Type> struct EffectTrait<Effect<Type>> {
	inline static constexpr EffectType kType = Type;
};

class EffectSet {
private:
	static constexpr std::size_t kEffectNum = (std::size_t)EffectType::__EFFECT_NUM__;
	using ArrayType = std::array<uint32_t, kEffectNum>;

	ArrayType m_effects{};

	template <typename> friend class Serializer;

public:
	inline void Clear() { std::fill(m_effects.begin(), m_effects.end(), 0); }
	template <EffectType Type> inline void Insert(Game *p_game, Player *p_player) {
		if (m_effects[static_cast<int>(Type)] == 0)
			Effect<Type>::OnActive(p_game, p_player);
		m_effects[static_cast<int>(Type)] += Effect<Type>::kDuration;
	}
	inline void Insert(EffectType type, Game *p_game, Player *p_player) {
		EffectIterator<>::ForEach([this, type, p_game, p_player](auto effect) {
			constexpr auto kType = EffectTrait<std::decay_t<decltype(effect)>>::kType;
			if (type == kType)
				Insert<kType>(p_game, p_player);
		});
	}
	inline void Update(bool next_second, Game *p_game, Player *p_player) {
		EffectIterator<>::ForEach([this, next_second, p_game, p_player](auto effect) {
			constexpr auto kType = EffectTrait<std::decay_t<decltype(effect)>>::kType;
			if constexpr (Effect<kType>::kDuration > 0)
				if (m_effects[static_cast<int>(kType)]) {
					if (next_second)
						--m_effects[static_cast<int>(kType)];

					if (m_effects[static_cast<int>(kType)])
						Effect<kType>::OnHold(p_game, p_player);
					else
						Effect<kType>::OnEnded(p_game, p_player);
				}
		});
	}
	inline uint32_t GetCount() const {
		uint32_t cnt = 0;
		EffectIterator<>::ForEach([this, &cnt](auto effect) {
			constexpr auto kType = EffectTrait<std::decay_t<decltype(effect)>>::kType;
			if constexpr (Effect<kType>::kDuration > 0)
				cnt += m_effects[static_cast<int>(kType)] != 0;
		});
		return cnt;
	}
	template <typename Func> inline void ForEach(Func &&func) const {
		EffectIterator<>::ForEach([this, func](auto effect) {
			constexpr auto kType = EffectTrait<std::decay_t<decltype(effect)>>::kType;
			if constexpr (Effect<kType>::kDuration > 0) {
				auto effect_time = m_effects[static_cast<int>(kType)];
				if (effect_time)
					func(kType, effect_time);
			}
		});
	}
};
