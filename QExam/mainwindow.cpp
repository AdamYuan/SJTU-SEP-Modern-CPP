#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <climits>
#include <fstream>
#include <iostream>
#include <set>
#include <string>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), m_ui(new Ui::MainWindow) {
	m_ui->setupUi(this);
	this->resize(800, 600);
	this->setWindowTitle("Find Path!");

	// @TODO: write your code here

	connect(m_ui->actionLoad, &QAction::triggered, this, [this] {
		auto filename = QFileDialog::getOpenFileName(this, tr("Load testcase"));
		if (filename.isEmpty())
			return;

		std::ifstream fin{QDir::toNativeSeparators(filename).toStdString()};
		m_matrix = Matrix::Input(fin);
		showResult();

		m_show_path = m_show_safety = false;

		update();
	});
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
	// @TODO: write your code here
	if (event->key() == Qt::Key_T) {
		transferMatrix();
		update();
	} else if (event->key() == Qt::Key_P) {
		findPath();
		update();
	}
}

void MainWindow::showResult() {
	// @TODO: write your code here
	for (int y = 0; y < m_matrix.GetHeight(); ++y) {
		for (int x = 0; x < m_matrix.GetWidth(); ++x)
			std::cout << m_matrix.GetSafety(x, y) << " ";
		std::cout << std::endl;
	}

	std::cout << std::endl;

	if (m_matrix.HavePath())
		for (const auto &c : m_matrix.GetPath()) {
			std::cout << "(" << c.y << ", " << c.x << ")" << std::endl;
		}
	else
		std::cout << "can not find path." << std::endl;
}

void MainWindow::transferMatrix() {
	// @TODO: write your code here, subproblem one
	// You can add parameters to this method as needed
	// Done in Matrix constructor
	m_show_safety = !m_show_safety;
}

void MainWindow::findPath() {
	// @TODO: write your code here, subproblem two
	// You can add parameters to this method as needed
	// Done in Matrix constructor
	if (!m_matrix.HavePath()) {
		QMessageBox::information(this, tr("error"), tr("can not find path."));
		m_show_path = false;
		return;
	}
	m_show_path = !m_show_path;
}

void MainWindow::paintEvent(QPaintEvent *event) {
	if (m_matrix.IsEmpty())
		return;

	using Coord = Matrix::Coord;

	constexpr int kGridUnit = 20;
	const int kGridWidth = kGridUnit * m_matrix.GetWidth();
	const int kGridHeight = kGridUnit * m_matrix.GetHeight();
	const int kMenuHeight = m_ui->menubar->height();

	QPainter painter(this);

	{ // Set Perspective
		int w = width(), h = std::max(0, height() - kMenuHeight);
		if (kGridWidth * h < kGridHeight * w) {
			painter.translate(w * 0.5 - h * (qreal)kGridWidth / kGridHeight * 0.5, kMenuHeight);
			qreal scale = h / (qreal)kGridHeight;
			painter.scale(scale, scale);
		} else {
			painter.translate(0, kMenuHeight + h * 0.5 - w * (qreal)kGridHeight / kGridWidth * 0.5);
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

	// Mark Path Blocks
	std::set<Coord> on_path;
	for (const auto &c : m_matrix.GetPath())
		on_path.insert(c);

	// Draw Blocks
	painter.setPen(QColor{0, 0, 0});
	m_matrix.ForEachCoord([&](Coord c) {
		QColor color = (m_show_path && on_path.contains(c))
		                   ? QColor{0, 0, 255}
		                   : (m_matrix.GetGrid(c) ? QColor{255, 0, 0} : QColor{0, 255, 0});
		painter.setBrush(color);
		draw_cube(c);
	});

	// Draw Text
	if (m_show_safety) {
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.setFont(QFont{"Source Code Pro", 6});
		m_matrix.ForEachCoord([&](Coord c) {
			int n = m_matrix.GetSafety(c);
			painter.drawText(c.x * kGridUnit, c.y * kGridUnit, kGridUnit, kGridUnit, Qt::AlignCenter,
			                 QString::fromStdString(n == INT_MAX ? "inf" : std::to_string(n)));
		});
	}
}
