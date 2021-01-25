/*
    Copyright 2020, 2021 Christian R. Halaszovich

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
#include <QVector>
#include <QPainter>
#include <QPainterPath>
#include <QString>
//#include "renderarea.h"

class RenderArea;

class DisplayTrace
{
public:
	DisplayTrace() : x0{ 0.0 }, deltax{ 0.0 }, x_unit{}, y_unit{}, data{} {};
	DisplayTrace(double X0, double DeltaX, const QString& xUnit, const QString& yUnit, const QVector<double>& Data);
	void reset();
	void render(QPainter& painter, RenderArea* display);
	bool isValid() { return !data.isEmpty(); }
	QString getXUnit() { return x_unit; }
	QString getYUnit() { return y_unit; }
    std::tuple<double, double> getDataMinMax(int pLeft, int pRight);
//	QVector<double>& Data() { return data; }
//private:
	double x0, deltax;
	QString x_unit, y_unit;
	QVector<double> data;
};

