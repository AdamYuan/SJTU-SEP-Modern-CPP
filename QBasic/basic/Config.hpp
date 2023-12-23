#pragma once

#include <cinttypes>
#include <string>
#include <variant>

namespace basic {

using Int = int64_t;
using Char = char;
using String = std::string;
using StringView = std::string_view;
using Count = uint32_t;
using LineID = uint32_t;

template <typename> struct VariantIterator;
template <typename... Types> struct VariantIterator<std::variant<Types...>> {
	// return true as break;
	template <typename Func> inline static void Run(Func &&func) { (func(Types{}) || ...); }
};

constexpr const char *kASTFormatAlign = "  ";

} // namespace basic
