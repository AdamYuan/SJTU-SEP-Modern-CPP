#pragma once

#include <QColor>

constexpr QColor kPlayerColors[] = {QColor{255, 0, 0}, QColor{0, 180, 0}, QColor{0, 0, 255}};
constexpr QColor kBlockColors[] = {QColor{40, 40, 40}, QColor{80, 80, 80}, QColor{120, 120, 120}, QColor{160, 160, 160},
                                   QColor{200, 200, 200}};
constexpr QColor kHintColor = QColor{0, 255, 255};

constexpr const char *kEffectSymbols[] = {
    "\uf017", "\uf523", "\uf059", "\uf28b", "\uf362",
};
constexpr const char *kWinnerSymbol = "\uf5a2";
constexpr const char *kFontName = "Font Awesome 6 Free Solid";
