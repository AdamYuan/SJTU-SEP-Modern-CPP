#include "GameWidget.hpp"
#include "Style.hpp"

#include <algorithm>

#include <QKeyEvent>
#include <QPainter>
#include <QString>
#include <QTimer>

GameWidget::GameWidget(const Game &game_ref, QWidget *parent) : m_game_ref{game_ref}, QWidget(parent) {
	setMinimumHeight(kBoardHeight);
}

void GameWidget::paintEvent(QPaintEvent *event) {
	if (!m_game_ref.IsValid())
		return;
	render_board();
	render_grid();
}

void GameWidget::render_board() {
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	// Draw Count Down
	painter.setFont(QFont(kFontName, 40));
	painter.setPen(QColor{0, 0, 0});

	if (m_game_ref.IsOver()) {
		const char *over_str = m_game_ref.GetNextSolution().has_value() ? "TIME OVER" : "UNSOLVABLE";
		int tw = painter.fontMetrics().boundingRect(over_str).width();
		painter.drawText(width() - kBoardPadding - tw, 0, tw, kBoardHeight, Qt::AlignVCenter, over_str);
	} else {
		auto time_str = QString::number(m_game_ref.GetTime());
		if (m_game_ref.IsPaused())
			time_str += " PAUSE";
		int tw = painter.fontMetrics().boundingRect(time_str).width();
		painter.drawText(width() - kBoardPadding - tw, 0, tw, kBoardHeight, Qt::AlignVCenter, time_str);
	}

	// Fetch Winner if Game is Over
	uint32_t max_score = -1;
	if (m_game_ref.IsOver() && m_game_ref.GetPlayers().size() > 1) {
		max_score = 0;
		for (const auto &player : m_game_ref.GetPlayers())
			max_score = std::max(max_score, player.GetScore());
	}

	{ // Draw Player Scores and Effects
		int x = kBoardPadding, tw;
		const QColor *p_cur_player_color = kPlayerColors;
		for (const auto &player : m_game_ref.GetPlayers()) {
			painter.setPen(*p_cur_player_color);
			painter.setFont(QFont(kFontName, 40));
			auto score_str = QString::number(player.GetScore());
			tw = painter.fontMetrics().boundingRect(score_str).width();
			painter.drawText(x, 0, tw, kBoardHeight, Qt::AlignVCenter, score_str);
			x += tw;

			if (!m_game_ref.IsOver()) {
				// Draw Effects Owned by Players
				painter.setFont(QFont(kFontName, 20));
				const auto draw_effect = [&painter, &x](EffectType type, uint32_t time) {
					x += kBoardSmallPadding;
					const char *effect_str = kEffectSymbols[static_cast<unsigned>(type)];
					int tw = painter.fontMetrics().boundingRect(effect_str).width();
					painter.drawText(x, 0, tw, kBoardHeight, Qt::AlignVCenter, effect_str);
					x += tw;
				};
				player.GetEffectSet().ForEach(draw_effect);
				m_game_ref.GetEffectSet().ForEach(draw_effect);
			} else if (player.GetScore() == max_score) {
				// Draw Winner Indicator if Game Ended
				painter.setFont(QFont(kFontName, 30));
				x += kBoardSmallPadding;
				tw = painter.fontMetrics().boundingRect(kWinnerSymbol).width();
				painter.drawText(x, 0, tw, kBoardHeight, Qt::AlignVCenter, kWinnerSymbol);
				x += tw;
			}

			x += kBoardPadding;
			++p_cur_player_color;
		}
	}
}

void GameWidget::render_grid() {
	constexpr int kGridUnit = 20;
	const int kGridWidth = kGridUnit * ((int)m_game_ref.GetGrid().GetWidth() + 2);
	const int kGridHeight = kGridUnit * ((int)m_game_ref.GetGrid().GetHeight() + 2);

	QPainter painter(this);

	{ // Set Perspective
		int w = width(), h = std::max(0, height() - kBoardHeight);
		if (kGridWidth * h < kGridHeight * w) {
			painter.translate(w * 0.5 - h * (qreal)kGridWidth / kGridHeight * 0.5, kBoardHeight);
			qreal scale = h / (qreal)kGridHeight;
			painter.scale(scale, scale);
		} else {
			painter.translate(0, kBoardHeight + h * 0.5 - w * (qreal)kGridHeight / kGridWidth * 0.5);
			qreal scale = w / (qreal)kGridWidth;
			painter.scale(scale, scale);
		}
	}

	const auto draw_cube = [&painter](Coord c) {
		const QPoint base{(int)c.x * kGridUnit, (int)c.y * kGridUnit};
		const QPoint square[] = {base, base + QPoint{kGridUnit, 0}, base + QPoint{kGridUnit, kGridUnit},
		                         base + QPoint{0, kGridUnit}};
		painter.drawConvexPolygon(square, 4);
	};

	// Draw Blocks
	painter.setPen(Qt::PenStyle::NoPen);
	for (uint32_t y = 1; y <= m_game_ref.GetGrid().GetHeight(); ++y)
		for (uint32_t x = 1; x <= m_game_ref.GetGrid().GetWidth(); ++x) {
			Block blk = m_game_ref.GetGrid()[{x, y}];
			if (blk == 0)
				continue;
			painter.setBrush(kBlockColors[blk - 1]);
			draw_cube({x, y});
		}

	painter.setRenderHint(QPainter::Antialiasing, true);

	// Draw Boundary
	{
		painter.setPen(QColor{0, 0, 0});
		painter.setBrush(Qt::NoBrush);
		const QPoint square[] = {{0, 0}, {kGridWidth, 0}, {kGridWidth, kGridHeight}, {0, kGridHeight}};
		painter.drawConvexPolygon(square, 4);
	}

	// Draw Players
	const QColor *p_cur_player_color = kPlayerColors;
	for (const auto &player : m_game_ref.GetPlayers()) {
		const QPoint base{(int)player.GetPosition().x * kGridUnit, (int)player.GetPosition().y * kGridUnit};

		painter.setPen(Qt::NoPen);
		painter.setBrush(*p_cur_player_color);
		painter.drawEllipse(base + QPoint{kGridUnit, kGridUnit} / 2, kGridUnit / 3, kGridUnit / 3);

		if (player.GetActivation().has_value()) {
			Coord a = player.GetActivation().value();
			painter.setPen(*p_cur_player_color);
			painter.setBrush(Qt::NoBrush);
			draw_cube(a);
		}
		if (!player.GetLinkJoints().empty()) {
			for (std::size_t i = 1; i < player.GetLinkJoints().size(); ++i) {
				Coord c1 = player.GetLinkJoints()[i - 1], c2 = player.GetLinkJoints()[i];
				painter.setPen(*p_cur_player_color);
				painter.setBrush(Qt::NoBrush);
				painter.drawLine((int)c1.x * kGridUnit + kGridUnit / 2, (int)c1.y * kGridUnit + kGridUnit / 2,
				                 (int)c2.x * kGridUnit + kGridUnit / 2, (int)c2.y * kGridUnit + kGridUnit / 2);
			}
		}

		++p_cur_player_color;
	}

	// Draw Hint
	if (m_game_ref.ShowHint() && m_game_ref.GetNextSolution().has_value()) {
		QPen hint_pen{kHintColor};
		hint_pen.setStyle(Qt::PenStyle::DotLine);
		painter.setPen(hint_pen);
		painter.setBrush(Qt::NoBrush);
		draw_cube(m_game_ref.GetNextSolution().value()[0]);
		draw_cube(m_game_ref.GetNextSolution().value()[1]);
	}

	// Draw Effect Block
	painter.setPen(QColor{0, 0, 0});
	painter.setFont(QFont(kFontName, 11));
	painter.drawText((int)m_game_ref.GetEffectBlock().coord.x * kGridUnit,
	                 (int)m_game_ref.GetEffectBlock().coord.y * kGridUnit, kGridUnit, kGridUnit, Qt::AlignCenter,
	                 kEffectSymbols[static_cast<unsigned>(m_game_ref.GetEffectBlock().type)]);
}
