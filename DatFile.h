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

// some basic structs to handle HEKA Patchmaster dat-files

#pragma once
#include <cstdint>
#include "hkTree.h"

struct BundleItem {
	int32_t Start;
	int32_t Length;
	char Extension[8];
};

struct BundleHeader {
	char Signature[8];
	char Version[32];
	double Time;
	int32_t Items;
	char IsLittleEndian;
	char pad[11];
	BundleItem BundleItems[12];
};

constexpr int32_t BundleHeaderSize = 256;
constexpr char BundleSignature[8] = "DAT2\0\0\0";
constexpr char ExtDat[] = ".dat", ExtPul[] = ".pul", ExtPgf[] = ".pgf", ExtAmp[] = ".amp";

class DatFile
{
	int32_t offsetDat, lenDat; // file offset and blocklength of rawdata
	double Time;
	bool isSwapped;
	hkTree PulTree, PgfTree, AmpTree;
public:
	DatFile() : offsetDat{ 0 }, lenDat{ 0 }, Time{ 0.0 }, isSwapped{ false }, PulTree{},
		PgfTree{}, AmpTree{} {};
	bool InitFromStream(std::istream& istream);
	hkTree& GetPulTree() { return PulTree; };
	hkTree& GetPgfTree() { return PgfTree; };
	hkTree& GetAmpTree() { return AmpTree; };
	double GetTime() { return Time; };
};

// offsets into data record fields that we are interested in
constexpr size_t stExtTrigger = 164, // in Stimulation record
TrTraceCount = 36,  // in Trace Record
TrData = 40,
TrDataPoints = 44,
TrDataKind = 64,
TrDataFormat = 70,
TrDataScaler = 72,
TrYUnit = 96,
TrXInterval = 104,
TrXStart = 112,
TrXUnit = 120,
TrSealResistance = 168,
TrCSlow = 176,
TrGSeries = 184,
TrRsValue = 192,
TrSelfChannel = 288,
TrInterleaveSize = 292,
TrTrHolding = 408,
SwLabel = 4,
SwStimCount = 40,
SwTime = 48,
SeLabel = 4, //(*String32Type*)
SeVoltage = 8,
SeSeriesCount = 116,
SeAmplStateRef = 128,
GrLabel=   4,
GrGroupCount = 120,
RoVersionName = 8, // root record
RoStartTime = 520;

// TrDataKind
constexpr uint16_t LittleEndianBit = 1, IsLeak = 1 << 1, IsImon = 1 << 3, IsVmon = 1 << 4, ClipBit = 1 << 5;

// TrDataFormatType
constexpr char DFT_int16 = 0, DFT_int32 = 1, DFT_float = 2, DFT_double = 3;

// some helpers
inline int32_t extractInt32(const char* data, size_t offset)
{
	return *reinterpret_cast<const int32_t*>(data + offset);
}

inline uint16_t extractUInt16(const char* data, size_t offset)
{
	return *reinterpret_cast<const uint16_t*>(data + offset);
}

inline double extractLongReal(const char* data, size_t offset)
{
	return *reinterpret_cast<const double*>(data + offset);
}
