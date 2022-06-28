/*
	Copyright 2020 -2022 Christian R. Halaszovich

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

// Routines to export traces as Igor Binary Waves (ibw)

#include <istream>
#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <sstream>
#include <stdexcept>
#include <memory>
#include <cstring>
#include <cassert>
#include "helpers.h"
#include "hkTree.h"
#include "DatFile.h"
#include "PMparameters.h"
#include "exportIBW.h"
#include "igor_ipf.h"
#include "Igor_IBW.h"

/// <summary>
/// Calculate checksum for Igor binary waves
/// Modified from code given by wavemetrics to avoid violation of strict
/// aliasing rules and silent type conversions
/// Still, it is somewhat odd...
/// </summary>
/// <param name="data">pointer to data for which to calculate the checksum</param>
/// <param name="oldcksum">old value of checksum</param>
/// <param name="numbytes">number of bytes of data</param>
/// <returns>checksum (couriously, a some point they are converted to short)</returns>
static int16_t Checksum(const char* data, int16_t oldcksum, std::size_t numbytes)
{
	auto numshorts = numbytes >> 1;				// 2 bytes to a short -- ignore trailing odd byte.
	int32_t cksum = oldcksum;
	while (numshorts-- > 0) {
		int16_t t;
		std::memcpy(&t, data, sizeof t);
		cksum += t;
		data += sizeof t;
	}
	return cksum & 0xffff;
}

std::string MakeWaveNote(hkTreeNode& TrRecord)
{
	std::stringstream note;
	formatParamListExportIBW(*TrRecord.getParent()->getParent()->getParent()->getParent(), parametersRoot, note);
	formatParamListExportIBW(*TrRecord.getParent()->getParent()->getParent(), parametersGroup, note);
	formatParamListExportIBW(*TrRecord.getParent()->getParent(), parametersSeries, note);
	formatParamListExportIBW(*TrRecord.getParent(), parametersSweep, note);
	formatParamListExportIBW(TrRecord, parametersTrace, note);
	return note.str();
}


void ExportTrace(std::istream& datafile, hkTreeNode& TrRecord, std::ostream& outfile, const std::string& wavename)
{
	assert(TrRecord.getLevel() == hkTreeNode::LevelTrace);
	char dataformat = TrRecord.getChar(TrDataFormat);

	std::string xunit, yunit;
	yunit = TrRecord.getString(TrYUnit); // assuming the string is zero terminated...
	xunit = TrRecord.getString(TrXUnit);
	double x0 = TrRecord.extractLongReal(TrXStart), deltax = TrRecord.extractLongReal(TrXInterval);
	int32_t trdatapoints = TrRecord.extractInt32(TrDataPoints);

	auto target = std::make_unique<double[]>(trdatapoints);

	switch (dataformat)
	{
	case DFT_int16:
		ReadScaleAndConvert<int16_t>(datafile, TrRecord, trdatapoints, target.get());
		break;
	case DFT_int32:
		ReadScaleAndConvert<int32_t>(datafile, TrRecord, trdatapoints, target.get());
		break;
	case DFT_float:
		ReadScaleAndConvert<float>(datafile, TrRecord, trdatapoints, target.get());
		break;
	case DFT_double:
		ReadScaleAndConvert<double>(datafile, TrRecord, trdatapoints, target.get());
		break;
	default:
		throw std::runtime_error("unknown data format type");
		break;
	}

	std::string note{ MakeWaveNote(TrRecord) };
	
	BinHeader5 bh{};
	// make sure the packing of the structs is as expected:
	static_assert(sizeof(bh) == 64, "wrong size of bh");

	WaveHeader5 wh{};
	constexpr size_t numbytes_wh = offsetof(WaveHeader5, wData);
	static_assert(numbytes_wh == 320, "wrong size of wh");

	bh.version = 5;
	bh.noteSize = int32_t(note.size());
	bh.wfmSize = int32_t(numbytes_wh + sizeof(double) * trdatapoints);
	// we will calculate checksum later, all other entries in bh remain 0
	wh.type = NT_FP64;
	wavename.copy(wh.bname, MAX_WAVE_NAME5);
	wh.npnts = trdatapoints;
	wh.nDim[0] = trdatapoints;
	std::memcpy((void*)wh.sfA, (void*)&deltax, sizeof(double));
	std::memcpy((void*)wh.sfB, (void*)&x0, sizeof(double));
	wh.whVersion = 1; // yes, for version 5 files files this must be 1...
	xunit.copy(wh.dimUnits[0], MAX_UNIT_CHARS);
	yunit.copy(wh.dataUnits, MAX_UNIT_CHARS);
	wh.platform = 2;

	// NOTE: in original code "cksum" is a short, triggering a conversion from 32bit int to short
	auto cksum = Checksum(reinterpret_cast<char*>(&bh), 0, sizeof(bh));
	cksum = Checksum(reinterpret_cast<char*>(&wh), cksum, numbytes_wh);
	bh.checksum = -cksum;

	outfile.write((char*)&bh, sizeof(bh));
	outfile.write((char*)&wh, numbytes_wh);
	outfile.write((char*)target.get(), sizeof(double) * trdatapoints);
	outfile.write(note.data(), note.size());
}

// Warning: this ist not recognize as a valid record by Igor!
void WriteIgorPlatformRecord(std::ostream& outfile)
{
	PackedFileRecordHeader pfrh{kPlatformRecord,0,sizeof(PlatformInfo)};
	PlatformInfo pli{ 2, 2, {0}, {0} };
	//double igorVersion = 5.00;
	//std::memcpy(pli.igorVersion, &igorVersion, sizeof(double));
	outfile.write((char*)&pfrh, sizeof(PackedFileRecordHeader));
	outfile.write((char*)&pli, sizeof(PlatformInfo));
}

void WriteIgorProcedureRecord(std::ostream& outfile)
{
	using namespace std::literals::string_view_literals;
	constexpr auto history_txt = "// pxp file created by PMbrowser\r"
		"// Use \"Macros\" --> \"Display Waves\" to create graphs\r"sv;
	PackedFileRecordHeader pfhr{ kHistoryRecord,0, int32_t(history_txt.size()) };
	outfile.write((char*)&pfhr, sizeof(PackedFileRecordHeader));
	outfile.write(history_txt.data(), history_txt.size());
	pfhr = { kProcedureRecord,0, int32_t(Igor_ipf.size()) };
	outfile.write((char*)&pfhr, sizeof(PackedFileRecordHeader));
	outfile.write(Igor_ipf.data(), Igor_ipf.size());
	pfhr = { kGetHistoryRecord,0,0 };
	outfile.write((char*)&pfhr, sizeof(PackedFileRecordHeader));
}

void ExportAllTraces(std::istream& datafile, DatFile& datf, const std::string& path, const std::string& prefix)
{
	int groupcount = 0;
	for (auto& group : datf.GetPulTree().GetRootNode().Children) {
		++groupcount;
		int seriescount = 0;
		for (auto& series : group.Children) {
			++seriescount;
			int sweepcount = 0;
			for (auto& sweep : series.Children) {
				++sweepcount;
				int tracecount = 0;
				for (auto& trace : sweep.Children) {
					++tracecount;
					std::stringstream wavename;
					wavename << prefix << "_" << groupcount << "_" << seriescount << "_" << sweepcount << "_";
					wavename << formTraceName(trace, tracecount);
					std::string filename = path + wavename.str() + ".ibw";
					std::ofstream outfile(filename, std::ios::binary | std::ios::out);
					ExportTrace(datafile, trace, outfile, wavename.str());
				}
			}
		}
	}
}

