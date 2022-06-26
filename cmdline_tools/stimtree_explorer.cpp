/*
    Copyright 2022 Christian R. Halaszovich

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

#include<iostream>
#include<filesystem>
#include<fstream>
#include"DatFile.h"
#include "StimTree.h"
#include"PMparameters.h"


void do_exploring(const hkTreeNode& root, int index, int ch) {
    //std::array<double, 10> params{};
    //char paramNames[10][32]{};
    for (size_t i = 0; i < 10; ++i) {
        std::cout << "p" << (i + 1) << " " << root.extractLongReal(i * 8 + 48)
            << ", p-name: " << root.getString(128 + i * 32) << "\n";
    }
    std::cout << "\nNum stim entries : " << root.Children.size() << '\n';
    const auto& stim_node = root.Children.at(index);
    StimulationRecord stim{ stim_node };
    std::cout << "entry name: " << stim.EntryName << ", file name: " << stim_node.getString(stFileName)
        << "\nstartSegment: " << stim.DataStartSegment
        << ", start time: " << stim.DataStartTime << '\n'
        << "num ch: " << stim.Channels.size() << ", actual DAC channels: " << stim.ActualDacChannels << '\n';
    const auto& ch_node = stim.Channels.at(ch);
    std::cout << "ch# (from 0):" << ch //<< "\ndac ch: " << ch_node.DacChannel
        << " mode: " << ch_node.DacMode << '\n' << "Linked: " << ch_node.LinkedChannel
        //<< "adc ch: " << ch_node.extractValue<int16_t>(chAdcChannel)
        //<< " mode: " << static_cast<int>(ch_node.getChar(chAdcMode)) << '\n'
        << "\n#segments: " << ch_node.Segments.size() << "\nexploring:\n";
    int count{};
    for (const auto& segment : ch_node.Segments) {
        std::cout << "\nsegment " << ++count << "\n";
        formatParamListExportIBW(*segment.Node, parametersStimSegment, std::cout);

    }
    std::cout << "\nContructing trace:\n";
    auto pts = stim.constructStimTrace(0);

    for (const auto& p : pts) {
        std::cout << p.at(0) << '\t' << p.at(1) << '\n';
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " <filename>.dat [<stim_index>] [<ch_index>]\n";
        return EXIT_FAILURE;
    }
    int stim_index{};
    if (argc > 2) {
        stim_index = std::strtol(argv[2], nullptr, 10);
    }
    int ch_index{};
    if (argc > 3) {
        ch_index = std::strtol(argv[3], nullptr, 10);
    }
    std::filesystem::path inpath(argv[1]);

    std::ifstream infile(inpath, std::ios::binary);
    if (!infile) {
        std::cerr << "error opening file " << argv[1] << '\n';
        return EXIT_FAILURE;
    }
    if (inpath.extension() == ".pgf") {
        std::cout << "pfg file detected\n";
        try {
            hkTree stimtree{};
            stimtree.InitFromStream(".pgf", infile, 0, static_cast<int>(std::filesystem::file_size(inpath)));
            do_exploring(stimtree.GetRootNode(), stim_index, ch_index);
        }
        catch (const std::exception& e) {
            std::cerr << "error " << e.what() << " processing file " << inpath << '\n';
            return EXIT_FAILURE;
        }
    }
    else {
        DatFile df{};
        try {
            df.InitFromStream(infile);
            auto& stimtree = df.GetPgfTree();
            do_exploring(stimtree.GetRootNode(), stim_index, ch_index);
        }
        catch (const std::exception& e) {
            std::cerr << "error " << e.what() << " processing file " << inpath << '\n';
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}