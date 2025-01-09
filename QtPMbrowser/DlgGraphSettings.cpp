/*
    Copyright 2020 - 2022, 2025 Christian R. Halaszovich

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

#include <QLocale>
#include <QColor>
#include <QColorDialog>
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
	QObject::connect(ui.pushButtonSelectGridColor, &QPushButton::clicked,
		this, &DlgGraphSettings::selectGridColor);
	QObject::connect(ui.pushButtonColorTrace, &QPushButton::clicked,
		this, &DlgGraphSettings::selectTraceColor);
	QObject::connect(ui.pushButtonColorBkTrace, &QPushButton::clicked,
		this, &DlgGraphSettings::selectBkTraceColor);
}

DlgGraphSettings::~DlgGraphSettings()
{
}

void DlgGraphSettings::selectGridColor()
{
	auto color = QColorDialog::getColor(m_color_grid);
	if (color.isValid()) {
		m_color_grid = color;
		ui.widgetColorGrid->setPalette(QPalette(m_color_grid));
	}
}

void DlgGraphSettings::selectTraceColor()
{
	auto color = QColorDialog::getColor(m_color_trace);
	if (color.isValid()) {
		m_color_trace = color;
		ui.widgetColorTrace->setPalette(QPalette(m_color_trace));
	}
}

void DlgGraphSettings::selectBkTraceColor()
{
	auto color = QColorDialog::getColor(m_color_bktrace);
	if (color.isValid()) {
		m_color_bktrace = color;
		ui.widgetColorBkTrace->setPalette(QPalette(m_color_bktrace));
	}
}

void DlgGraphSettings::setValues(bool autoscale, double xmin, double xmax, double ymin, double ymax,
	int numtraces, bool grid_horz, bool grid_vert, bool shift_all_y_scales,
	QColor color_grid, QColor color_trace, QColor color_bktrace)
{
	QLocale loc{};
	ui.checkBoxEnableAutoscale->setChecked(autoscale);
	ui.checkBoxHorzGrid->setChecked(grid_horz);
	ui.checkBoxVertGrid->setChecked(grid_vert);
	ui.checkBoxShiftAllYScales->setChecked(shift_all_y_scales);
	ui.lineEditXMin->setText(loc.toString(xmin));
	ui.lineEditXMax->setText(loc.toString(xmax));
	ui.lineEditYMin->setText(loc.toString(ymin));
	ui.lineEditYMax->setText(loc.toString(ymax));
	ui.lineEditNumTraces->setText(loc.toString(numtraces));
	m_color_grid = color_grid;
	ui.widgetColorGrid->setPalette(QPalette(m_color_grid));
	m_color_trace = color_trace;
	ui.widgetColorTrace->setPalette(QPalette(m_color_trace));
	m_color_bktrace = color_bktrace;
	ui.widgetColorBkTrace->setPalette(QPalette(m_color_bktrace));
}

void DlgGraphSettings::getValues(bool& autoscale, double& xmin, double& xmax, double& ymin, double& ymax,
	int& numtraces, bool& grid_horz, bool& grid_vert, bool& shift_all_y_scales,
	QColor& color_grid, QColor& color_trace, QColor& color_bktrace)
{
	QLocale loc{};
	autoscale = ui.checkBoxEnableAutoscale->isChecked();
	grid_horz = ui.checkBoxHorzGrid->isChecked();
	grid_vert = ui.checkBoxVertGrid->isChecked();
	shift_all_y_scales = ui.checkBoxShiftAllYScales->isChecked();
	xmin = loc.toDouble(ui.lineEditXMin->text());
	xmax = loc.toDouble(ui.lineEditXMax->text());
	ymin = loc.toDouble(ui.lineEditYMin->text());
	ymax = loc.toDouble(ui.lineEditYMax->text());
	numtraces = loc.toInt(ui.lineEditNumTraces->text());
	color_grid = m_color_grid;
	color_trace = m_color_trace;
	color_bktrace = m_color_bktrace;
}
