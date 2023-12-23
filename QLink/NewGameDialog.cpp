#include "NewGameDialog.hpp"
#include "./ui_newgamedialog.h"

NewGameDialog::NewGameDialog(QWidget *parent) : QDialog(parent), m_ui{new Ui::NewGameDialog} {
	m_ui->setupUi(this);

	connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, [this]() {
		int w = m_ui->spinBox_width->value();
		int h = m_ui->spinBox_height->value();
		w = std::max(w, 2), h = std::max(h, 2);
		if (w & 1)
			++w;
		if (h & 1)
			++h;

		emit newGame((uint32_t)w, (uint32_t)h, m_ui->radioButton_multiple->isChecked());
	});
}

NewGameDialog::~NewGameDialog() { delete m_ui; }
