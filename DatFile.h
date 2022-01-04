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
#include <string>
#include <algorithm>
#include <cassert>
#include "machineinfo.h"
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
constexpr char BundleSignature[8] = "DAT2\0\0\0", BundleSignatureInvalid[8]= "DAT1\0\0\0";
constexpr char ExtDat[] = ".dat", ExtPul[] = ".pul", ExtPgf[] = ".pgf", ExtAmp[] = ".amp";

class DatFile
{
	int32_t offsetDat, lenDat; // file offset and blocklength of rawdata
	std::string Version;
	double Time; // file time given in header
	bool isSwapped;
	hkTree PulTree, PgfTree, AmpTree;
public:
	DatFile() : offsetDat{ 0 }, lenDat{ 0 }, Version{}, Time{ 0.0 }, isSwapped{ false }, PulTree{},
		PgfTree{}, AmpTree{} {};
	bool InitFromStream(std::istream& istream);
	std::string getFileDate() const;
	hkTree& GetPulTree() { return PulTree; };
	hkTree& GetPgfTree() { return PgfTree; };
	hkTree& GetAmpTree() { return AmpTree; };
	std::string getVersion() const { return Version; };
	double GetTime() const { return Time; };
	bool getIsSwapped() const { return isSwapped; };
};

enum RecordingModeType {
	InOut=0,
	OnCell=1,
	OutOut=2,
	WholeCell=3,
	CClamp=4,
	VClamp=5,
	NoMode=6
};

// offsets into data record fields that we are interested in
constexpr size_t stExtTrigger = 164, // in Stimulation record
TrLabel = 4,
TrTraceCount = 36,  // in Trace Record
TrData = 40,
TrDataPoints = 44,
TrDataKind = 64,
TrRecordingMode = 68,
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
TrInterleaveSkip = 296,
TrTrHolding = 408,
SwLabel = 4,
SwStimCount = 40,
SwSweepCount = 44,
SwTime = 48,
SwTimer = 56,
SeLabel = 4, //(*String32Type*)
SeSeriesCount = 116,
SeAmplStateFlag = 124, // flag > 0 => load local oldAmpState, otherwise load from .amp File
SeAmplStateRef = 128, // ref  = 0 => use local oldAmpState. Caution: This is a 1-based offset!
SeTime = 136,
SeOldAmpState = 472,
GrLabel = 4,
GrGroupCount = 120,
RoVersionName = 8, // root record
RoStartTime = 520,
// now from Amp records:
RoAmplifierName = 40,
RoAmplifier = 72, // CHAR
RoADBoard = 73, // CHAR
// For AmplStateRecord:
AmplifierStateSize = 400,
AmStateCount = 4,
AmAmplifierState = 112;

// offset for Stim-Tree
constexpr size_t seVoltage = 8;

// TrDataKind
constexpr uint16_t LittleEndianBit = 1, IsLeak = 1 << 1, IsImon = 1 << 3, IsVmon = 1 << 4, ClipBit = 1 << 5;

// TrDataFormatType
constexpr char DFT_int16 = 0, DFT_int32 = 1, DFT_float = 2, DFT_double = 3;

// some routines to read trace data
template<typename T> void ReadScaleAndConvert(std::istream& datafile, hkTreeNode& TrRecord, std::size_t trdatapoints, 
	double* target)
{
	assert(trdatapoints == TrRecord.extractInt32(TrDataPoints));
	int32_t trdata = TrRecord.extractInt32(TrData);
	datafile.seekg(trdata);

	int32_t     interleavesize = TrRecord.extractValue<int32_t>(TrInterleaveSize, 0),
		interleaveskip = TrRecord.extractValue<int32_t>(TrInterleaveSkip, 0);
	uint16_t tracekind = TrRecord.extractUInt16(TrDataKind);
	bool need_swap = bool(tracekind & LittleEndianBit) != MachineIsLittleEndian();
	double datascaler = TrRecord.extractLongReal(TrDataScaler);

	auto source = std::make_unique<T[]>(trdatapoints);
	if (interleavesize == 0) {
		datafile.read((char*)source.get(), sizeof(T) * trdatapoints);
	}
	else { // it's interleaved data
		assert(interleaveskip >= interleavesize);
		std::size_t bytesremaining = sizeof(T) * trdatapoints;
		int bytestoskip = interleaveskip - interleavesize; // interleaveskip is from block-start to block-start!
		char* p = (char*)source.get();
		while (bytesremaining > 0) {
			auto bytestoread = std::min(bytesremaining, std::size_t(interleavesize));
			datafile.read(p, bytestoread);
			if (!datafile) { break; }
			p += bytestoread;
			bytesremaining -= bytestoread;
			if (bytesremaining > 0) {
				datafile.seekg(bytestoskip, std::ios::cur); // skip to next block
			}
		}
	}
	if (!datafile) {
		throw std::runtime_error("error while reading datafile");
	}
	if (!need_swap) {
		std::transform(source.get(), source.get() + trdatapoints, target, [=](T x) { return datascaler * x; });
	}
	else {
		std::transform(source.get(), source.get() + trdatapoints, target, [=](T x) {
			return datascaler * swap_bytes(x); });
	}
}
