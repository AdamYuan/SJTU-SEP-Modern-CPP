#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "game/Game.hpp"
#include "GameWidget.hpp"
#include "NewGameDialog.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow final : public QMainWindow {
	Q_OBJECT

private:
	Game m_game;
	QTimer *m_timer;
	GameWidget *m_game_grid_widget;
	NewGameDialog *m_new_game_dialog;

	void update_game();
	void pause_game();

public:
	explicit MainWindow(QWidget *parent = nullptr);

	~MainWindow() final;

private:
	Ui::MainWindow *m_ui;

protected:
	void keyPressEvent(QKeyEvent *event) final;
};

#endif // MAINWINDOW_H
