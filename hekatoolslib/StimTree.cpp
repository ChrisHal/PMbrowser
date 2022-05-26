/*
    Copyright 2022 Christian R. Halaszovich

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

#include "StimTree.h"
#include <cassert>

StimRootRecord::StimRootRecord(const hkTreeNode& node)
{
    assert(node.getLevel() == 0);
    for (const auto& c : node.Children) {
        Stims.emplace_back(c);
    }
}

StimulationRecord::StimulationRecord(const hkTreeNode& node)
{
    assert(node.getLevel() == 1);
    EntryName = node.getString(stEntryName);
    DataStartSegment = node.extractValue<int32_t>(stDataStartSegment);
    DataStartTime = node.extractValue<double>(stDataStartTime);
    ActualDacChannels = node.extractValue<int32_t>(stActualDacChannels);
    HasLockIn = node.getChar(stHasLockIn);
    for (const auto& c : node.Children) {
        Channels.emplace_back(c);
    }
}

const ChannelRecord& StimulationRecord::getStimChannel() const
{
    auto linked = Channels.front().LinkedChannel;
    return Channels.at(linked);
}

double StimulationRecord::getHolding() const
{
    return getStimChannel().Holding;
}

std::vector<std::array<double, 2>> StimulationRecord::constructStimTrace(int sweep_count) const
{
    std::vector< std::array<double, 2>> points;
    double curr_t{ 0.0 };
    auto holding = getHolding();
    if (hasStimChannel()) {
        const auto& stim_ch = getStimChannel();
        const auto& first_seg = stim_ch.Segments.front();
        if ((first_seg.Class == SegmentClass::Constant || first_seg.Class == SegmentClass::Continuous)
            && first_seg.VoltageSource != 1 && first_seg.Voltage != holding) {
            points.push_back({ curr_t, holding });
        }
        for (const auto& seg : stim_ch.Segments) {
            double actV{seg.Voltage};
            if (seg.VoltageSource == 1) {
                actV = holding;
            }
            if (seg.Class == SegmentClass::Constant ||
                seg.Class == SegmentClass::Continuous) {
                points.push_back({ curr_t,actV });
                curr_t += seg.Duration;
                points.push_back({ curr_t,actV });
            }
            else if (seg.Class == SegmentClass::Ramp) {
                curr_t += seg.Duration;
                points.push_back({ curr_t,actV });
            }
            else {
                throw std::runtime_error("unsupported segment class '"
                    + std::string(SegmentClassNames.at(static_cast<int>(seg.Class)))+"'");
            }
        }
        if (!stim_ch.SetLastSegVmemb) {
            const auto& last_seg = stim_ch.Segments.back();
            if (last_seg.Voltage != holding && last_seg.VoltageSource != 1) {
                // we jump back to holding after last segment
                points.push_back({ curr_t, holding });
            }
        }
    }
    else {
        points.push_back({ curr_t, holding });
    }
    return points;
}

ChannelRecord::ChannelRecord(const hkTreeNode& node) :
    LinkedChannel{node.extractInt32(chLinkedChannel) - 1},
    DacMode{node.getChar(chDacMode)}, DacUnit{node.getString(chDacUnit)},
    Holding{ node.extractLongReal(chHolding) },
    SetLastSegVmemb{ static_cast<bool>(node.getChar(chSetLastSegVmemb)) }
{
    assert(node.getLevel() == 2);
    
    for (const auto& c : node.Children) {
        Segments.emplace_back(c);
    }
}

StimSegmentRecord::StimSegmentRecord(const hkTreeNode& node) :
    Class{ node.getChar(seClass) }, Voltage{node.extractLongReal(seVoltage)},
    VoltageSource{node.extractInt32(seVoltageSource)}, Duration{node.extractLongReal(seDuration)}, Node{&node}
{
    assert(node.getLevel() == 3);
}
