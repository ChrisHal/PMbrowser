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

#include "DlgGraphSettings.h"

DlgGraphSettings::DlgGraphSettings(QWidget *parent)
	: QDialog(parent), validator{ 0, 99}, dvalidator{ }
{
	ui.setupUi(this);
	ui.lineEditNumTraces->setValidator(&validator);
	ui.lineEditXMin->setValidator(&dvalidator);
	ui.lineEditXMax->setValidator(&dvalidator);
	ui.lineEditYMin->setValidator(&dvalidator);
	ui.lineEditYMax->setValidator(&dvalidator);
}

DlgGraphSettings::~DlgGraphSettings()
{
}

void DlgGraphSettings::setValues(bool autoscale, double xmin, double xmax, double ymin, double ymax, int numtraces)
{
	ui.checkBoxEnableAutoscale->setChecked(autoscale);
	ui.lineEditXMin->setText(QString("%1").arg(xmin));
	ui.lineEditXMax->setText(QString("%1").arg(xmax));
	ui.lineEditYMin->setText(QString("%1").arg(ymin));
	ui.lineEditYMax->setText(QString("%1").arg(ymax));
	ui.lineEditNumTraces->setText(QString("%1").arg(numtraces));
}

void DlgGraphSettings::getValues(bool& autoscale, double& xmin, double& xmax, double& ymin, double& ymax, int& numtraces)
{
	autoscale = ui.checkBoxEnableAutoscale->isChecked();
	xmin = ui.lineEditXMin->text().toDouble();
	xmax = ui.lineEditXMax->text().toDouble();
	ymin = ui.lineEditYMin->text().toDouble();
	ymax = ui.lineEditYMax->text().toDouble();
	numtraces = ui.lineEditNumTraces->text().toInt();
}
