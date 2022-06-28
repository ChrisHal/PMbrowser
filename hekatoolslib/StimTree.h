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
#include <array>
#include "hkTree.h"

class StimSegmentRecord
{
public:
    StimSegmentRecord(const hkTreeNode& node);
    SegmentClass Class;
    double Voltage,
        DeltaVFactor,
        DeltaVIncrement;
    int VoltageSource;
    IncrementModeType
        DurationIncMode,
        VoltageIncMode;
    double Duration,
        DeltaTFactor,
        DeltaTIncrement;
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
    bool SetLastSegVmemb;
private:

};


/// <summary>
/// contains needed info about stim record from stim tree
/// </summary>
class StimulationRecord
{
public:
	StimulationRecord(const hkTreeNode& node);
    bool hasStimChannel() const { return ActualDacChannels != 0; };
    const ChannelRecord& getStimChannel() const;
    double getHolding() const;

    /// <summary>
    /// Contruct the theoretical stimulation trace
    /// (might need to be calculated bsed on sweep-count)
    /// </summary>
    /// <param name="sweep_count">zero-base index of sweep</param>
    /// <returns>vector containing x/y coordinate pairs</returns>
    std::vector<std::array<double,2>> constructStimTrace(int sweep_count) const;

	//~StimulationRecord();
    std::string EntryName;
    int DataStartSegment;
    double DataStartTime;
    int NumberSweeps;
    int ActualDacChannels;
    bool HasLockIn;
    std::vector<ChannelRecord> Channels;
private:

};

class StimRootRecord
{
public:
    StimRootRecord(const hkTreeNode& node);
    std::vector<StimulationRecord> Stims;

private:

};


#endif // !STIM_TREE_H
