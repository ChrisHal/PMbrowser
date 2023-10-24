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

#include <QDialog>
#include "ui_DlgExportMetadata.h"
QT_BEGIN_NAMESPACE
namespace Ui { class DlgExportMetadata; }
QT_END_NAMESPACE

class DlgExportMetadata : public QDialog
{
	Q_OBJECT

public slots:
    void copyToClipboard();

public:
	DlgExportMetadata(QWidget* parent = nullptr);
	~DlgExportMetadata();
	int getSelection() {
		return selection;
	}
    bool doCopy() {
        return m_doCopy;
    }
	void accept() override;

private:
	int selection;
	Ui::DlgExportMetadata *ui;
    bool m_doCopy{ false };
};
