#include "DlgExportMetadata.h"
#include "ui_DlgExportMetadata.h"

DlgExportMetadata::DlgExportMetadata(QWidget *parent)
	: QDialog(parent), selection{ }, ui(new Ui::DlgExportMetadata())
{
	ui->setupUi(this);
	ui->comboBoxLevel->setCurrentIndex(3);
}

DlgExportMetadata::~DlgExportMetadata()
{
	delete ui;
}
