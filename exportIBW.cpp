// (c) 2020 Christian R. Halaszovich
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

void ExportTrace(std::istream& datafile, hkTreeNode& TrRecord, const std::string& exportname)
{
#ifdef _DEBUG	
	std::cerr << "exporting '" << exportname << "'" << std::endl;
#endif
	char dataformat = TrRecord.getChar(TrDataFormat);
	if (dataformat != DFT_int16) {
		throw std::runtime_error("can't export data that is not int16");
	}
	int32_t interleavesize;
	try {
		interleavesize = TrRecord.extractInt32(TrInterleaveSize);
	}
	catch (std::out_of_range& e) { // fileformat too old to have interleave entry
		interleavesize = 0;
	}
	assert(interleavesize == 0);
	uint16_t tracekind = TrRecord.extractUInt16(TrDataKind);
	bool need_swap = !(tracekind & LittleEndianBit);

	std::string xunit, yunit;
	yunit = TrRecord.getString(TrYUnit); // assuming the string is zero terminated...
	xunit = TrRecord.getString(TrXUnit);
	double x0 = TrRecord.extractLongReal(TrXStart), deltax = TrRecord.extractLongReal(TrXInterval);
	double datascaler = TrRecord.extractLongReal(TrDataScaler);
	int32_t trdata = TrRecord.extractInt32(TrData), trdatapoints = TrRecord.extractInt32(TrDataPoints);
	int16_t* source = new int16_t[trdatapoints];
	double* target = new double[trdatapoints];
	datafile.seekg(trdata);
	datafile.read((char*)source, sizeof(int16_t) * trdatapoints);
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

	std::string filename = exportname + ".ibw";
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
	exportname.copy(wh.bname, MAX_WAVE_NAME5);
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

void ExportAllTraces(std::istream& datafile, DatFile& datf, const std::string& prefix)
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
					std::stringstream name;
					name << prefix << "_" << groupcount << "_" << seriescount << "_" << sweepcount;
					if (tracekind & IsImon) {
						name << "_Imon";
					}
					else if (tracekind & IsVmon) {
						name << "_Vmon";
					}
					else if (tracekind & IsLeak) {
						name << "_Leak";
					}
					else {
						name << "_" << tracecount;
					}
					ExportTrace(datafile, trace, name.str());
				}
			}
		}
	}
}
