/*
	Copyright 2020 - 2024 Christian R. Halaszovich

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

	setScrollArea(ui->scrollAreaRoot, v_root, hkLib::parametersRoot);
	setScrollArea(ui->scrollAreaGrp, v_grp, hkLib::parametersGroup);
	setScrollArea(ui->scrollAreaSer, v_ser, hkLib::parametersSeries);
	setScrollArea(ui->scrollAreaSwp, v_swp, hkLib::parametersSweep);
	setScrollArea(ui->scrollAreaTr, v_tr, hkLib::parametersTrace);
}

DlgSelectParameters::~DlgSelectParameters()
{
	delete ui;
}

void DlgSelectParameters::accept()
{
	storeParams();
	QDialog::accept();
}

void DlgSelectParameters::storeParams()
{
	readSelections(v_root, hkLib::parametersRoot);
	readSelections(v_grp, hkLib::parametersGroup);
	readSelections(v_ser, hkLib::parametersSeries);
	readSelections(v_swp, hkLib::parametersSweep);
	readSelections(v_tr, hkLib::parametersTrace);
}
