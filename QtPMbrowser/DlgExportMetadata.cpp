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
#include <QPushButton>
#include "DlgExportMetadata.h"
#include "ui_DlgExportMetadata.h"

DlgExportMetadata::DlgExportMetadata(PMbrowserWindow* parent)
    : QDialog(parent), selection{}, ui(new Ui::DlgExportMetadata()), pmbrowserwindow{ parent }
{
	QSettings settings;
	ui->setupUi(this);
	auto index = settings.value("DlgExportMetadata/selection", 3).toInt();
	ui->comboBoxLevel->setCurrentIndex(index);
    ui->checkBoxSystemLocale->setChecked(settings.value("DlgExportMetadata/nativeEncoding", 0).toInt());
    QObject::connect(ui->pushButtonSlectParamters, &QPushButton::clicked,
        pmbrowserwindow, &PMbrowserWindow::on_actionSelect_Parameters_triggered);
    QObject::connect(ui->pushButtonCopy, &QPushButton::clicked, this, &DlgExportMetadata::copyToClipboard);
}

DlgExportMetadata::~DlgExportMetadata()
{
	delete ui;
}

void DlgExportMetadata::accept() {
	QSettings settings;
	selection = ui->comboBoxLevel->currentIndex();
	settings.setValue("DlgExportMetadata/selection", selection);
    m_nativeEncoding = ui->checkBoxSystemLocale->isChecked();
    settings.setValue("DlgExportMetadata/nativeEncoding", m_nativeEncoding);
	QDialog::accept();
};

void DlgExportMetadata::copyToClipboard() {
    m_doCopy = true;
    this->accept();
}