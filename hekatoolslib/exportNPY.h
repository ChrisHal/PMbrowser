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

#pragma once

#ifndef EXPORT_NPY_H
#define EXPORT_NPY_H

#include <istream>
#include <filesystem>
#include "hkTree.h"

void NPYExportTrace(std::istream& datafile, hkTreeNode& TrRecord, std::filesystem::path filename, bool createJSON);

/// <summary>
/// Export all trace in NPY format
/// mainly for testing purposes
/// </summary>
/// <param name="datafile">heka data stream</param>
/// <param name="datf">datafile object</param>
/// <param name="path">path in which exporteed files will be saved</param>
/// <param name="prefix">tracename prefix (selected by user)</param>
void NPYExportAllTraces(std::istream& datafile, DatFile& datf, const std::string& path, const std::string& prefix);


#endif // !EXPORT_NPY_H
