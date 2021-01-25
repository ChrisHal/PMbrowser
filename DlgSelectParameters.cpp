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

#include "DlgSelectParameters.h"
//#include "ui_DlgSelectParameters.h"
//#include "PMparameters.h"

DlgSelectParameters::DlgSelectParameters(QWidget *parent)
	: QDialog(parent)
{
	ui = new Ui::DlgSelectParameters();
	ui->setupUi(this);
	//gridGrp.setParent(ui->scrollAreaGrp);
	//ui->scrollAreaGrp->setWidget(gridGrp.widget());
	
	//gridSer.setParent(ui->scrollAreaSer);
	//ui->scrollAreaSer->setWidget(gridSer.widget());
	//gridSwp.setParent(ui->scrollAreaSwp);
	//ui->scrollAreaSwp->setWidget(gridSwp.widget());
	//gridTr.setParent(ui->scrollAreaTr);
	//ui->scrollAreaTr->setWidget(gridTr.widget());
	populateGrid(ui->gridLayoutGrp, parametersGroup);
	populateGrid(ui->gridLayoutSer, parametersSeries);
	populateGrid(ui->gridLayoutSwp, parametersSweep);
	populateGrid(ui->gridLayoutTr, parametersTrace);
}

DlgSelectParameters::~DlgSelectParameters()
{
	while (auto t = ui->gridLayoutGrp->takeAt(0)) {
		delete t->widget();
		delete t;
	}
	while (auto t = ui->gridLayoutSer->takeAt(0)) {
		delete t->widget();
		delete t;
	}
	while (auto t = ui->gridLayoutSwp->takeAt(0)) {
		delete t->widget();
		delete t;
	}
	while (auto t = ui->gridLayoutTr->takeAt(0)) {
		delete t->widget();
		delete t;
	}
	delete ui;
}

void DlgSelectParameters::storeParams()
{
	readFromGrid(ui->gridLayoutGrp, parametersGroup);
	readFromGrid(ui->gridLayoutSer, parametersSeries);
	readFromGrid(ui->gridLayoutSwp, parametersSweep);
	readFromGrid(ui->gridLayoutTr, parametersTrace);
}
