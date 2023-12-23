#pragma once

#include <QWidget>

#include "game/Game.hpp"

class GameWidget final : public QWidget {
	Q_OBJECT
private:
	inline static constexpr int kBoardHeight = 80;
	inline static constexpr int kBoardPadding = 40;
	inline static constexpr int kBoardSmallPadding = 10;

	const Game &m_game_ref;

	void render_grid();
	void render_board();

public:
	explicit GameWidget(const Game &game_ref, QWidget *parent = nullptr);

protected:
	void paintEvent(QPaintEvent *event) final;
};
