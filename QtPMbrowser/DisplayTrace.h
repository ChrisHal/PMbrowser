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
#include <vector>
#include <QPainter>
#include <QPainterPath>
#include <QString>
//#include "renderarea.h"

class RenderArea;

class DisplayTrace
{
public:
    DisplayTrace() : x0{ 0.0 }, deltax{ 0.0 }, x_unit{}, y_unit{}, data{}, p_xdata{} {};
    DisplayTrace(DisplayTrace&& dtrace) noexcept : x0{ dtrace.x0 }, deltax{ dtrace.deltax },
        x_unit{ std::move(dtrace.x_unit) }, y_unit{ std::move(dtrace.y_unit) }, data{ std::move(dtrace.data) },
        p_xdata{std::move(dtrace.p_xdata)} {};
    DisplayTrace(const std::vector<std::array<double, 2>>& xy_trace, const std::string_view& DACunit);
    DisplayTrace& operator=(const DisplayTrace& dtrace) {
        x0 = dtrace.x0;
        deltax = dtrace.deltax;
        x_unit = dtrace.x_unit;
        y_unit = dtrace.y_unit;
        data = dtrace.data;
        if (dtrace.has_x_trace()) {
            p_xdata = std::make_unique<std::vector<double>>(*dtrace.p_xdata);
        }
        return *this;
    };
    DisplayTrace& operator=(DisplayTrace&& dtrace) noexcept {
        x0 = dtrace.x0;
        deltax = dtrace.deltax;
        x_unit = dtrace.x_unit;
        y_unit = dtrace.y_unit;
        data = std::move(dtrace.data);
        p_xdata = std::move(dtrace.p_xdata);
        return *this;
    };
	//DisplayTrace(double X0, double DeltaX, const QString& xUnit, const QString& yUnit, const QVector<double>& Data);
	void reset();
	void render(QPainter& painter, RenderArea* display);
	bool isValid() const { return !data.empty(); }
    bool has_x_trace() const { return !!p_xdata; }
    
    /// <summary>
    /// interpolate datapoint for given x value
    /// if x-trace is present, uses linear interpolation
    /// else uses nearest neibor
    /// </summary>
    /// <param name="x">position to interpolate</param>
    /// <returns>if x is in range, interpolated value, nan else</returns>
    double interp(double x);
	QString getXUnit() const { return x_unit; }
	QString getYUnit() const { return y_unit; }
    std::tuple<double, double> getDataMinMax(int pLeft, int pRight);
//	QVector<double>& Data() { return data; }
//private:
	double x0, deltax;
	QString x_unit, y_unit;
	std::vector<double> data;
    std::unique_ptr<std::vector<double>> p_xdata;
};

