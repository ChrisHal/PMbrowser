/*
    Copyright 2020 - 2023 Christian R. Halaszovich

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
#include <iomanip>

using namespace hkLib;

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

    bool isValid = std::memcmp(bh->Signature, BundleSignature, 8) == 0;
    if (!isValid) {
        bool isInvalidBundle = std::memcmp(bh->Signature, BundleSignatureInvalid, 8) == 0;
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
            if (offsetDat < 0 || lenDat <= 0) throw std::runtime_error("invalid data offset or length");
        }
        else if (std::strcmp(item.Extension, ExtPul) == 0) {
            // process pulse tree
            res = PulTree.InitFromStream(ExtPul, infile, item.Start, item.Length);
            PulTree.GetRootNode().setAsTime0();
        }
        else if (std::strcmp(item.Extension, ExtPgf) == 0) {
            // process pgf tree
            res = PgfTree.InitFromStream(ExtPgf, infile, item.Start, item.Length);
        }
        else if (std::strcmp(item.Extension, ExtAmp) == 0) {
            // process amp tree
            res = AmpTree.InitFromStream(ExtAmp, infile, item.Start, item.Length);
        }
        if (!res) {
            throw std::runtime_error("error processing tree");
        }
    }
    // make reasonably certain we succeded at loading and file is valid:
    if (lenDat == 0) throw std::runtime_error("no data in file");
    if (!PulTree.isValid()) throw std::runtime_error("no valid Pulse tree in file");
    if (!PgfTree.isValid())throw std::runtime_error("no valid Pgf in file");
}

std::string DatFile::getFileDate() const
{
#ifndef NDEBUG
    return formatPMtimeUTC(Time);
#else
    return formatPMtimeDate(Time);
#endif //NDEBUG
}

void DatFile::metadataCreateTableHeader(std::ostream& os)
{
    os << "GrpCount\tSerCount\tSwCount\tTrCount";
    getTableHeadersExport(parametersGroup, os);
    getTableHeadersExport(parametersSeries, os);
    getTableHeadersExport(parametersSweep, os);
    getTableHeadersExport(parametersTrace, os) << '\n';
}

void DatFile::formatStimMetadataAsTableExport(std::ostream& os, int max_level)
{
    if (max_level > hkTreeNode::LevelTrace) {
        throw std::runtime_error("max_level exceeds LevelTrace(=4)");
    }
    os << std::setprecision(10);
    auto& rootnode = GetPulTree().GetRootNode();
    metadataCreateTableHeader(os);
    for (const auto& grp : rootnode.Children) {
        auto gpr_count = grp.extractValue<std::int32_t>(GrGroupCount);
        std::string grp_entry = formatParamListExportTable(grp, parametersGroup);
        for (const auto& series : grp.Children) {
            auto se_count = series.extractValue<std::int32_t>(SeSeriesCount);
            std::string se_entry = formatParamListExportTable(series, parametersSeries);
            for (const auto& sweep : series.Children) {
                auto sw_count = sweep.extractValue<std::int32_t>(SwSweepCount);
                std::string sw_entry = formatParamListExportTable(sweep, parametersSweep);
                for (const auto& trace : sweep.Children) {
                    auto tr_count = trace.extractValue<std::int32_t>(TrTraceCount);
                    std::string tr_entry = formatParamListExportTable(trace, parametersTrace);
                    os << gpr_count << '\t' << se_count << '\t' << sw_count << '\t'
                        << tr_count  <<
                        grp_entry 
                        << se_entry << sw_entry << tr_entry << '\n';
                    if (max_level < hkTreeNode::LevelTrace) break;
                }
                if (max_level < hkTreeNode::LevelSweep) break;
            }
            if (max_level < hkTreeNode::LevelSeries) break;
        }
        if (max_level < hkTreeNode::LevelGroup) break;
    }
}

double DatFile::getTraceHolding(const hkTreeNode& trace, std::string& unit)
{
    assert(trace.getLevel() == hkTreeNode::LevelTrace);
    double holding = trace.extractLongRealNoThrow(TrTrHolding);
    int mode = trace.getChar(TrRecordingMode);
    unit = "V";
    if (mode == CClamp) {
        unit = "A";
    }
    if (std::isnan(holding)) {
        // we can also try to get this info from the stim tree (usuful for old files):
        auto linkedDAchannel = trace.extractValue<std::int32_t>(TrLinkDAChannel) - 1;
        assert(linkedDAchannel >= 0);
        const auto& sweep_record = *trace.getParent();
        int stim_index = sweep_record.extractValue<std::int32_t>(SwStimCount) - 1;
        const auto& stim_node = GetPgfTree().GetRootNode().Children.at(stim_index);
        const auto& channel0_record = stim_node.Children.at(linkedDAchannel);
        int linked_channel = channel0_record.extractInt32(chLinkedChannel) - 1;
        const auto& stimchannel_record = stim_node.Children.at(linked_channel);
        unit = stimchannel_record.getString(chDacUnit);
        holding = stimchannel_record.extractLongRealNoThrow(chHolding);
        if (unit == "A") {
            holding *= 1e-6; // for some strange reason this is in microA
        }
#ifndef NDEBUG
        unit += "srec";
#endif // !NDEBUG

    }
    return holding;
}
