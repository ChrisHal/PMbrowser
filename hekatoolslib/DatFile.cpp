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
#include <cstdint>
#include "time_handling.h"
#include "helpers.h"
#include "DatFile.h"
#include "machineinfo.h"
#include "PMparameters.h"

void DatFile::InitFromStream(std::istream& infile)
{
    if (!infile) {
        throw std::runtime_error("cannot access file");
    }
    auto bh = std::make_unique<BundleHeader>();
    infile.read(reinterpret_cast<char*>(bh.get()), BundleHeaderSize);
    if (!infile) {
        throw std::runtime_error("cannot read file");
    }

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
}

std::string DatFile::getFileDate() const
{
#ifndef NDEBUG
    return formatPMtimeUTC(Time);
#else
    return formatPMtimeDate(Time);
#endif //NDEBUG
}


void DatFile::formatStimMetadataAsTableExport(std::ostream& os, int max_level)
{
    if (max_level > hkTreeNode::LevelTrace) {
        throw std::runtime_error("max_level exceeds LevelTrace(=4)");
    }
    auto& rootnode = GetPulTree().GetRootNode();
    os << "GrpCount\tSerCount\tSwCount\tTrCount\t";
    getTableHeadersExport(parametersRoot, os) << '\t';
    getTableHeadersExport(parametersGroup, os) << '\t';
    getTableHeadersExport(parametersSeries, os) << '\t';
    getTableHeadersExport(parametersSweep, os) << '\t';
    getTableHeadersExport(parametersTrace, os) << '\n';
    std::string root_entry = formatParamListExportTable(rootnode, parametersRoot);
    for (const auto& grp : rootnode.Children) {
        auto gpr_count = grp.extractValue<int32_t>(GrGroupCount);
        std::string grp_entry = formatParamListExportTable(grp, parametersGroup);
        for (const auto& series : grp.Children) {
            auto se_count = series.extractValue<int32_t>(SeSeriesCount);
            std::string se_entry = formatParamListExportTable(series, parametersSeries);
            for (const auto& sweep : series.Children) {
                auto sw_count = sweep.extractValue<int32_t>(SwSweepCount);
                std::string sw_entry = formatParamListExportTable(sweep, parametersSweep);
                for (const auto& trace : sweep.Children) {
                    auto tr_count = trace.extractValue<int32_t>(TrTraceCount);
                    std::string tr_entry = formatParamListExportTable(trace, parametersTrace);
                    os << gpr_count << '\t' << se_count << '\t' << sw_count << '\t'
                        << tr_count << '\t' << root_entry << '\t' << grp_entry << '\t'
                        << se_entry << '\t' << sw_entry << 't' << tr_entry << '\n';
                    if (max_level < hkTreeNode::LevelTrace) break;
                }
                if (max_level < hkTreeNode::LevelSweep) break;
            }
            if (max_level < hkTreeNode::LevelSeries) break;
        }
        if (max_level < hkTreeNode::LevelGroup) break;
    }
}