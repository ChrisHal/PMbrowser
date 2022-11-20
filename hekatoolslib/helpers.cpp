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

#include <cinttypes>
#include <sstream>
#include <cassert>
#include "hkTree.h"
#include "DatFile.h"
#include "helpers.h"

// some tools

double swap_bytes(double x)
{
    char* a = reinterpret_cast<char*>(&x);
    uint64_t b{};
    char* p = reinterpret_cast<char*>(&b);
    p[0] = a[7];
    p[1] = a[6];
    p[2] = a[5];
    p[3] = a[4];
    p[4] = a[3];
    p[5] = a[2];
    p[6] = a[1];
    p[7] = a[0];
    return *reinterpret_cast<double*>(p);
}

float swap_bytes(float x)
{
    char* a = reinterpret_cast<char*>(&x);
    float b{};
    char* p = reinterpret_cast<char*>(&b);
    p[0] = a[3];
    p[1] = a[2];
    p[2] = a[1];
    p[3] = a[0];
    return b;
}

int32_t swap_bytes(int32_t x)
{
    char* a = reinterpret_cast<char*>(&x);
    int32_t b{};
    char* p = reinterpret_cast<char*>(&b);
    p[0] = a[3];
    p[1] = a[2];
    p[2] = a[1];
    p[3] = a[0];
    return b;
}

uint32_t swap_bytes(uint32_t x)
{
    char* a = reinterpret_cast<char*>(&x);
    uint32_t b{};
    char* p = reinterpret_cast<char*>(&b);
    p[0] = a[3];
    p[1] = a[2];
    p[2] = a[1];
    p[3] = a[0];
    return b;
}

int16_t swap_bytes(int16_t x)
{
    return ((x & 0xff00) >> 8)|((x & 0xff) << 8);
}

uint16_t swap_bytes(uint16_t x)
{
    return ((x & 0xff00U) >> 8U) | ((x & 0xffU) << 8U);
}

hkSettings global_hkSettings{};


/// <summary>
/// GEnerate canonical name for trace using extensions
/// for Imon and Vmon and possibly Leak traces given in global_hkSettings.
/// (LEak traces are a special case, since there might be several of them.)
/// If strings in settings are empty, lables provided in DAT-file will be used,
/// if available. Otherwise, trace count will be used.
/// </summary>
/// <param name="tr">tree node the must repesent a trace</param>
/// <param name="count">trace count used for labeling</param>
/// <returns>trace name</returns>
std::string formTraceName(const hkTreeNode& tr, int count)
{
    assert(tr.getLevel() == hkTreeNode::LevelTrace);
    int datakind = tr.extractUInt16(TrDataKind);
    std::stringstream trace_ext;
    if (datakind & IsImon && !global_hkSettings.ext_Imon.empty()) {
        trace_ext << global_hkSettings.ext_Imon;
    }
    else if (datakind & IsVmon && !global_hkSettings.ext_Vmon.empty()) {
        trace_ext << global_hkSettings.ext_Vmon;
    }
    else {
        auto lable = tr.getString(TrLabel);
        if (lable.empty()) {
            if (datakind & IsLeak && !global_hkSettings.ext_Leak.empty()) {
                trace_ext << global_hkSettings.ext_Leak;
            }
            else {
                trace_ext << "trace_" << count;
            }
        }
        else {
            trace_ext << lable;
        }
    }
    return trace_ext.str();
}