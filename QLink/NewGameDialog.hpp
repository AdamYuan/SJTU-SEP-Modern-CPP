#pragma once

#include <QDialog>

#include "game/Game.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class NewGameDialog;
}
QT_END_NAMESPACE

class NewGameDialog final : public QDialog {
	Q_OBJECT
public:
	explicit NewGameDialog(QWidget *parent = nullptr);

	~NewGameDialog() final;

signals:
	void newGame(uint32_t width, uint32_t height, bool multi_player);

private:
	Ui::NewGameDialog *m_ui;
};
