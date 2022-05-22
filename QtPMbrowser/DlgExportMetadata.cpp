#include "DlgExportMetadata.h"
#include "ui_DlgExportMetadata.h"

DlgExportMetadata::DlgExportMetadata(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::DlgExportMetadata()), selection{ }
{
	ui->setupUi(this);
	ui->comboBoxLevel->setCurrentIndex(3);
}

DlgExportMetadata::~DlgExportMetadata()
{
	delete ui;
}
