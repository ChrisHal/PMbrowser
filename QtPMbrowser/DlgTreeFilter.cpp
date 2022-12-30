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

#include "DlgTreeFilter.h"

DlgTreeFilter::DlgTreeFilter(QWidget *parent, const QString& Grp, const QString& Ser,
	const QString& Swp, const QString& Trace)
    : QDialog{parent}, grp{Grp}, ser{Ser}, swp{Swp}, trace{Trace}
{
	ui.setupUi(this);
	QObject::connect(ui.pushButtonReset, &QPushButton::clicked, this, &DlgTreeFilter::resetFilter);
	ui.lineEditGrp->setText(grp);
	ui.lineEditSer->setText(ser);
	ui.lineEditSwp->setText(swp);
	ui.lineEditTr->setText(trace);
}

DlgTreeFilter::~DlgTreeFilter()
{
}

void DlgTreeFilter::resetFilter()
{
	ui.lineEditGrp->setText(".*");
	ui.lineEditSer->setText(".*");
	ui.lineEditSwp->setText(".*");
	ui.lineEditTr->setText(".*");
}

void DlgTreeFilter::accept()
{
	grp = ui.lineEditGrp->text();
	ser = ui.lineEditSer->text();
	swp = ui.lineEditSwp->text();
	trace = ui.lineEditTr->text();
	QDialog::accept();
}
