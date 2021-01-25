/*
    Copyright 2020 Christian R. Halaszovich

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
#include "ui_DlgTreeFilter.h"

class DlgTreeFilter : public QDialog
{
	Q_OBJECT

public:
	DlgTreeFilter(QWidget *parent, const QString& Grp, const QString& Ser,
		const QString& Swp, const QString& Trace);
	~DlgTreeFilter();
	QString grp, ser, swp, trace;
private slots:
	void accept() override;
private:
	Ui::DlgTreeFilter ui;
};
