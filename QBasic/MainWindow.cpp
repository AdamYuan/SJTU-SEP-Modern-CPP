#include "MainWindow.h"
#include "ui_mainwindow.h"

#include <fstream>

#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), m_ui(new Ui::MainWindow) {
	m_ui->setupUi(this);
	m_program = basic::Program::Create();

	QFont font_display{"Source Code Pro", 13};
	m_ui->treeDisplay->setFont(font_display);
	m_ui->codeDisplay->setFont(font_display);
	m_ui->outputDisplay->setFont(font_display);
	QFont font_input{"Source Code Pro", 16};
	m_ui->cmdEdit->setFont(font_input);

	connect(this, &MainWindow::machineReady, this, &MainWindow::on_machineReady);
}

MainWindow::~MainWindow() { delete m_ui; }

bool MainWindow::is_running() const { return m_machine || m_context; }

void MainWindow::start_machine() {
	m_machine = basic::Machine::Execute(std::move(m_program), std::move(m_context), [this]() { emit machineReady(); });
}

void MainWindow::update_code_view() {
	if (!m_program)
		return;
	m_ui->codeDisplay->setText(QString::fromStdString(m_program->Format()));
}
void MainWindow::update_tree_view() {
	if (!m_program)
		return;
	m_ui->treeDisplay->setText(QString::fromStdString(m_program->FormatAST(m_context.get())));
}
void MainWindow::update_ui() {
	if (is_running()) {
		// running
		m_ui->btnClear->setDisabled(true);

		m_ui->btnRun->setText(u8"终止执行 (TERM)");
	} else {
		m_ui->btnClear->setDisabled(false);

		m_ui->btnRun->setText(u8"执行代码 (RUN)");
	}
}
bool MainWindow::run_command(const basic::String &cmd) {
	auto tokens = basic::Token::Tokenize(cmd);
	if (tokens.empty())
		return false;

	if (tokens[0].IsDigit()) {
		// Insert or erase statement
		if (is_running()) {
			show_status("Cannot modify program when running");
			return false;
		}
		auto line = tokens[0].ToDigit<basic::LineID>();
		if (tokens.size() == 1) {
			m_program->EraseStatement(line);
		} else {
			auto stmt_res = basic::Statement::Parse({tokens.begin() + 1, tokens.end()});
			if (stmt_res.IsOK()) {
				m_program->InsertStatement(line, stmt_res.PopValue());
			} else {
				show_status(stmt_res.PopError().Format());
				return false;
			}
		}
	} else if (tokens[0].GetView() == "?") {
		// Request input and resume program
		if (!is_running()) {
			show_status("Cannot input when not running");
			return false;
		}
		auto input = basic::Token::DeTokenize({tokens.begin() + 1, tokens.end()});
		if (m_machine)
			m_machine->PushInput(input);
		else {
			m_context->PushInput(input);
			start_machine();
		}
	} else if (tokens.size() == 1) {
		auto view = tokens[0].GetView();
		if (view == "CLEAR") {
			if (is_running()) {
				show_status("Cannot modify program when running");
				return false;
			}
			m_program->Clear();
			m_ui->outputDisplay->clear();
		} else if (view == "RUN") {
			if (is_running()) {
				show_status("Program is already running");
				return false;
			}
			m_ui->outputDisplay->clear();
			m_context = nullptr;
			start_machine();
		} else if (view == "TERM") {
			if (!is_running()) {
				show_status("Program is already stopped");
				return false;
			}
			if (m_machine)
				m_machine->Terminate();
			else {
				m_context->Terminate();
				start_machine();
			}
		} else if (view == "QUIT") {
			QApplication::quit();
		} else if (view == "LOAD") {
			auto filename =
			    QFileDialog::getOpenFileName(this, tr("Load QBASIC Script"), "", tr("QBASIC Script (*.qbasic)"));
			if (filename.isEmpty()) {
				show_status("No file to load");
				return false;
			}

			std::ifstream fin{QDir::toNativeSeparators(filename).toStdString()};
			if (fin.is_open()) {
				basic::String line;
				while (std::getline(fin, line)) {
					if (!run_command(line))
						return false;
				}
			} else {
				show_status("Unable to load \'" + filename.toStdString() + "\'");
				return false;
			}
		} else if (view == "HELP") {
			QMessageBox::information(this, tr("QBASIC Help"), tr("A minimal BASIC interpreter made by AdamYuan."));
		} else {
			show_status("Invalid command \'" + basic::Token::DeTokenize(tokens) + "\'");
			return false;
		}
	} else {
		show_status("Invalid command \'" + basic::Token::DeTokenize(tokens) + "\'");
		return false;
	}

	show_status("");

	update_code_view();
	update_tree_view();
	update_ui();
	return true;
}

void MainWindow::print_message(const basic::String &msg) { m_ui->outputDisplay->append(QString::fromStdString(msg)); }

void MainWindow::show_status(const basic::String &status) {
	m_ui->statusbar->showMessage(QString::fromStdString(status));
}

void MainWindow::on_machineReady() {
	if (!m_machine)
		return;

	basic::ExecuteResult result = basic::Machine::GetResult(&m_machine);

	m_program = std::move(result.program);
	m_context = std::move(result.context);

	update_code_view();
	update_tree_view();

	if (m_context) {
		// print outputs
		basic::String outputs = m_context->PopOutputs();
		if (!outputs.empty())
			print_message(outputs);
	}

	auto error = result.result.PopError();
	error.Visit([this](const auto &error) {
		using Error = std::decay_t<decltype(error)>;
		if (m_context) {
			if constexpr (std::is_same_v<Error, basic::MsgRequestInput>) {
				if (m_context->HaveInput())
					start_machine(); // resume if inputs are already there
				else {
					// wait for input
					m_ui->cmdEdit->setText("? ");
					print_message("[" + std::to_string(m_context->GetLine()) + "]" + error.Format());
				}
			} else if constexpr (std::is_same_v<Error, basic::MsgPrint>) {
				// resume
				start_machine();
			} else {
				print_message("[" + std::to_string(m_context->GetLine()) + "]" + error.Format());
				m_context = nullptr;
			}
		} else
			print_message(error.Format());
	});

	update_ui();
}

void MainWindow::on_cmdEdit_returnPressed() {
	basic::String cmd = m_ui->cmdEdit->text().toStdString();
	if (run_command(cmd))
		m_ui->cmdEdit->clear();
}
void MainWindow::on_btnLoad_clicked() { run_command("LOAD"); }
void MainWindow::on_btnClear_clicked() { run_command("CLEAR"); }
void MainWindow::on_btnRun_clicked() { run_command(is_running() ? "TERM" : "RUN"); }
