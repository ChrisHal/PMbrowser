/*
    Copyright 2023 - 2024 Christian R. Halaszovich

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

// small tool to test npy file export

#include <iostream>
#include <fstream>
#include <string>
#include "DatFile.h"
#include "exportNPY.h"

using namespace hkLib;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " <filename>.dat [<prefix>]\n";
        return EXIT_FAILURE;
    }

    std::string prefix{ "PM" };
    if (argc > 2) {
        prefix = argv[2];
    }
    std::ifstream infile(argv[1], std::ios::binary);
    if (!infile) {
        std::cerr << "error opening file " << argv[1] << '\n';
        return EXIT_FAILURE;
    }
    DatFile df;
    try {
        df.InitFromStream(infile);
        NPYExportAllTraces(infile, df, "./", prefix);
    }
    catch (const std::exception& e) {
        std::cerr << "error " << e.what() << " processing file " << argv[1] << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
