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

ChannelRecord::ChannelRecord(const hkTreeNode& node) :
    LinkedChannel{node.extractInt32(chLinkedChannel) - 1},
    DacMode{node.getChar(chDacMode)}, DacUnit{node.getString(chDacUnit)},
    Holding{node.extractLongReal(chHolding)}
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
