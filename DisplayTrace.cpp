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

#include <cmath>
#include <algorithm>
#ifndef NDEBUG
#include <QDebug>
#endif
#include "DisplayTrace.h"
#include "renderarea.h"

DisplayTrace::DisplayTrace(double X0, double DeltaX, const QString& xUnit,
	const QString& yUnit, const QVector<double>& Data):
	x0{ X0 }, deltax{ DeltaX }, x_unit{ xUnit }, y_unit{ yUnit }, data{ Data }
{
}

void DisplayTrace::reset()
{
	data.clear();
	x_unit.clear();
	y_unit.clear();
}

void DisplayTrace::render(QPainter& painter, RenderArea* display)
{
	QPainterPath path;
	const auto& xdata = display->xTrace.data;
	if (display->isXYmode()) {
		if (xdata.size() != data.size()) {
			// skip incompatible traces for x-y-mode
			return;
		}
		path.moveTo(display->scaleToQPF(xdata[0], data[0]));
		for (int i = 1; i < data.size(); ++i) {
			path.lineTo(display->scaleToQPF(xdata[i], data[i]));
		}
	}
	else {
		//in YT-mode we speed things up by drawing only the
		//datapoints actually visible
		int N = data.size();
		int pFirst = std::max(0, int(std::floor((display->x_min - x0) / deltax)));
		int pEnd = std::min(int(std::ceil((display->x_max - x0) / deltax)), N);
#ifndef NDEBUG
		qDebug() << "first: " << pFirst << ", end: " << pEnd;
#endif
		if (pFirst < pEnd) { // pFirst might even be larger than data.size(), we catch this case also here
			int step = (pEnd - pFirst) / display->width();
			if (step > 3) { // speed up drawing if we have a lot of datapoints
#ifndef NDEBUG
				qDebug() << "step: " << step;

#endif // !NDEBUG

				const auto [data_min, data_max] = getDataMinMax(pFirst, pFirst + step);
				path.moveTo(display->scaleToQPF(x0 + pFirst * deltax, data_min));
				path.lineTo(display->scaleToQPF(x0 + pFirst * deltax, data_max));
				for (int i = step + pFirst; i < pEnd; i += step) {
					const auto [data_min, data_max] = getDataMinMax(i, i + step);
					path.lineTo(display->scaleToQPF(x0 + i * deltax, data_min));
					path.lineTo(display->scaleToQPF(x0 + i * deltax, data_max));
				}
			}
			else {
				path.moveTo(display->scaleToQPF(x0 + pFirst * deltax, data[pFirst]));
				for (int i = 1 + pFirst; i < pEnd; ++i) {
					path.lineTo(display->scaleToQPF(x0 + i * deltax, data[i]));
				}
			}
		}
	}
	painter.drawPath(path);
}

std::tuple<double, double> DisplayTrace::getDataMinMax(int pLeft, int pRight)
{
	double min_val, max_val;
	max_val = min_val = data[pLeft];
	pRight = std::min(pRight, data.size());
	for (int i = pLeft + 1; i < pRight; ++i) {
		double v = data[i];
		min_val = std::min(min_val, v);
		max_val = std::max(max_val, v);
	}
	return {min_val, max_val};
}
