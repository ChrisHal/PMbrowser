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

#include <cmath>
#include <algorithm>
#ifndef NDEBUG
#include <QDebug>
#endif
#include "DisplayTrace.h"
#include "renderarea.h"
#include <vector>
#include <qstring_helper.h>

//DisplayTrace::DisplayTrace(double X0, double DeltaX, const QString& xUnit,
//	const QString& yUnit, const QVector<double>& Data):
//	x0{ X0 }, deltax{ DeltaX }, x_unit{ xUnit }, y_unit{ yUnit }, data{ Data }
//{
//}

DisplayTrace::DisplayTrace(const std::vector<std::array<double, 2>>& xy_trace, const std::string_view& DACunit) : x0{ 0.0 },
deltax{ 0.0 }, x_unit{"s"}, y_unit{ qs_from_sv(DACunit) }, data{},
p_xdata{ std::make_unique<std::vector<double>>(xy_trace.size())}
{
	data.resize(xy_trace.size());
	for (std::size_t i = 0; i < xy_trace.size(); ++i) {
		const auto& p = xy_trace.at(i);
		p_xdata->at(i) = p[0];
		data.at(i) = p[1];
	}
}



void DisplayTrace::reset()
{
	data.clear();
	p_xdata.reset();
	x_unit.clear();
	y_unit.clear();
}

void DisplayTrace::render(QPainter& painter, RenderArea* display)
{
	QPainterPath path;
	const auto& xdata = display->xTrace.data;
	bool special_color{ false };
	if (display->isXYmode()) {
		if (xdata.size() != data.size()) {
			// skip incompatible traces for x-y-mode
			return;
		}
		path.moveTo(display->scaleToQPF(xdata[0], data[0]));
		for (std::size_t i = 1; i < data.size(); ++i) {
			path.lineTo(display->scaleToQPF(xdata[i], data[i]));
		}
	}
	else {
		if (has_x_trace()) {
			assert(data.size() == p_xdata->size());
			const std::size_t N = data.size();
			path.moveTo(display->scaleToQPF(p_xdata->at(0), data.at(0)));
			for (std::size_t i = 1; i < N; ++i) {
				path.lineTo(display->scaleToQPF(p_xdata->at(i), data.at(i)));
			}
			special_color = true;
		}
		else {
			//in YT-mode we speed things up by drawing only the
			//datapoints actually visible
			auto N = static_cast<int>(data.size());
			int pFirst = std::max(0, int(std::floor((display->x_min - x0) / deltax)));
			int pEnd = std::min(int(std::ceil((display->x_max - x0) / deltax)), N);
			//#ifndef NDEBUG
			//		qDebug() << "first: " << pFirst << ", end: " << pEnd;
			//#endif
			if (pFirst < pEnd) { // pFirst might even be larger than data.size(), we catch this case also here
				int step = (pEnd - pFirst) / display->width();
				if (step > 3) { // speed up drawing if we have a lot of datapoints
	//#ifndef NDEBUG
	//				qDebug() << "step: " << step;
	//#endif // !NDEBUG
					pEnd -= step;
					auto [data_min, data_max] = getDataMinMax(pFirst, pFirst + step);
					path.moveTo(display->scaleToQPF(x0 + pFirst * deltax, data_min));
					path.lineTo(display->scaleToQPF(x0 + pFirst * deltax, data_max));
					for (int i = step + pFirst; i < pEnd; i += step) {
						auto [data_min, data_max] = getDataMinMax(i, i + step);
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
	}
	if (special_color) {
		auto old_color = painter.pen().color();
		painter.setPen(QColorConstants::Red);
		painter.drawPath(path);
		painter.setPen(old_color);
	}
	else {
		painter.drawPath(path);
	}
}

std::tuple<double, double> DisplayTrace::getDataMinMax(int pLeft, int pRight)
{
	double min_val, max_val;
	max_val = min_val = data[pLeft];

	// pRight should be adjusted by caller such that
	// we do not read past end of data vector
	// pRight = std::min(pRight, data.size());
	for (int i = pLeft + 1; i < pRight; ++i) {
		double v = data[i];
		min_val = std::min(min_val, v);
		max_val = std::max(max_val, v);
	}
	return {min_val, max_val};
}

double DisplayTrace::interp(double x)
{
	double datay = std::numeric_limits<double>::quiet_NaN();
	if (has_x_trace()) {
		for (std::size_t i = 0; i < p_xdata->size() - 1; ++i) {
			if (p_xdata->at(i) <= x && p_xdata->at(i + 1) >= x) {
				auto x0 = p_xdata->at(i), x1 = p_xdata->at(i + 1),
					y0 = data.at(i), y1 = data.at(i + 1);
				datay = y0 + (y1 - y0) * (x - x0) / (x1 - x0);
				break;
			}
		}
	}
	else {
		long dataindex = std::lrint((x - x0) / deltax);
		if (dataindex >= 0 &&
			static_cast<std::size_t>(dataindex) < data.size()) {
			datay = data.at(dataindex);
		}
	}
	return datay;
}