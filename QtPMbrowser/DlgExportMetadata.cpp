#include <QSettings>
#include "DlgExportMetadata.h"
#include "ui_DlgExportMetadata.h"

DlgExportMetadata::DlgExportMetadata(QWidget *parent)
	: QDialog(parent), selection{ }, ui(new Ui::DlgExportMetadata())
{
	QSettings settings;
	ui->setupUi(this);
	auto index = settings.value("DlgExportMetadata/selection", 3).toInt();
	ui->comboBoxLevel->setCurrentIndex(index);
}

DlgExportMetadata::~DlgExportMetadata()
{
	delete ui;
}

void DlgExportMetadata::accept(){
	QSettings settings;
	selection = ui->comboBoxLevel->currentIndex();
	settings.setValue("DlgExportMetadata/selection", selection);
	QDialog::accept();
};