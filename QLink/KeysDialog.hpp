#pragma once

#include <QDialog>

#include "game/Game.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class KeysDialog;
}
QT_END_NAMESPACE

class KeysDialog final : public QDialog {
Q_OBJECT
public:
	explicit KeysDialog(QWidget *parent = nullptr);

	~KeysDialog() final;

private:
	Ui::KeysDialog *m_ui;
};
