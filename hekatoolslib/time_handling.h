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

#ifndef TIME_HANDLING_H
#define TIME_HANDLING_H

#pragma once

/// <summary>
/// Convert time as found in .dat file, only return date part
/// (ignore time)
/// </summary>
/// <param name="t">time found in .dat file</param>
/// <returns>std::string containing formatted date</returns>
std::string formatPMtimeDate(double t); // date only

/// <summary>
/// Convert time as found in .dat file to date and UTC time
/// </summary>
/// <param name="t">time found in .dat file</param>
/// <returns>std::string containing formatted date and UTC time</returns>
std::string formatPMtimeUTC(double t); // UTC date/time

#endif // !TIME_HANDLING_H
