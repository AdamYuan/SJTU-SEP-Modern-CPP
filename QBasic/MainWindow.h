#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "basic/Machine.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow final : public QMainWindow {
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);

	~MainWindow() final;

private slots:
	void on_cmdEdit_returnPressed();
	void on_btnLoad_clicked();
	void on_btnClear_clicked();
	void on_btnRun_clicked();

	void on_machineReady();

signals:
	void machineReady();

private:
	Ui::MainWindow *m_ui;

	std::unique_ptr<basic::Context> m_context;
	std::unique_ptr<basic::Program> m_program;
	std::unique_ptr<basic::Machine> m_machine;

	bool run_command(const basic::String &cmd);

	void start_machine();
	bool is_running() const;

	void print_message(const basic::String &msg);
	void show_status(const basic::String &status);
	void update_ui();
	void update_code_view();
	void update_tree_view();
};

#endif // MAINWINDOW_H
