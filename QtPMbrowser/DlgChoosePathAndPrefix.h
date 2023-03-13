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

#pragma once

#include <QWidget>
#include <QString>
#include "ui_DlgChoosePathAndPrefix.h"

QT_BEGIN_NAMESPACE
namespace Ui { class PMbrowserWindow; }
QT_END_NAMESPACE

enum class ExportType {
    Igor = 0,
    NPY = 1,
    BIN = 2
};

class DlgChoosePathAndPrefix : public QDialog
{
	Q_OBJECT

public:

	DlgChoosePathAndPrefix(QWidget* parent, const QString& Path);
	~DlgChoosePathAndPrefix();
    void accept() override;
	QString path, prefix{};
    bool pxp_export{}, create_datafolders{};
    int level_last_folder{};
    ExportType export_type{ ExportType::Igor };

private slots:
	void choosePath();
    void stateExportPXPchanged(int state);
    void stateCreateFoldersChanged(int state);

private:
	Ui::DlgChoosePathAndPrefix* ui;
};
