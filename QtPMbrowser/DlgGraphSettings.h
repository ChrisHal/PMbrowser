/*
    Copyright 2020 -2022, 2025 Christian R. Halaszovich

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
#include <QValidator>
#include "ui_DlgGraphSettings.h"

class DlgGraphSettings : public QDialog
{
	Q_OBJECT

private slots:
    void selectGridColor();
    void selectTraceColor();
    void selectBkTraceColor();

public:
	DlgGraphSettings(QWidget *parent = Q_NULLPTR);
	~DlgGraphSettings();
    void setValues(bool autoscale, bool global_autoscale, double xmin, double xmax,
		double ymin, double ymax, int numtraces, bool grid_horz, bool gird_vert,
        bool shift_all_y_scales,
        QColor color_grid, QColor color_trace, QColor color_bktrace);
    void getValues(bool& autoscale, bool& global_autoscale, double& xmin, double& xmax,
		double& ymin, double& ymax, int& numtraces,
        bool& grid_horz, bool& gird_vert, bool& shift_all_y_scales,
        QColor& color_grid, QColor& color_trace, QColor& color_bktrace);

private:
	Ui::DlgGraphSettings ui;
	QIntValidator validator;
	QDoubleValidator dvalidator;
    QColor m_color_grid{}, m_color_trace{}, m_color_bktrace{};
};
