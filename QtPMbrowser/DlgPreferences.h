#pragma once

#include <QDialog>
QT_BEGIN_NAMESPACE
namespace Ui { class DlgPreferences; };
QT_END_NAMESPACE

class DlgPreferences : public QDialog
{
	Q_OBJECT

public:
	DlgPreferences(QWidget *parent = nullptr);
	~DlgPreferences();
	void accept() override;

private:
	Ui::DlgPreferences *ui;
};
