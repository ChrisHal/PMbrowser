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
//#include "ui_DlgSelectParameters.h"
//#include "PMparameters.h"

DlgSelectParameters::DlgSelectParameters(QWidget *parent)
	: QDialog(parent)
{
	ui = new Ui::DlgSelectParameters();
	ui->setupUi(this);
	//gridGrp.setParent(ui->scrollAreaGrp);
	areaGrp.setLayout(gridLayoutGrp);
	areaGrp.setBackgroundRole(QPalette::Base);
	ui->scrollAreaGrp->setWidget(&areaGrp);
	
	//gridSer.setParent(ui->scrollAreaSer);
	areaSer.setLayout(gridLayoutSer);
	areaSer.setBackgroundRole(QPalette::Base);
	ui->scrollAreaSer->setWidget(&areaSer);
	//gridSwp.setParent(ui->scrollAreaSwp);
	areaSwp.setLayout(gridLayoutSwp);
	areaSwp.setBackgroundRole(QPalette::Base);
	ui->scrollAreaSwp->setWidget(&areaSwp);
	//gridTr.setParent(ui->scrollAreaTr);
	areaTr.setLayout(gridLayoutTr);
	areaTr.setBackgroundRole(QPalette::Base);
	ui->scrollAreaTr->setWidget(&areaTr);
	populateGrid(gridLayoutGrp, parametersGroup);
	populateGrid(gridLayoutSer, parametersSeries);
	populateGrid(gridLayoutSwp, parametersSweep);
	populateGrid(gridLayoutTr, parametersTrace);
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
	readFromGrid(gridLayoutGrp, parametersGroup);
	readFromGrid(gridLayoutSer, parametersSeries);
	readFromGrid(gridLayoutSwp, parametersSweep);
	readFromGrid(gridLayoutTr, parametersTrace);
}
