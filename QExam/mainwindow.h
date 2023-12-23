#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QPlainTextEdit>
#include <QWidget>
#include <QtWidgets>

#include "Matrix.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow final : public QMainWindow {
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);

protected:
	void paintEvent(QPaintEvent *event) final;
	void keyPressEvent(QKeyEvent *event) final;

private:
	Ui::MainWindow *m_ui;
	Matrix m_matrix;
	bool m_show_safety{false}, m_show_path{false};

	void transferMatrix();
	void findPath();
	void showResult();
};

#endif // MAINWINDOW_H
