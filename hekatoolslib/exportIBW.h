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

#ifndef EXPORT_IBW_H
#define EXPORT_IBW_H

#pragma once

namespace hkLib {
    struct PackedFileRecordHeader {
        uint16_t recordType; 	/* Record type plus superceded flag. */
        int16_t version;				/* Version information depends on the type of record. */
        int32_t numDataBytes;			/* Number of data bytes in the record following this record header. */
    };
    static_assert(sizeof(PackedFileRecordHeader) == 8, "struct PackedFileRecordHeader has wrong size");

    constexpr unsigned short
        PACKEDRECTYPE_MASK = 0x7FFF,
        SUPERCEDED_MASK = 0x8000; // on load: ignore pxp record if superceded bit is set

    struct PlatformInfo {			// Data written for a record of type kPlatformRecord.
        int16_t platform;				// 0=unspecified, 1=Macintosh, 2=Windows.
        int16_t architecture;			// 0=invalid, 1=PowerPC, 2=Intel.
        char igorVersion[sizeof(double)];			// e.g., 5.00 for Igor Pro 5.00.
        char reserved[256 - 12];	// Reserved. Write as zero.
    };
    static_assert(sizeof(PlatformInfo) == 256, "PlatformInfo struct has wrong size");
    // pxp record type IDs
    constexpr auto
        kHistoryRecord = 2,
        kWaveRecord = 3,	//  3: Contains the data for a wave
        kRecreationRecord = 4,
        kProcedureRecord = 5,
        kGetHistoryRecord = 7,
        kDataFolderStartRecord = 9,		// datafollder, followed by 32 chars, zero-terminated
        kDataFolderEndRecord = 10,
        kPlatformRecord = 20;

    void WriteIgorPlatformRecord(std::ostream& outfile);
    void WriteIgorProcedureRecord(std::ostream& outfile);
    void ExportAllTraces(std::istream& datafile, DatFile& datf, const std::string& path, const std::string& prefix);
    void ExportTrace(std::istream& datafile, hkTreeNode& TrRecord, std::ostream& outfile, const std::string& wavename);

}

#endif // !EXPORT_IBW_H
