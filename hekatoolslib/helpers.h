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

#ifndef HELPERS_H
#define HELPERS_H


/** \file
 * various helper functions, e.g. for byte-swapping
 */

#pragma once
//#include "hkTree.h"
#include <cstdint>

template<typename T> T swap_bytes(T) = delete;
double swap_bytes(double x);
float swap_bytes(float x);
int32_t swap_bytes(int32_t x);
uint32_t swap_bytes(uint32_t x);
int16_t swap_bytes(int16_t x);
uint16_t swap_bytes(uint16_t x);

template<typename T> void swapInPlace(T& x)
{
    x = swap_bytes(x);
}

struct hkTreeNode;

/// <summary>
/// form canonical displayname for trace
/// treat Vmon, Imon and to some extend Leak traces
/// as special cases.
/// If a lable is provided in the trace-record and the trace is
/// neither of type Vmon nor Imon, the lable will be used. Otherwise,
/// leak traces will be labled "leak", all remaining traces will
/// be labled "trace_&lt;count&gt;".
/// </summary>
/// <param name="tr">trace record</param>
/// <param name="count">count of trace (starting from 1)</param>
/// <returns>string containing canonical trace-name</returns>
std::string formTraceName(const hkTreeNode& tr, int count);

#endif // !HELPERS_H
