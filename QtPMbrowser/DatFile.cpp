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
#define _CRT_SECURE_NO_WARNINGS // get rid of some unnecessary warnings om Windows
#include <istream>
#include <algorithm>
#include <cstring>
#include <ctime>
#include <cmath>
#include <cassert>
#include <cinttypes>
#include "time_handling.h"
#include "helpers.h"
#include "DatFile.h"
#include "machineinfo.h"

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
    BundleHeader* bh = reinterpret_cast<BundleHeader*>(buffer);
    bool isValid = std::strncmp(bh->Signature, BundleSignature, 8) == 0;
    bool isInvalidBundle = std::strncmp(bh->Signature, BundleSignatureInvalid, 8) == 0;
    if (!isValid) {
        if (isInvalidBundle) {
            throw std::runtime_error("invalid bundle signature");
        }
        else {
            throw std::runtime_error("invalid file (not a PM dat file)");
        }
    }
    isSwapped = bool(bh->IsLittleEndian) != MachineIsLittleEndian();
    Version = bh->Version;
    Time = bh->Time;
    if (isSwapped) {
        swapInPlace(Time);
    }

    auto nitems = bh->Items;
    if (isSwapped) {
        swapInPlace(nitems);
    }

    nitems = std::min(nitems, 12); // make sure malformed files do not cause out of bounds read
    for (int i = 0; i < nitems; ++i) {
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
            res = PulTree.InitFromStream(infile, item.Start, item.Length);
        }
        else if (std::strcmp(item.Extension, ExtPgf) == 0) {
            // process pgf tree
            res = PgfTree.InitFromStream(infile, item.Start, item.Length);
        }
        else if (std::strcmp(item.Extension, ExtAmp) == 0) {
            // process amp tree
            res = AmpTree.InitFromStream(infile, item.Start, item.Length);
        }
        if (!res) {
            throw std::runtime_error("error processing tree");
        }
    }
    return true;
}

std::string DatFile::getFileDate() const
{
#ifndef NDEBUG
    return formatPMtimeUTC(Time);
#else
    return formatPMtimeDate(Time);
#endif //NDEBUG
}