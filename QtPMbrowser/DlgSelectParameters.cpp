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

#include "DlgSelectParameters.h"

DlgSelectParameters::DlgSelectParameters(QWidget *parent)
	: QDialog(parent)
{
	ui = new Ui::DlgSelectParameters();
	ui->setupUi(this);

	auto w = new QWidget;
	w->setLayout(gridLayoutGrp);
	w->setBackgroundRole(QPalette::Base);
	ui->scrollAreaGrp->setWidget(w); // this transfers ownership, doesn't it?
	
	w = new QWidget;
	w->setLayout(gridLayoutSer);
	w->setBackgroundRole(QPalette::Base);
	ui->scrollAreaSer->setWidget(w);
	
	w = new QWidget;
	w->setLayout(gridLayoutSwp);
	w->setBackgroundRole(QPalette::Base); 
	ui->scrollAreaSwp->setWidget(w);
	
	w = new QWidget;
	w->setLayout(gridLayoutTr);
	w->setBackgroundRole(QPalette::Base); 
	ui->scrollAreaTr->setWidget(w);
	
	populateGrid(gridLayoutGrp, hkLib::parametersGroup);
	populateGrid(gridLayoutSer, hkLib::parametersSeries);
	populateGrid(gridLayoutSwp, hkLib::parametersSweep);
	populateGrid(gridLayoutTr, hkLib::parametersTrace);
}

DlgSelectParameters::~DlgSelectParameters()
{
	while (auto t = gridLayoutGrp->takeAt(0)) {
		delete t->widget();
		delete t;
	}
	while (auto t = gridLayoutSer->takeAt(0)) {
		delete t->widget();
		delete t;
	}
	while (auto t = gridLayoutSwp->takeAt(0)) {
		delete t->widget();
		delete t;
	}
	while (auto t = gridLayoutTr->takeAt(0)) {
		delete t->widget();
		delete t;
	}
	delete ui;
}

void DlgSelectParameters::storeParams()
{
	readFromGrid(gridLayoutGrp, hkLib::parametersGroup);
	readFromGrid(gridLayoutSer, hkLib::parametersSeries);
	readFromGrid(gridLayoutSwp, hkLib::parametersSweep);
	readFromGrid(gridLayoutTr, hkLib::parametersTrace);
}
