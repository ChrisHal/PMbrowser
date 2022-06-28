#pragma once

#include <QDialog>
#include "ui_DlgExportMetadata.h"
QT_BEGIN_NAMESPACE
namespace Ui { class DlgExportMetadata; }
QT_END_NAMESPACE

class DlgExportMetadata : public QDialog
{
	Q_OBJECT

public:
	DlgExportMetadata(QWidget* parent = nullptr);
	~DlgExportMetadata();
	int getSelection() {
		return selection;
	}
	void accept() override {
		selection = ui->comboBoxLevel->currentIndex();
		QDialog::accept();
	};

private:
	int selection;
	Ui::DlgExportMetadata *ui;
};
