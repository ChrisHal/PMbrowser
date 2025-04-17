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

#include <cmath>
#include <algorithm>
#ifndef NDEBUG
#include <QDebug>
#endif
#include "DisplayTrace.h"
#include "renderarea.h"
#include <vector>
#include <qstring_helper.h>

DisplayTrace::DisplayTrace(const QString& xunit, const QString& yunit, double x0, double deltax, std::vector<double>&& data):
	x_unit{xunit},y_unit{yunit},m_x0{x0},m_deltax{deltax},m_data{std::move(data)}
{
	set_ymin_ymax();
}

DisplayTrace::DisplayTrace(const std::vector<std::array<double, 2>>& xy_trace, const std::string_view& DACunit) : m_x0{ 0.0 },
m_deltax{ 0.0 }, x_unit{"s"}, y_unit{ qs_from_sv(DACunit) }, m_data{},
p_xdata{ std::make_unique<std::vector<double>>(xy_trace.size())}
{
	m_data.resize(xy_trace.size());
	for (std::size_t i = 0; i < xy_trace.size(); ++i) {
		const auto& p = xy_trace.at(i);
		p_xdata->at(i) = p[0];
		m_data.at(i) = p[1];
	}
	set_ymin_ymax();
}



void DisplayTrace::reset()
{
	m_data.clear();
	p_xdata.reset();
	x_unit.clear();
	y_unit.clear();
}

void DisplayTrace::render(QPainter& painter, RenderArea* display)
{
	QPainterPath path;
	const auto& xdata = display->xTrace.m_data;
	bool special_color{ false };
	if (display->isXYmode()) {
		if (xdata.size() != m_data.size()) {
			// skip incompatible traces for x-y-mode
			return;
		}
		path.moveTo(display->scaleToQPF(xdata[0], m_data[0]));
		for (std::size_t i = 1; i < m_data.size(); ++i) {
			path.lineTo(display->scaleToQPF(xdata[i], m_data[i]));
		}
	}
	else {
		if (has_x_trace()) {
			assert(m_data.size() == p_xdata->size());
			const std::size_t N = m_data.size();
			path.moveTo(display->scaleToQPF(p_xdata->at(0), m_data.at(0)));
			for (std::size_t i = 1; i < N; ++i) {
				path.lineTo(display->scaleToQPF(p_xdata->at(i), m_data.at(i)));
			}
			special_color = true;
		}
		else {
			//in YT-mode we speed things up by drawing only the
			//datapoints actually visible
			auto N = static_cast<int>(m_data.size());
			int pFirst = std::max(0, int(std::floor((display->x_min - m_x0) / m_deltax)));
			int pEnd = std::min(int(std::ceil((display->x_max - m_x0) / m_deltax)), N);
			if (pFirst < pEnd) { // pFirst might even be larger than data.size(), we catch this case also here
				int step = (pEnd - pFirst) / display->width();
				if (step > 3) { // speed up drawing if we have a lot of datapoints
					pEnd -= step;
					auto [data_min, data_max] = getDataMinMax(pFirst, pFirst + step);
					path.moveTo(display->scaleToQPF(m_x0 + pFirst * m_deltax, data_min));
					path.lineTo(display->scaleToQPF(m_x0 + pFirst * m_deltax, data_max));
					for (int i = step + pFirst; i < pEnd; i += step) {
						auto [datamin, datamax] = getDataMinMax(i, i + step);
						path.lineTo(display->scaleToQPF(m_x0 + i * m_deltax, datamin));
						path.lineTo(display->scaleToQPF(m_x0 + i * m_deltax, datamax));
					}
				}
				else {
					path.moveTo(display->scaleToQPF(m_x0 + pFirst * m_deltax, m_data[pFirst]));
					for (int i = 1 + pFirst; i < pEnd; ++i) {
						path.lineTo(display->scaleToQPF(m_x0 + i * m_deltax, m_data[i]));
					}
				}
			}
		}
	}
	if (special_color) {
		auto old_color = painter.pen().color();
		painter.setPen(0xff0000);
		painter.drawPath(path);
		painter.setPen(old_color);
	}
	else {
		painter.drawPath(path);
	}
}

const std::vector<double>& DisplayTrace::x_data() const
{
	if (p_xdata) {
		return *p_xdata;
	}
	else {
		throw std::runtime_error("trying to acces non-existing x-data");
	}
}

std::tuple<double, double> DisplayTrace::getDataMinMax(int pLeft, int pRight) const
{
	double min_val, max_val;
	max_val = min_val = m_data[pLeft];

	// pRight should be adjusted by caller such that
	// we do not read past end of data vector
	// pRight = std::min(pRight, data.size());
	for (int i = pLeft + 1; i < pRight; ++i) {
		double v = m_data[i];
		min_val = std::min(min_val, v);
		max_val = std::max(max_val, v);
	}
	return {min_val, max_val};
}

void DisplayTrace::set_ymin_ymax()
{
	y_min = y_max = m_data.at(0);
	for (const auto& y : m_data) {
		y_min = std::min(y_min, y);
		y_max = std::max(y_max, y);
	}
}

double DisplayTrace::interp(double x)
{
	double datay = std::numeric_limits<double>::quiet_NaN();
	if (has_x_trace()) {
		for (std::size_t i = 0; i < p_xdata->size() - 1; ++i) {
			if (p_xdata->at(i) <= x && p_xdata->at(i + 1) >= x) {
				auto x_0 = p_xdata->at(i), x_1 = p_xdata->at(i + 1),
					y0 = m_data.at(i), y1 = m_data.at(i + 1);
				datay = y0 + (y1 - y0) * (x - x_0) / (x_1 - x_0);
				break;
			}
		}
	}
	else {
		long dataindex = std::lrint((x - m_x0) / m_deltax);
		if (dataindex >= 0 &&
			static_cast<std::size_t>(dataindex) < m_data.size()) {
			datay = m_data.at(dataindex);
		}
	}
	return datay;
}

void DisplayTrace::convertToInterpolated(std::size_t numpoints, double new_x0, double new_delta)
{
	assert(has_x_trace());
	if (has_x_trace()) {
		std::vector<double> tmp(numpoints);
		for (std::size_t i = 0; i < numpoints; ++i) {
			tmp.at(i) = interp(new_x0 + new_delta * i);
		}
		m_x0 = new_x0;
		m_deltax = new_delta;
		m_data = std::move(tmp);
		p_xdata = nullptr;
	}
}
