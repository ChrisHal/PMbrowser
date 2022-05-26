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
#pragma once

#ifndef STIM_TREE_H
#define STIM_TREE_H

#include <vector>
#include "hkTree.h"


class StimSegmentRecord
{
public:
    StimSegmentRecord(const hkTreeNode& node);
    int Class;
    double Voltage;
    int VoltageSource;
    double Duration;
    // for testing:
    const hkTreeNode* Node;
private:

};


/// <summary>
/// ChannelRecord from stim tree
/// </summary>
class ChannelRecord
{
public:
    ChannelRecord(const hkTreeNode& node);
    //~ChannelRecord();
    int LinkedChannel;
    int DacMode;
    std::string DacUnit;
    double Holding;
    std::vector<StimSegmentRecord> Segments;
private:

};


/// <summary>
/// contains needed info about stim record from stim tree
/// </summary>
class StimulationRecord
{
public:
	StimulationRecord(const hkTreeNode& node);
	//~StimulationRecord();
    std::string EntryName;
    int DataStartSegment;
    double DataStartTime;
    int ActualDacChannels;
    bool HasLockIn;
    std::vector<ChannelRecord> Channels;
private:

};



#endif // !STIM_TREE_H
