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

// Routines to export traces as Igor Binary Waves (ibw)

#define _CRT_SECURE_NO_WARNINGS // get rid of some unnecessary warnings
#include <istream>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <cassert>
#include "helpers.h"
#include "hkTree.h"
#include "DatFile.h"
#include "exportIBW.h"
#include "Igor_IBW.h"

//	This is how Igor wants its checksum done
int Checksum(short* data, int oldcksum, int numbytes)
{
	numbytes >>= 1;				// 2 bytes to a short -- ignore trailing odd byte.
	while (numbytes-- > 0)
		oldcksum += *data++;
	return oldcksum & 0xffff;
}

void MakeWaveNote(hkTreeNode& TrRecord, std::string& notetxt)
{
	double cslow = TrRecord.extractLongReal(TrCSlow),
		gseries = TrRecord.extractLongReal(TrGSeries),
		rsvalue = TrRecord.extractLongReal(TrRsValue),
		sealres = TrRecord.extractLongReal(TrSealResistance);
	std::stringstream note;
	note << "Cslow=" << cslow
		<< " F\nRseries=" << (1.0 / gseries)
		<< " Ohm\nRsValue=" << rsvalue 
		<< "\nSealResistance=" << sealres << " Ohm\n";
	notetxt = note.str();
}

template<typename T> void ReadScaleAndConvert(std::istream& datafile, bool need_swap, size_t trdatapoints, double datascaler,
	double* target, int interleavesize, int interleaveskip)
{
	T* source = new T[trdatapoints];
	if (interleavesize == 0) {
		datafile.read((char*)source, sizeof(T) * trdatapoints);
	}
	else { // it's interleaved data
		assert(interleaveskip >= interleavesize);
		size_t bytesremaining = sizeof(T) * trdatapoints;
		int bytestoskip = interleaveskip - interleavesize; // interleaveskip is from block-start to block-start!
		char* p = (char*)source;
		while (bytesremaining > 0) {
			size_t bytestoread = std::min(bytesremaining, size_t(interleavesize));
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
		for (int i = 0; i < trdatapoints; ++i) {
			target[i] = datascaler * source[i];
		}
	}
	else {
		for (int i = 0; i < trdatapoints; ++i) {
			target[i] = datascaler * swap_bytes(source[i]);
		}
	}
	delete[] source; source = nullptr;
}

void ExportTrace(std::istream& datafile, hkTreeNode& TrRecord, const std::string& filename, const std::string& wavename)
{
	char dataformat = TrRecord.getChar(TrDataFormat);
	int32_t     interleavesize = TrRecord.extractValue<int32_t>(TrInterleaveSize, 0),
		interleaveskip = TrRecord.extractValue<int32_t>(TrInterleaveSkip, 0);
	uint16_t tracekind = TrRecord.extractUInt16(TrDataKind);
	bool need_swap = !(tracekind & LittleEndianBit);

	std::string xunit, yunit;
	yunit = TrRecord.getString(TrYUnit); // assuming the string is zero terminated...
	xunit = TrRecord.getString(TrXUnit);
	double x0 = TrRecord.extractLongReal(TrXStart), deltax = TrRecord.extractLongReal(TrXInterval);
	double datascaler = TrRecord.extractLongReal(TrDataScaler);
	int32_t trdata = TrRecord.extractInt32(TrData), trdatapoints = TrRecord.extractInt32(TrDataPoints);

	double* target = new double[trdatapoints];
	datafile.seekg(trdata);

	switch (dataformat)
	{
	case DFT_int16:
		ReadScaleAndConvert<int16_t>(datafile, need_swap, trdatapoints, datascaler, target, interleavesize, interleaveskip);
		break;
	case DFT_int32:
		ReadScaleAndConvert<int32_t>(datafile, need_swap, trdatapoints, datascaler, target, interleavesize, interleaveskip);
		break;
	case DFT_float:
		ReadScaleAndConvert<float>(datafile, need_swap, trdatapoints, datascaler, target, interleavesize, interleaveskip);
		break;
	case DFT_double:
		ReadScaleAndConvert<double>(datafile, need_swap, trdatapoints, datascaler, target, interleavesize, interleaveskip);
		break;
	default:
		throw std::runtime_error("unknown data format type");
		break;
	}

	std::ofstream outfile(filename, std::ios::out | std::ios::binary);
	if (!outfile) {
		std::stringstream msg;
		msg << "error opening file '" << filename << "' for writing: " << strerror(errno);
		throw std::runtime_error(msg.str());
	}

	std::string note;
	MakeWaveNote(TrRecord, note);

	BinHeader5 bh;
	// make sure the packing of the structs is as expected:
	static_assert(sizeof(bh) == 64, "wrong size of bh");
	memset((void*)&bh, 0, sizeof(bh));
	WaveHeader5 wh;
	constexpr size_t numbytes_wh = offsetof(WaveHeader5, wData);
	static_assert(numbytes_wh == 320, "wrong size of wh");
	memset((void*)&wh, 0, sizeof(wh));

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

	short cksum = Checksum((short*)&bh, 0, sizeof(bh));
	cksum = Checksum((short*)&wh, cksum, numbytes_wh);
	bh.checksum = -cksum;

	outfile.write((char*)&bh, sizeof(bh));
	outfile.write((char*)&wh, numbytes_wh);
	outfile.write((char*)target, sizeof(double) * trdatapoints);
	outfile.write(note.data(), note.size());
	outfile.close();
	delete[] target;
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
					uint16_t tracekind = trace.extractUInt16(TrDataKind);
					std::stringstream wavename;
					wavename << prefix << "_" << groupcount << "_" << seriescount << "_" << sweepcount;
					if (tracekind & IsImon) {
						wavename << "_Imon";
					}
					else if (tracekind & IsVmon) {
						wavename << "_Vmon";
					}
					else if (tracekind & IsLeak) {
						wavename << "_Leak";
					}
					else {
						wavename << "_" << tracecount;
					}
					std::string filename = path + wavename.str() + ".ibw";
					ExportTrace(datafile, trace, filename, wavename.str());
				}
			}
		}
	}
}
