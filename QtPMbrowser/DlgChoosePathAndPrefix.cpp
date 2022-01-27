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

#include <QDialog>
#include <QFileDialog>
#include <QSettings>
#include "DlgChoosePathAndPrefix.h"

DlgChoosePathAndPrefix::DlgChoosePathAndPrefix(QWidget* parent, const QString& Path) : QDialog(parent), path(Path), prefix{},
pxp_export{}, create_datafolders{}, ui(new Ui::DlgChoosePathAndPrefix)
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

	QObject::connect(ui->pushButtonChoosePath, SIGNAL(clicked()), this, SLOT(choosePath()));
	QObject::connect(ui->checkBox_pxp_export, &QCheckBox::stateChanged,
		this, &DlgChoosePathAndPrefix::stateExportPXPchanged);
	QObject::connect(ui->checkBox_create_datafolders, &QCheckBox::stateChanged,
		this, &DlgChoosePathAndPrefix::stateCreateFoldersChanged);
}

DlgChoosePathAndPrefix::~DlgChoosePathAndPrefix()
{
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
};


void DlgChoosePathAndPrefix::stateCreateFoldersChanged(int state) {
	if (state) { ui->checkBox_pxp_export->setChecked(true); }
};


void DlgChoosePathAndPrefix::accept()
{
	path = ui->lineEditPath->text();
	prefix = ui->lineEditPrefix->text();
	pxp_export = ui->checkBox_pxp_export->isChecked();
	create_datafolders = ui->checkBox_create_datafolders->isChecked();

	QSettings settings;

	settings.setValue("DlgChoosePathAndPrefix/prefix", prefix);
	settings.setValue("DlgChoosePathAndPrefix/pxp_export", pxp_export);
	settings.setValue("DlgChoosePathAndPrefix/create_folders", create_datafolders);

	QDialog::accept();
}
