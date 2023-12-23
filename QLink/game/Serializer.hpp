#pragma once

#include "Game.hpp"

#include <cstring>

template <typename> struct Serializer;

template <> struct Serializer<uint8_t> {
	template <typename Stream> inline static void Write(Stream &&ostr, uint8_t val) {
		const char *src = (const char *)(&val);
		ostr.write(src, 1);
	}
	template <typename Stream> inline static uint8_t Read(Stream &&istr) {
		uint8_t val;
		istr.read((char *)(&val), 1);
		return val;
	}
};

template <> struct Serializer<bool> {
	template <typename Stream> inline static void Write(Stream &&ostr, bool val) {
		Serializer<uint8_t>::Write(ostr, val ? 1 : 0);
	}
	template <typename Stream> inline static bool Read(Stream &&istr) { return Serializer<uint8_t>::Read(istr); }
};

template <> struct Serializer<uint32_t> {
	template <typename Stream> inline static void Write(Stream &&ostr, uint32_t val) {
		Serializer<uint8_t>::Write(ostr, val & 0xffu);
		Serializer<uint8_t>::Write(ostr, (val >> 8u) & 0xffu);
		Serializer<uint8_t>::Write(ostr, (val >> 16u) & 0xffu);
		Serializer<uint8_t>::Write(ostr, (val >> 24u) & 0xffu);
	}
	template <typename Stream> inline static uint32_t Read(Stream &&istr) {
		uint32_t v0 = Serializer<uint8_t>::Read(istr);
		uint32_t v1 = Serializer<uint8_t>::Read(istr);
		uint32_t v2 = Serializer<uint8_t>::Read(istr);
		uint32_t v3 = Serializer<uint8_t>::Read(istr);
		return v0 | (v1 << 8u) | (v2 << 16u) | (v3 << 24u);
	}
};

template <> struct Serializer<EffectType> {
	template <typename Stream> inline static void Write(Stream &&ostr, const EffectType &val) {
		Serializer<uint32_t>::Write(ostr, static_cast<uint32_t>(val));
	}
	template <typename Stream> inline static EffectType Read(Stream &&istr) {
		uint32_t v = Serializer<uint32_t>::Read(istr);
		return static_cast<EffectType>(v);
	}
};

template <> struct Serializer<Coord> {
	template <typename Stream> inline static void Write(Stream &&ostr, const Coord &val) {
		Serializer<uint32_t>::Write(ostr, val.x);
		Serializer<uint32_t>::Write(ostr, val.y);
	}
	template <typename Stream> inline static Coord Read(Stream &&istr) {
		uint32_t x = Serializer<uint32_t>::Read(istr);
		uint32_t y = Serializer<uint32_t>::Read(istr);
		return {x, y};
	}
};

template <> struct Serializer<EffectBlock> {
	template <typename Stream> inline static void Write(Stream &&ostr, const EffectBlock &val) {
		Serializer<Coord>::Write(ostr, val.coord);
		Serializer<EffectType>::Write(ostr, val.type);
	}
	template <typename Stream> inline static EffectBlock Read(Stream &&istr) {
		EffectBlock ret = {};
		ret.coord = Serializer<Coord>::Read(istr);
		ret.type = Serializer<EffectType>::Read(istr);
		return ret;
	}
};

template <typename T, std::size_t S> struct Serializer<std::array<T, S>> {
	template <typename Stream> inline static void Write(Stream &&ostr, const std::array<T, S> &val) {
		for (std::size_t i = 0; i < S; ++i)
			Serializer<T>::Write(ostr, val[i]);
	}
	template <typename Stream> inline static std::array<T, S> Read(Stream &&istr) {
		std::array<T, S> ret;
		for (std::size_t i = 0; i < S; ++i)
			ret[i] = Serializer<T>::Read(istr);
		return ret;
	}
};

template <typename T> struct Serializer<std::vector<T>> {
	template <typename Stream> inline static void Write(Stream &&ostr, const std::vector<T> &val) {
		Serializer<uint32_t>::Write(ostr, (uint32_t)val.size());
		for (const auto &i : val)
			Serializer<T>::Write(ostr, i);
	}
	template <typename Stream> inline static std::vector<T> Read(Stream &&istr) {
		std::size_t size = Serializer<uint32_t>::Read(istr);
		std::vector<T> ret;
		ret.reserve(size);
		while (size--)
			ret.push_back(Serializer<T>::Read(istr));
		return ret;
	}
};

template <typename T> struct Serializer<std::optional<T>> {
	template <typename Stream> inline static void Write(Stream &&ostr, const std::optional<T> &val) {
		Serializer<uint8_t>::Write(ostr, val.has_value() ? 1 : 0);
		if (val.has_value())
			Serializer<T>::Write(ostr, val.value());
	}
	template <typename Stream> inline static std::optional<T> Read(Stream &&istr) {
		uint8_t has_value = Serializer<uint8_t>::Read(istr);
		if (has_value)
			return std::optional<T>{Serializer<T>::Read(istr)};
		return std::optional<T>{};
	}
};

template <> struct Serializer<EffectSet> {
	template <typename Stream> inline static void Write(Stream &&ostr, const EffectSet &val) {
		Serializer<EffectSet::ArrayType>::Write(ostr, val.m_effects);
	}
	template <typename Stream> inline static EffectSet Read(Stream &&istr) {
		EffectSet ret = {};
		ret.m_effects = Serializer<EffectSet::ArrayType>::Read(istr);
		return ret;
	}
};

template <> struct Serializer<Player> {
	template <typename Stream> inline static void Write(Stream &&ostr, const Player &val) {
		Serializer<Coord>::Write(ostr, val.m_position);
		Serializer<std::optional<Coord>>::Write(ostr, val.m_activation);
		Serializer<std::vector<Coord>>::Write(ostr, val.m_link_joints);
		Serializer<EffectSet>::Write(ostr, val.m_effect_set);
		Serializer<uint32_t>::Write(ostr, val.m_score);
	}
	template <typename Stream> inline static Player Read(Stream &&istr) {
		Player ret = {};
		ret.m_position = Serializer<Coord>::Read(istr);
		ret.m_activation = Serializer<std::optional<Coord>>::Read(istr);
		ret.m_link_joints = Serializer<std::vector<Coord>>::Read(istr);
		ret.m_effect_set = Serializer<EffectSet>::Read(istr);
		ret.m_score = Serializer<uint32_t>::Read(istr);
		return ret;
	}
};

template <> struct Serializer<Grid> {
	template <typename Stream> inline static void Write(Stream &&ostr, const Grid &val) {
		Serializer<uint32_t>::Write(ostr, val.m_width);
		Serializer<uint32_t>::Write(ostr, val.m_height);
		Serializer<std::vector<Block>>::Write(ostr, val.m_blocks);
	}
	template <typename Stream> inline static Grid Read(Stream &&istr) {
		Grid ret = {};
		ret.m_width = Serializer<uint32_t>::Read(istr);
		ret.m_height = Serializer<uint32_t>::Read(istr);
		ret.m_blocks = Serializer<std::vector<Block>>::Read(istr);
		return ret;
	}
};

template <> struct Serializer<Game> {
	inline static constexpr char kVersionStr[] = "QLink1.0";
	template <typename Stream> inline static void Write(Stream &&ostr, const Game &val) {
		ostr.write(kVersionStr, sizeof(kVersionStr));
		Serializer<uint32_t>::Write(ostr, val.m_time);
		Serializer<Grid>::Write(ostr, val.m_grid);
		Serializer<std::vector<Player>>::Write(ostr, val.m_players);
		Serializer<std::optional<std::array<Coord, 2>>>::Write(ostr, val.m_next_solution);
		Serializer<bool>::Write(ostr, val.m_over);
		Serializer<bool>::Write(ostr, val.m_show_hint);
		Serializer<bool>::Write(ostr, val.m_paused);
		Serializer<std::vector<EffectType>>::Write(ostr, val.m_available_effects);
		Serializer<EffectSet>::Write(ostr, val.m_effect_set);
		Serializer<EffectBlock>::Write(ostr, val.m_effect_block);
	}
	template <typename Stream> static Game Read(Stream &&istr) {
		Game ret{};
		char version_str[sizeof(kVersionStr)];
		istr.read(version_str, sizeof(kVersionStr));
		if (strcmp(kVersionStr, version_str) != 0)
			return ret;
		ret.m_time = Serializer<uint32_t>::Read(istr);
		ret.m_grid = Serializer<Grid>::Read(istr);
		ret.m_players = Serializer<std::vector<Player>>::Read(istr);
		ret.m_next_solution = Serializer<std::optional<std::array<Coord, 2>>>::Read(istr);
		ret.m_over = Serializer<bool>::Read(istr);
		ret.m_show_hint = Serializer<bool>::Read(istr);
		ret.m_paused = Serializer<bool>::Read(istr);
		ret.m_available_effects = Serializer<std::vector<EffectType>>::Read(istr);
		ret.m_effect_set = Serializer<EffectSet>::Read(istr);
		ret.m_effect_block = Serializer<EffectBlock>::Read(istr);
		return ret;
	}
};
