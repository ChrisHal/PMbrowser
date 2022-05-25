/** \file
 *	Classes and functions to handle PatchMaster dat files.
 */

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



// some basic structs to handle HEKA Patchmaster dat-files

#pragma once
#include <cstdint>
#include <string>
#include <algorithm>
#include <type_traits>
#include <cassert>
#include "machineinfo.h"
#include "hkTree.h"

/// <summary>
/// each BundleItem describes offset and length of
/// a tree stored in the bundled dat file
/// </summary>
struct BundleItem {
	int32_t Start; // file offset of tree/data 
	int32_t Length; // length of data in bytes 
	char Extension[8]; // file extension of tree, e.g. .pul
};

/// <summary>
/// header of bundled .dat file
/// </summary>
struct BundleHeader {
	char Signature[8]; // "DAT2" for valid files
	char Version[32]; // name and version of file creator
	double Time; // creation time in PatchMaster format (see time_handling.h)
	int32_t Items;
	char IsLittleEndian;
	char pad[11]; // unused
	BundleItem BundleItems[12]; // one item for each tree stored in file
};
constexpr auto BundleHeaderSize = 256;
static_assert(sizeof(BundleHeader) == BundleHeaderSize, "unexpected size of BundleHeader");
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
	void InitFromStream(std::istream& istream);
	std::string getFileDate() const; // return formatted file creation date
	hkTree& GetPulTree() { return PulTree; };
	hkTree& GetPgfTree() { return PgfTree; };
	hkTree& GetAmpTree() { return AmpTree; };
	std::string getVersion() const { return Version; }; // returns name and version of file creator
	double GetTime() const { return Time; }; // return creation time in PatchMaster format (see time_handling.h)
	bool getIsSwapped() const { return isSwapped; };
	/// <summary>
	/// create the header (1st line) containing the metadatafields
	/// </summary>
	/// <param name="os">stream to receive result</param>
	static void metadataCreateTableHeader(std::ostream& os);
	/// <summary>
	/// format metadata with set export flag as tab delimited table
	/// </summary>
	/// <param name="os">stream to receive output</param>
	/// <param name="max_level">one line per this level will be exported</param>
	void formatStimMetadataAsTableExport(std::ostream& os, int max_level);

	/// <summary>
	/// Gets the V or I holding from trace, stores appropiate unit
	/// </summary>
	/// <param name="trace">trace-node</param>
	/// <param name="unit">receives unit</param>
	/// <returns>holding value</returns>
	double getTraceHolding(const hkTreeNode& trace, std::string& unit);
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
TrLinkDAChannel = 216, // int32
TrSelfChannel = 288,
TrInterleaveSize = 292,
TrInterleaveSkip = 296,
TrTrHolding = 408,
SwLabel = 4, // for Sweep ...
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


/// stim tree
// from channel record
constexpr size_t
	chLinkedChannel = 4, //int32
	chAdcChannel = 20, // (*INT16*)
	chAdcMode = 22, // (*BYTE*)
	chDacChannel = 28, // (*INT16*)
	chDacMode = 30, // (*BYTE*)
	chDacUnit = 40, // String8Type
	chHolding = 48, // LONGREAL, for CC in micro-ampere!
// for stim recorde
	stEntryName = 4, //(*String32Type*)
	stDataStartSegment = 100, // (*INT32*)
	stDataStartTime = 104, // LongReal
// for segment record
	seVoltage = 8;

// TrDataKind
constexpr uint16_t LittleEndianBit = 1, IsLeak = 1 << 1, IsImon = 1 << 3, IsVmon = 1 << 4, ClipBit = 1 << 5;

// TrDataFormatType
constexpr char DFT_int16 = 0, DFT_int32 = 1, DFT_float = 2, DFT_double = 3;

// some routine to read trace data

/// <summary>
/// read trace data from dat file and convert to double using 
/// the appropiate data-scaler (and byte swapping if needed) as specified in the trace record
/// </summary>
/// <typeparam name="T">type of raw data (short, long, float or double)</typeparam>
/// <param name="datafile">stream (usually file-stream) from which to read data</param>
/// <param name="TrRecord">trace record specifying the trace to be loaded</param>
/// <param name="trdatapoints">number of datapoints (also size of target buffer provided by caller)</param>
/// <param name="target">pointer to buffer allocated by caller, must have space for trdatapoints doubles</param>
template<typename T> void ReadScaleAndConvert(std::istream& datafile, hkTreeNode& TrRecord, std::size_t trdatapoints, 
	double* target)
{
	static_assert(std::is_arithmetic_v<T>, "must be arithmetic type");
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
