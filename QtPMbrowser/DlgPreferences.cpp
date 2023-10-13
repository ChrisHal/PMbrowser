/*
    Copyright 2020 - 2022 Christian R. Halaszovich

     This file is part of PMbrowser.

    PMbrowser is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PMbrowser is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with PMbrowser.  If not, see <https://www.gnu.org/licenses/>.
*/

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
		hkLib::global_hkSettings.ext_Imon = "Imon";
		hkLib::global_hkSettings.ext_Vmon = "Vmon";
		break;
	case 1: // use names from file
		hkLib::global_hkSettings.ext_Imon.clear();
		hkLib::global_hkSettings.ext_Vmon.clear();
		break;
	default:
		hkLib::global_hkSettings.ext_Imon = ui->lineEditImon->text().toStdString();
		hkLib::global_hkSettings.ext_Vmon = ui->lineEditVmon->text().toStdString();
	}

	QDialog::accept();
}
