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

#ifndef PM_PARAMETERS_H
#define PM_PARAMETERS_H

#pragma once
#include <array>
#include <string>
#include <sstream>
#include <ostream>
#include "hkTree.h"

namespace hkLib {

	struct PMparameter {
		enum data_types {
			Byte,
			Int16,
			UInt16,
			Set16, // for bitfields
			Int32,
			UInt32,
			//Real32,
			LongReal, // i.e. double
			DateTime, // weird PowerMod date
			StringType,
			String8, // String of length 8
			String16,
			String32,
			String80,
			String400,
			Boolean,
			InvLongReal, // invert double found in file, useful if e.g. conductance is given but we want resistance
			LongReal2, // array of 2 doubles
			LongReal4, // array of 4 doubles
			LongReal8,	// array of 8 double
			LongReal16,  // 8 double
			RecordingMode,
			RelativeTime,
			AmpModeName,
			UserParamDesc4, // 4x UserParamDesc
			UserParamDesc2,
			UserParamDesc8
		};
		bool exportIBW, print;
		const char* const name;
		const char* const unit;
		const data_types data_type;
		const size_t offset;

		void format(const hkTreeNode& node, std::string& s) const;
		void format(const hkTreeNode& node, std::ostream& ss) const;
		void formatJSON(const hkTreeNode& node, std::ostream& ss) const;
		void formatValueOnly(const hkTreeNode& node, std::ostream& ss) const;

		/// <summary>
		/// encode flag state as int for saving in setting
		/// </summary>
		/// <returns>flags</returns>
		int toInt() const { return int(exportIBW) | (int(print) << 1); };

		/// <summary>
		/// decode flags from int (cf. toInt())
		/// </summary>
		/// <param name="i">flags</param>
		void fromInt(int i) { exportIBW = i & 1; print = i & 2; };
	private:
		double getRootTime(const hkTreeNode& node) const;
		template<std::size_t N>void formatUserParamDesc(const hkTreeNode& node, std::size_t offset, std::ostream& ss) const {
			ss << "(name,unit):[";
			for (std::size_t i = 0; i < N; ++i) {
				ss << node.getUserParamDescr(offset + i * UserParamDescr::Size) << ';';
			}
			ss << ']';
		}
	};

	extern std::array<PMparameter, 34>parametersTrace;
	extern std::array<PMparameter, 18>parametersSweep;
	extern std::array<PMparameter, 15>parametersSeries;
	extern std::array<PMparameter, 5>parametersGroup;
	extern std::array<PMparameter, 8>parametersRoot;

	extern std::array<PMparameter, 31>parametersAmpplifierState;

	extern const std::array<const char*, 7>RecordingModeNames;
	extern const std::array<const char*, 4> AmpModeNames;

	extern std::array<PMparameter, 14> parametersStimSegment;

	/// <summary>
	/// Format parameters stored in node n using
	/// all parameters defined in array ar
	/// </summary>
	/// <param name="n">node coontaining data</param>
	/// <param name="ar">array of parameters</param>
	/// <param name="str">string that will receive result</param>
	template<std::size_t Nrows> void formatParamList(const hkTreeNode& n,
		const std::array<PMparameter, Nrows>& ar, std::string& str)
	{
		std::stringstream ss;
		for (const PMparameter& p : ar) {
			p.format(n, ss);
			ss << "\n";
		}
		str = ss.str();
	}

	template<std::size_t Nrows> void formatParamListPrint(const hkTreeNode& n,
		const std::array<PMparameter, Nrows>& ar, std::string& str)
	{
		std::stringstream ss;
		for (const PMparameter& p : ar) {
			if (p.print) {
				p.format(n, ss);
				ss << "\n";
			}
		}
		str = ss.str();
	}

	template<std::size_t Nrows> void formatParamListExportIBW(const hkTreeNode& n,
		const std::array<PMparameter, Nrows>& ar, std::ostream& ss)
	{
		for (const PMparameter& p : ar) {
			if (p.exportIBW) {
				p.format(n, ss);
				ss << "\n";
			}
		}
	}

	template<std::size_t Nrows> void formatParamListExportJSON(const hkTreeNode& n,
		const std::array<PMparameter, Nrows>& ar, std::ostream& ss)
	{
		ss << "{ ";
		bool is_first{ true };
		for (const PMparameter& p : ar) {
			if (p.exportIBW) {
				if (is_first) {
					is_first = false;
				}
				else {
					ss << ", ";
				}
				p.formatJSON(n, ss);
			}
		}
		ss << " }";
	}

	template<std::size_t Nrows> void formatParamListExportIBW(const hkTreeNode& n,
		const std::array<PMparameter, Nrows>& ar, std::string& str)
	{
		std::stringstream ss;
		formatParamListExportIBW(n, ar, ss);
		str.append(ss.str());
	}

	/// <summary>
	/// format haders for format as tab-delimited list (no tab after last element)
	/// format is "name[unit]"
	/// </summary>
	/// <param name="ar">parameter array</param>
	/// <param name="ss">stream to receive output</param>
	template<std::size_t Nrows> std::ostream& getTableHeadersExport(
		const std::array<PMparameter, Nrows>& ar, std::ostream& ss)
	{
		//bool is_first{ true };
		for (const auto& p : ar) {
			if (p.exportIBW) {
				ss << '\t' << p.name;
				if (*p.unit) { ss << '[' << p.unit << ']'; }
			}
		}
		return ss;
	}

	/// <summary>
	/// format tab del. list for export as table
	/// </summary>
	/// <param name="n">node containg data to be exported</param>
	/// <param name="ar">definition of parameters</param>
	/// <param name="ss">stream to recevie output</param>
	template<std::size_t Nrows> std::ostream& formatParamListExportTable(const hkTreeNode& n,
		const std::array<PMparameter, Nrows>& ar, std::ostream& ss)
	{
		for (const auto& p : ar) {
			if (p.exportIBW) {
				ss << '\t';
				p.formatValueOnly(n, ss);
			}
		}
		return ss;
	}

	/// <summary>
	/// format tab del. list for export as table
	/// </summary>
	/// <param name="n">node containg data to be exported</param>
	/// <param name="ar">definition of parameters</param>
	/// <returns>string containing result</returns>
	template<std::size_t Nrows> std::string formatParamListExportTable(const hkTreeNode& n,
		const std::array<PMparameter, Nrows>& ar)
	{
		std::stringstream ss;
		formatParamListExportTable(n, ar, ss);
		return ss.str();
	}
}

#endif // !PM_PARAMETERS_H
