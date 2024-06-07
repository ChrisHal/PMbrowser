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

#include <QFileDialog>
#include <QSettings>
#include "ui_DlgChoosePathAndPrefix.h"
#include "DlgChoosePathAndPrefix.h"

DlgChoosePathAndPrefix::DlgChoosePathAndPrefix(QWidget* parent, const QString& Path) : QDialog(parent), path(Path),
ui(new Ui::DlgChoosePathAndPrefix)
{
	ui->setupUi(this);

	QSettings settings;

	ui->lineEditPath->setText(path);
	ui->lineEditPrefix->setText(settings.value("DlgChoosePathAndPrefix/prefix", "PM").toString());


	ui->checkBox_pxp_export->setChecked(
		settings.value("DlgChoosePathAndPrefix/pxp_export", 0).toBool()
	);
	ui->checkBox_create_datafolders->setChecked(
		settings.value("DlgChoosePathAndPrefix/create_folders", 0).toBool()
	);
	ui->comboBoxLevel->setCurrentIndex(
		settings.value("DlgChoosePathAndPrefix/last_export_level", 1).toInt()
	);
	export_type = static_cast<ExportType>(settings.value("DlgChoosePathAndPrefix/export_type", 0).toInt());
	switch (export_type) {
	case ExportType::Igor:
		ui->radioButtonIgor->setChecked(true);
		break;
	case ExportType::NPYarray:
		ui->checkBoxNPYarray->setChecked(true);
		ui->radioButtonNPY->setChecked(true);
		break;
	case ExportType::NPY:
		ui->checkBoxNPYarray->setChecked(false);
		ui->radioButtonNPY->setChecked(true);
		break;
	case ExportType::BIN:
		ui->radioButtonBIN->setChecked(true);
		break;
	}
	QObject::connect(ui->pushButtonChoosePath, SIGNAL(clicked()), this, SLOT(choosePath()));
	QObject::connect(ui->checkBox_pxp_export, &QCheckBox::stateChanged,
		this, &DlgChoosePathAndPrefix::stateExportPXPchanged);
	QObject::connect(ui->checkBox_create_datafolders, &QCheckBox::stateChanged,
		this, &DlgChoosePathAndPrefix::stateCreateFoldersChanged);
}

DlgChoosePathAndPrefix::~DlgChoosePathAndPrefix()
{
	delete ui;
}

void DlgChoosePathAndPrefix::choosePath()
{
	QFileDialog dlg;
	dlg.setFileMode(QFileDialog::Directory);
	dlg.selectFile(path);
	if (dlg.exec()) {
        auto files = dlg.selectedFiles();
        ui->lineEditPath->setText(files[0]);
	}
}

void DlgChoosePathAndPrefix::stateExportPXPchanged(int state) {
	if (!state) { ui->checkBox_create_datafolders->setChecked(false); }
}


void DlgChoosePathAndPrefix::stateCreateFoldersChanged(int state) {
	if (state) { ui->checkBox_pxp_export->setChecked(true); }
}


void DlgChoosePathAndPrefix::accept()
{
	path = ui->lineEditPath->text();
	prefix = ui->lineEditPrefix->text();
	pxp_export = ui->checkBox_pxp_export->isChecked();
	create_datafolders = ui->checkBox_create_datafolders->isChecked();
	level_last_folder = ui->comboBoxLevel->currentIndex();

	if (ui->radioButtonIgor->isChecked()) {
		export_type = ExportType::Igor;
	}
	else if (ui->radioButtonNPY->isChecked()) {
		if (ui->checkBoxNPYarray->isChecked()) {
			export_type = ExportType::NPYarray;
		}
		else {
			export_type = ExportType::NPY;
		}
	}
	else if (ui->radioButtonBIN->isChecked()) {
		export_type = ExportType::BIN;
	}

	QSettings settings;

	settings.setValue("DlgChoosePathAndPrefix/prefix", prefix);
	settings.setValue("DlgChoosePathAndPrefix/pxp_export", pxp_export);
	settings.setValue("DlgChoosePathAndPrefix/create_folders", create_datafolders);
	settings.setValue("DlgChoosePathAndPrefix/last_export_level", level_last_folder);
	settings.setValue("DlgChoosePathAndPrefix/export_type", static_cast<int>(export_type));
	QDialog::accept();
}
