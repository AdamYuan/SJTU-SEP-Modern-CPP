#include "KeysDialog.hpp"
#include "./ui_keysdialog.h"

KeysDialog::KeysDialog(QWidget *parent) : QDialog(parent), m_ui{new Ui::KeysDialog} { m_ui->setupUi(this); }
KeysDialog::~KeysDialog() { delete m_ui; }
