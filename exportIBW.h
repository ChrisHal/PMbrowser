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

#pragma once
struct PackedFileRecordHeader {
    uint16_t recordType; 	/* Record type plus superceded flag. */
    int16_t version;				/* Version information depends on the type of record. */
    int32_t numDataBytes;			/* Number of data bytes in the record following this record header. */
};
static_assert(sizeof(PackedFileRecordHeader) == 8, "struct PackedFileRecordHeader has wrong size");

constexpr unsigned short
PACKEDRECTYPE_MASK = 0x7FFF,
SUPERCEDED_MASK = 0x8000; // on load: ignore pxp record if superceded bit is set

// pxp record type IDs
constexpr auto kWaveRecord = 3,	//  3: Contains the data for a wave
kDataFolderStartRecord = 9,		// datafollder, followed by 32 chars, zero-terminated
kDataFolderEndRecord = 10;

void ExportAllTraces(std::istream& datafile, DatFile& datf, const std::string& path, const std::string& prefix);
void ExportTrace(std::istream& datafile, hkTreeNode& TrRecord, std::ostream& outfile, const std::string& wavename);