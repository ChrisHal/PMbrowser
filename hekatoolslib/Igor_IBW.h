/*
	Copyright 2020 - 2023 Christian R. Halaszovich

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

// file headers for WaveMetrics(tm) IgorPro ibw format

#ifndef IGOR_IBW_H
#define IGOR_IBW_H

#pragma once
#include <cstdint>

namespace hkLib {
	using std::int16_t;
	using std::int32_t;
	using std::uint32_t;

	constexpr auto NT_I32 = 0x20;		// 32 bit integer numbers.
	constexpr auto NT_FP64 = 4;
	constexpr auto NT_UNSIGNED = 0x40;	// Makes above signed integers unsigned.

	constexpr auto MAXDIMS = 4;
	constexpr auto MAX_WAVE_NAME5 = 31;	// Maximum length of wave name in version 5 files. Does not include the trailing null.;
	constexpr auto MAX_UNIT_CHARS = 3;

	// Mac'c (deprecated) GetDateTime returnes seconde starting from 1.1.1904,
	// Unix's epoch is from 1.1.1970. This is in seconds.
	constexpr auto EPOCHDIFF_MAC_UNIX = 2082844800;

	struct BinHeader5 {
		int16_t version;						// Version number for backwards compatibility.
		int16_t checksum;						// Checksum over this header and the wave header.
		int32_t wfmSize;					// The size of the WaveHeader5 data structure plus the wave data.
		int32_t formulaSize;				// The size of the dependency formula, including the null terminator, if any. Zero if no dependency formula.
		int32_t noteSize;					// The size of the note text.
		int32_t dataEUnitsSize;				// The size of optional extended data units.
		int32_t dimEUnitsSize[MAXDIMS];		// The size of optional extended dimension units.
		int32_t dimLabelsSize[MAXDIMS];		// The size of optional dimension labels.
		int32_t sIndicesSize;				// The size of string indicies if this is a text wave.
		int16_t longWaveNameSize;				// The size of the long name in version 7 files.
		int16_t optionsSize1;					// Reserved. Write zero. Ignore on read.
		int32_t optionsSize2;				// Reserved. Write zero. Ignore on read.
	};

	// compared to origianl, some pointer types have been converted to uint32_t for compilation for 64-bit
	struct WaveHeader5 {
		uint32_t pad0;

		uint32_t creationDate;				// DateTime of creation.
		uint32_t modDate;					// DateTime of last modification.

		int32_t npnts;						// Total number of points (multiply dimensions up to first zero).
		int16_t type;							// See types (e.g. NT_FP64) above. Zero for text waves.
		int16_t pad1;

		char pad2[6];
		int16_t whVersion;					// Write 1. Ignore on read.
		char bname[MAX_WAVE_NAME5 + 1];		// Name of wave plus trailing null.
		char pad3[4];
		uint32_t dFolder;		// Used in memory only. Write zero. Ignore on read.

		// Dimensioning info. [0] == rows, [1] == cols etc
		int32_t nDim[MAXDIMS];				// Number of of items in a dimension -- 0 means no data.
		// CAUTION: The next element is NOT 8 byte aligned! This is why we need the ugly pack(4)
		// or we work around it! I prefer to do so...
		//double sfA[MAXDIMS];				// Index value for element e of dimension d = sfA[d]*e + sfB[d].
		//double sfB[MAXDIMS];
		char sfA[MAXDIMS][sizeof(double)];
		char sfB[MAXDIMS][sizeof(double)];

		// SI units
		char dataUnits[MAX_UNIT_CHARS + 1];			// Natural data units go here - null if none.
		char dimUnits[MAXDIMS][MAX_UNIT_CHARS + 1];	// Natural dimension units go here - null if none.

		int16_t fsValid;						// TRUE if full scale values have meaning.
		char pad4[2];
		// The next 2 are out of aligment,too:
		//double topFullScale, botFullScale;	// The max and max full scale value for wave.
		// tweak aligment:
		char topFullScale[sizeof(double)], botFullScale[sizeof(double)];
		char pad5[40];
		unsigned char platform;				// 0=unspecified, 1=Macintosh, 2=Windows; Added for Igor Pro 5.5.
		unsigned char pad6[91];
		float wData[1];						// The start of the array of data. Must be 64 bit aligned.
	};
}
#endif // !IGOR_IBW_H
