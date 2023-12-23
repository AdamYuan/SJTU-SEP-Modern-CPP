#include "MainWindow.hpp"
#include "./ui_mainwindow.h"

#include <fstream>

#include "game/Serializer.hpp"

#include <QFileDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QTimer>

#include "Config.hpp"
#include "KeysDialog.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_ui(new Ui::MainWindow), m_game_grid_widget{new GameWidget{m_game, this}},
      m_new_game_dialog{new NewGameDialog{this}}, m_timer{new QTimer{this}} {

	m_ui->setupUi(this);
	connect(qApp, &QGuiApplication::applicationStateChanged, this, [this](Qt::ApplicationState state) {
		if (state == Qt::ApplicationInactive || state == Qt::ApplicationHidden)
			pause_game();
	});

	// Setup Timer
	m_timer->setInterval(1000);
	connect(m_timer, &QTimer::timeout, this, [this] {
		m_game.NextSecond();
		update_game();
	});

	// Setup New Game Dialog
	connect(m_new_game_dialog, &NewGameDialog::newGame, this,
	        [this](uint32_t width, uint32_t height, bool multi_player) {
		        if (multi_player)
			        m_game.Start(width, height, kInitialTime, kBlockTypes, 2,
			                     {EffectType::kShuffle, EffectType::kDizzy, EffectType::kHint, EffectType::kFreeze,
			                      EffectType::kPlus1S});
		        else
			        m_game.Start(width, height, kInitialTime, kBlockTypes, 1,
			                     {EffectType::kShuffle, EffectType::kHint, EffectType::kPlus1S});
		        update_game();
		        m_timer->start();
	        });
	m_new_game_dialog->show();

	// Setup Menu Actions
	connect(m_ui->actionNew, &QAction::triggered, this, [this] {
		pause_game();
		m_new_game_dialog->show();
	});
	connect(m_ui->actionLoad, &QAction::triggered, this, [this] {
		pause_game();
		auto filename = QFileDialog::getOpenFileName(this, tr("Open QLink Game"), "", tr("QLink File (*.qlink)"));
		if (filename.isEmpty())
			return;

		std::ifstream fin{QDir::toNativeSeparators(filename).toStdString(), std::ios::binary};
		fin.seekg(0);
		if (fin.is_open()) {
			m_game = Serializer<Game>::Read(fin);
			update_game();
			if (!m_game.IsValid())
				QMessageBox::critical(this, tr("QLink"), tr("Invalid QLink File ") + filename);
			else
				m_timer->start();
		} else
			QMessageBox::critical(this, tr("QLink"), tr("Unable to Load ") + filename);
	});
	connect(m_ui->actionSave, &QAction::triggered, this, [this] {
		if (!m_game.IsValid()) {
			QMessageBox::critical(this, tr("QLink"), tr("Invalid Game to Save"));
			return;
		}
		pause_game();
		auto filename = QFileDialog::getSaveFileName(this, tr("Save QLink Game"), "", tr("QLink File (*.qlink)"));
		if (filename.isEmpty())
			return;

		std::ofstream fout{QDir::toNativeSeparators(filename).toStdString(), std::ios::binary};
		if (fout.is_open()) {
			Serializer<Game>::Write(fout, m_game);
			QMessageBox::information(this, tr("QLink"), tr("Current Game Saved to ") + filename);
		} else
			QMessageBox::critical(this, tr("QLink"), tr("Unable to Save ") + filename);
	});
	connect(m_ui->actionKeys, &QAction::triggered, this, [this] {
		pause_game();
		auto keys_dialog = new KeysDialog{this};
		keys_dialog->show();
	});

	// Setup Game Widget
	setCentralWidget(m_game_grid_widget);
}

MainWindow::~MainWindow() {
	delete m_ui;
	delete m_game_grid_widget;
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
	if (event->key() == Qt::Key_P) {
		if (m_game.IsPaused())
			m_timer->start();
		else
			m_timer->stop();
		m_game.TogglePause();
	}
	if (!m_game.GetPlayers().empty()) {
		switch (event->key()) {
		case Qt::Key_W:
			m_game.GetPlayers()[0].Move(0, -1);
			break;
		case Qt::Key_A:
			m_game.GetPlayers()[0].Move(-1, 0);
			break;
		case Qt::Key_S:
			m_game.GetPlayers()[0].Move(0, 1);
			break;
		case Qt::Key_D:
			m_game.GetPlayers()[0].Move(1, 0);
			break;
		}
	}
	if (m_game.GetPlayers().size() >= 2) {
		switch (event->key()) {
		case Qt::Key_Up:
			m_game.GetPlayers()[1].Move(0, -1);
			break;
		case Qt::Key_Left:
			m_game.GetPlayers()[1].Move(-1, 0);
			break;
		case Qt::Key_Down:
			m_game.GetPlayers()[1].Move(0, 1);
			break;
		case Qt::Key_Right:
			m_game.GetPlayers()[1].Move(1, 0);
			break;
		}
	}
	update_game();
}

void MainWindow::update_game() {
	m_game.Update();
	m_game_grid_widget->update();
}
void MainWindow::pause_game() {
	if (!m_game.IsPaused()) {
		m_timer->stop();
		m_game.TogglePause();
		update_game();
	}
}
