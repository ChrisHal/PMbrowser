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

#pragma once
#include <vector>
#include <memory>
#include <cstddef>
#include <QPainter>
#include <QPainterPath>
#include <QString>

class RenderArea;

class DisplayTrace
{
public:
    DisplayTrace() = default;
    DisplayTrace(DisplayTrace&& dtrace) = default;
    DisplayTrace(const QString& xunit, const QString& yunit, double x0,
        double deltax, std::vector<double>&& m_data);
    DisplayTrace(const std::vector<std::array<double, 2>>& xy_trace, const std::string_view& DACunit);
    DisplayTrace& operator=(const DisplayTrace& dtrace) {
        m_x0 = dtrace.m_x0;
        m_deltax = dtrace.m_deltax;
        x_unit = dtrace.x_unit;
        y_unit = dtrace.y_unit;
        m_data = dtrace.m_data;
        y_min = dtrace.y_min;
        y_max = dtrace.y_max;
        if (dtrace.has_x_trace()) {
            p_xdata = std::make_unique<std::vector<double>>(*dtrace.p_xdata);
        }
        return *this;
    };
    DisplayTrace& operator=(DisplayTrace&& dtrace) = default;

    void reset();
	void render(QPainter& painter, RenderArea* display);
	bool isValid() const { return !m_data.empty(); }
    bool has_x_trace() const { return !!p_xdata; }

    /// <summary>
    /// Converts a x-y-trace to an y-only trace (the default)
    /// of numpoints length by interpolation using
    /// member function interp(x)
    /// </summary>
    /// <param name="numpoints">number of point the resulting trace should have</param>
    /// <param name="new_x0">starting value for x</param>
    /// <param name="new_delta">sample intervall</param>
    void convertToInterpolated(std::size_t numpoints, double new_x0, double new_delta);

    /// <summary>
    /// Converts a x-y-trace to an y-only trace (the default)
    /// matching the templ trace using
    /// member function interp(x)
    /// </summary>
    /// <param name="templ">trace the parameters of which are used as template</param>
    void convertToInterpolated(const DisplayTrace& templ) {
        assert(templ.isValid() && !templ.has_x_trace());
        convertToInterpolated(templ.m_data.size(), templ.m_x0, templ.m_deltax);
    }
    
    /// <summary>
    /// interpolate datapoint for given x value
    /// if x-trace is present, uses linear interpolation
    /// else uses nearest neibor
    /// </summary>
    /// <param name="x">position to interpolate</param>
    /// <returns>if x is in range, interpolated value, nan else</returns>
    double interp(double x);
    QString getXUnit() const { return x_unit; };
    QString getYUnit() const { return y_unit; };
    auto size() { return m_data.size(); };
    const std::vector<double>& data() const { return m_data; };
    double deltax() const { return m_deltax; };
    double x0() const { return m_x0; };
    std::tuple<double, double> getDataMinMax(int pLeft, int pRight) const;
    std::tuple<double, double> getDataMinMax() const
    {
        return { y_min, y_max };
    }
private:
    void set_ymin_ymax();
    double m_x0{}, m_deltax{}, y_min{}, y_max{};
    QString x_unit, y_unit;
	std::vector<double> m_data;
public:
    std::unique_ptr<std::vector<double> > p_xdata;

};

