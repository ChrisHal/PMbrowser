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
#define _CRT_SECURE_NO_WARNINGS // get rid of some unnecessary warnings om Windows
#include <istream>
#ifdef _DEBUG
#include <iostream>
#endif
#include <algorithm>
#include <cstring>
#include <ctime>
#include <cmath>
#include <cassert>
#include <cinttypes>
#include "helpers.h"
#include "DatFile.h"
static_assert(sizeof(BundleHeader) == 256, "unexpected size of BundleHeader");

constexpr std::time_t EPOCHDIFF_MAC_UNIX = 2082844800;
constexpr double JanFirst1990MACTime = 1580947200.0; // better value?
constexpr auto HIGH_DWORD = 4294967296.0;

std::time_t PMtime2time_t(double t)
{
    t -= JanFirst1990MACTime;
    if (t < 0.0) {
        t += HIGH_DWORD; // why is this necessary?
    }
    return std::time_t(std::floor(t)) - EPOCHDIFF_MAC_UNIX;
}


bool DatFile::InitFromStream(std::istream& infile)
{
    char buffer[BundleHeaderSize];
    if (!infile) {
        throw std::runtime_error("cannot access file");
    }
    infile.read(buffer, BundleHeaderSize);
    if (!infile) {
        throw std::runtime_error("cannot read file");
    }
    bool isValid = std::strncmp(buffer, BundleSignature, 8) == 0;
    BundleHeader* bh = reinterpret_cast<BundleHeader*>(buffer);
    if (!isValid) {
        throw std::runtime_error("invalid file");
    }
    if (!bh->IsLittleEndian) {
        isSwapped = true;
    }
    Version = bh->Version;
    Time = bh->Time;
    if (isSwapped) {
        swapInPlace(Time);
    }


    //int filesize = infile.seekg(0, std::ios::end).tellg();
    auto nitems = bh->Items;
    if (isSwapped) {
        swapInPlace(nitems);
    }

    nitems = std::min(nitems, 12); // make sure malformed files do not cause out of bounds read
    for (int i = 0; i < nitems; ++i) {
        //assert(!!infile);
        auto& item = bh->BundleItems[i];
        if (isSwapped) {
            swapInPlace(item.Length);
            swapInPlace(item.Start);
        }
        bool res = true;
        if (std::strcmp(item.Extension, ExtDat) == 0) {
            offsetDat = item.Start;
            lenDat = item.Length;
        }
        else if (std::strcmp(item.Extension, ExtPul) == 0) {
            // process pulse tree
#ifdef _DEBUG
            std::cerr << "loading pul tree\n";
#endif // _DEBUG
            res = PulTree.InitFromStream(infile, item.Start, item.Length);
        }
        else if (std::strcmp(item.Extension, ExtPgf) == 0) {
            // process pgf tree
#ifdef _DEBUG
            std::cerr << "loading pgf tree\n";
#endif // _DEBUG
            res = PgfTree.InitFromStream(infile, item.Start, item.Length);
        }
        else if (std::strcmp(item.Extension, ExtAmp) == 0) {
#ifdef _DEBUG
            std::cerr << "loading amp tree\n";
#endif // _DEBUG
            // process amp tree
            res = AmpTree.InitFromStream(infile, item.Start, item.Length);
        }
        if (!res) {
            throw std::runtime_error("error processing tree");
        }
    }
    return true;
}

std::string DatFile::getFileDate()
{
    auto unixtime = PMtime2time_t(Time);
    char buffer[128];
    std::strftime(buffer, 128, "%F", localtime(&unixtime));
    return std::string(buffer);
}