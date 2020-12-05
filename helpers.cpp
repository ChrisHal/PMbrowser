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

#include<cinttypes>
#include <sstream>
#include "hkTree.h"
#include "DatFile.h"
#include"helpers.h"

// some tools

double swap_bytes(double x)
{
    char* a = reinterpret_cast<char*>(&x);
    uint64_t b;
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
    float b;
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
    int32_t b;
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
    int32_t b;
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
    return ((x & 0xff00) >> 8) | ((x & 0xff) << 8);
}

std::string formTraceName(const hkTreeNode& tr, int count)
{
    int32_t datakind = tr.extractUInt16(TrDataKind);
    std::stringstream trace_ext;
    if (datakind & IsImon) {
        trace_ext << "Imon";
    }
    else if (datakind & IsVmon) {
        trace_ext << "Vmon";
    }
    else {
        trace_ext << tr.getString(TrLabel);
        if (trace_ext.str().length() == 0) {
            if (datakind & IsLeak) {
                trace_ext << "Leak";
            }
            else {
                trace_ext << "trace_" << count;
            }
        }
    }
    return trace_ext.str();
}