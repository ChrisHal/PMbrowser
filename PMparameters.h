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
#include <array>
#include <string>
#include <sstream>
#include "hkTree.h"


struct PMparameter {
	enum data_types {
		Byte,
		Int16,
		UInt16,
		Int32,
		UInt32,
		//Real32,
		LongReal, // i.e. double
		DateTime, // weird PowerMod date
		StringType,
		Boolean,
		InvLongReal, // invert double found in file, useful if e.g. conductance is given but we want resistance
		LongReal4, // array of 4 doubles
		LongReal8,	// array of 8 double
		LongReal16,  // 8 double
		RecordingMode,
		RootRelativeTime
	};
	bool exportIBW, print;
	const char* const name;
	const char* const unit;
	const data_types data_type;
	const size_t offset;

	void format(const hkTreeNode& node, std::string& s) const;
	void format(const hkTreeNode& node, std::stringstream& ss) const;
private:
	double getRootTime(const hkTreeNode& node) const;
};

extern std::array<PMparameter, 30>parametersTrace;
extern std::array<PMparameter, 18>parametersSweep;
extern std::array<PMparameter, 11>parametersSeries;
extern std::array<PMparameter, 5>parametersGroup;
extern std::array<PMparameter, 8>parametersRoot;

extern const std::array<const char*, 7>RecordingModeNames;

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
	const std::array<PMparameter, Nrows>& ar, std::stringstream& ss)
{
	for (const PMparameter& p : ar) {
		if (p.exportIBW) {
			p.format(n, ss);
			ss << "\n";
		}
	}
}

template<std::size_t Nrows> void formatParamListExportIBW(const hkTreeNode& n,
	const std::array<PMparameter, Nrows>& ar, std::string& str)
{
	std::stringstream ss;
	formatParamListExportIBW(n, ar, ss);
	str.append(ss.str());
}
