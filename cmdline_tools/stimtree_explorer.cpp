/*
    Copyright 2022-2025 Christian R. Halaszovich

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
#include<locale>
#include"DatFile.h"
#include "StimTree.h"
#include"PMparameters.h"

using namespace hkLib;

void do_exploring(const hkTreeNode& root, int index, int /*ch*/) {
    //formatParamListExportIBW(root,parametersStimRoot,std::cout);
    auto headerList = hkLib::getHeaderList(parametersStimRoot, false, false, true);
    auto pList = hkLib::getParamList(root, parametersStimRoot, false, false, true);
    std::cout << "root:\n";
    std::size_t N = headerList.size();
    for (std::size_t i = 0; i < N; ++i) {
        std::cout << headerList.at(i) << '\t' << pList.at(i) << '\n';
    }

    std::cout << "\nNum stim entries : " << root.Children.size() << '\n';
    const auto& stim_node = root.Children.at(index);
    headerList = hkLib::getHeaderList(parametersStimulation, false, false, true);
    pList = hkLib::getParamList(stim_node, parametersStimulation, false, false, true);
    std::cout << "\nstimulation #" << index << ":\n";
    N = headerList.size();
    for (std::size_t i = 0; i < N; ++i) {
        std::cout << headerList.at(i) << '\t' << pList.at(i) << '\n';
    }
    
    auto Nch = stim_node.Children.size();
    headerList= hkLib::getHeaderList(parametersChannel, false, false, true);
    std::vector<std::vector<std::string>> chData;
    chData.reserve(Nch);
    std::cout << "\nChannels";
    for (std::size_t i = 0; i < Nch; ++i) {
        std::cout << "\tChannel# " << i;
        chData.push_back(hkLib::getParamList(stim_node.Children.at(i), parametersChannel, false, false, true));
    }
    std::cout << '\n';
    N = headerList.size();
    for (std::size_t i = 0; i < N; ++i) {
        std::cout << headerList.at(i);
        for (std::size_t j = 0; j < Nch; ++j) {
            std::cout << '\t' << chData.at(j).at(i);
        }
        std::cout << '\n';
    }
    
    headerList = hkLib::getHeaderList(parametersStimSegment, false, false, true);
    N = headerList.size();
    bool more_segs = true;
    for(std::size_t seg_count = 0; more_segs; ++seg_count){
        more_segs = false;
        std::vector<std::vector<std::string>> seg_params(Nch);
        for(std::size_t ch_count{0}; ch_count < Nch; ++ch_count){
            const auto& channel_node = stim_node.Children.at(ch_count);
            if(seg_count < channel_node.Children.size()){
                more_segs = true;
                const auto& segment_node = channel_node.Children.at(seg_count);
                seg_params.at(ch_count) = hkLib::getParamList(segment_node, parametersStimSegment, false, false, true);
            }
        }
        if(!more_segs) break;
        //now print it:
        std::cout << "\nsegment# " << seg_count << ":\n";
        for(std::size_t i{0}; i < N; ++i) {
            std::cout << headerList.at(i);
            for(std::size_t ch_count{0}; ch_count < Nch; ++ch_count){
                std::cout << '\t';
                if(!seg_params.at(ch_count).empty()) {
                    std::cout << seg_params.at(ch_count).at(i);
                }
            }
            std::cout << '\n';
        }
    }
    
//    std::cout << "\nContructing trace:\n";
//    auto pts = stim.constructStimTrace(0);
//
//    for (const auto& p : pts) {
//        std::cout << p.at(0) << '\t' << p.at(1) << '\n';
//    }
}

int main(int argc, char** argv) {
    std::locale::global(std::locale(""));
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
