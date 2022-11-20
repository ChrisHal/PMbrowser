#include <QSettings>
#include "helpers.h"
#include "DlgPreferences.h"
#include "ui_DlgPreferences.h"

DlgPreferences::DlgPreferences(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::DlgPreferences())
{
	QSettings settings;
	ui->setupUi(this);
	settings.beginGroup("Preferences");
	
	int selection = settings.value("selectionButton", 0).toInt();
	if (selection == 0) ui->radioButtonDefaultExt->setChecked(true);
	else if (selection == 1) ui->radioButtonExtFromFile->setChecked(true);
	else ui->radioButtonCustomExt->setChecked(true);
	ui->lineEditVmon->setText(settings.value("Vmon", "Vmon").toString());
	ui->lineEditImon->setText(settings.value("Imon", "Imon").toString());

	settings.endGroup();
}

DlgPreferences::~DlgPreferences()
{
	delete ui;
}

void DlgPreferences::accept()
{
	QSettings settings;
	settings.beginGroup("Preferences");
	int selection{};
	if (ui->radioButtonDefaultExt->isChecked()) selection = 0;
	else if (ui->radioButtonExtFromFile->isChecked()) selection = 1;
	else selection = 2;
	settings.setValue("selectionButton", selection);
	settings.setValue("Vmon", ui->lineEditVmon->text());
	settings.setValue("Imon", ui->lineEditImon->text());
	settings.endGroup();
	switch (selection) {
	case 0:
		global_hkSettings.ext_Imon = "Imon";
		global_hkSettings.ext_Vmon = "Vmon";
		break;
	case 1: // use names from file
		global_hkSettings.ext_Imon.clear();
		global_hkSettings.ext_Vmon.clear();
		break;
	default:
		global_hkSettings.ext_Imon = ui->lineEditImon->text().toStdString();
		global_hkSettings.ext_Vmon = ui->lineEditVmon->text().toStdString();
	}

	QDialog::accept();
}
