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

// Routines to export traces as numpy arrays (*.npy)

#include <istream>
#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <sstream>
#include <stdexcept>
#include <memory>
#include <cstring>
#include <cassert>
#include <vector>
#include <cstddef>
#include "helpers.h"
#include "hkTree.h"
#include "DatFile.h"
#include "PMparameters.h"
#include "exportNPY.h"

namespace hkLib {

    constexpr std::string_view NPY_MAGIC{ "\x93NUMPY" };
    constexpr char NPY_VERSION_MAJOR{ '\x01' };
    constexpr char NPY_VERSION_MINOR{ '\x00' };
    constexpr std::size_t NPY_OFFSET_HEADER_LEN{ 8 }; // file offset to HEADER_LEN field
    constexpr std::size_t NPY_ALIGN{ 64 };

    static void fixNpyHeader(std::ostream& os)
    {
        std::size_t offset = os.tellp();
        ++offset; // leave room for '\n'
        auto remainder = offset % NPY_ALIGN;
        if (remainder != 0) {
            // pad to multiple of NPY_ALIGN
            os << std::string(NPY_ALIGN - remainder, ' ');
        }
        os << '\n';
        uint16_t header_len = static_cast<uint16_t>(static_cast<int>(os.tellp()) - 10);
        os.seekp(NPY_OFFSET_HEADER_LEN);
        os.write(reinterpret_cast<const char*>(&header_len), sizeof(uint16_t));
        os.seekp(0, std::ios::end);
    }

    static std::ostream& writeNpy(std::ostream& os, const std::vector<double>& v, bool raw_only)
    {
        if (!raw_only) {
            os << NPY_MAGIC << NPY_VERSION_MAJOR << NPY_VERSION_MINOR << '\0' << '\0' <<
                "{'descr': 'f8', 'fortran_order': False, 'shape': (" << v.size() << ",), }";
            // NOTE: 'f8': double in native byte order, '<f8': double in little endian byte order
            fixNpyHeader(os);
        }
        os.write(reinterpret_cast<const char*>(v.data()), v.size() * sizeof(double));
        return os;
    }

	static std::ostream& writeNpyArray(std::ostream& os, const std::vector<std::vector<double> >& v)
	{
        auto n_points = v.front().size();
        auto n_traces = v.size();
        os << NPY_MAGIC << NPY_VERSION_MAJOR << NPY_VERSION_MINOR << '\0' << '\0' <<
			"{'descr': 'f8', 'fortran_order': False, 'shape': (" << n_traces << ", "
			<< n_points << "), }";
		// NOTE: 'f8': double in native byte order, '<f8': double in little endian byte order
        fixNpyHeader(os);

        for (auto& v2 : v) {
            assert(v2.size() == n_points);
            os.write(reinterpret_cast<const char*>(v2.data()), n_points * sizeof(double));
        }
		return os;
	}

    static std::vector<double> read_data(std::istream& datafile, const hkTreeNode& TrRecord)
    {
        assert(TrRecord.getLevel() == hkTreeNode::LevelTrace);
        auto dataformat = TrRecord.getChar(TrDataFormat);
        auto trdatapoints = TrRecord.extractValue<uint32_t>(TrDataPoints);
        std::vector<double> tr_data(trdatapoints);
        switch (dataformat)
        {
        case DFT_int16:
            ReadScaleAndConvert<int16_t>(datafile, TrRecord, tr_data.size(), tr_data.data());
            break;
        case DFT_int32:
            ReadScaleAndConvert<int32_t>(datafile, TrRecord, tr_data.size(), tr_data.data());
            break;
        case DFT_float:
            ReadScaleAndConvert<float>(datafile, TrRecord, tr_data.size(), tr_data.data());
            break;
        case DFT_double:
            ReadScaleAndConvert<double>(datafile, TrRecord, tr_data.size(), tr_data.data());
            break;
        default:
            throw std::runtime_error{ "unknown data format type" };
            break;
        }

        return tr_data;
    }

    void NPYorBINExportTrace(std::istream& datafile, hkTreeNode& TrRecord, std::filesystem::path filename, bool createJSON = true)
    {
        assert(TrRecord.getLevel() == hkTreeNode::LevelTrace);
        auto yunit = TrRecord.getString(TrYUnit);
        auto xunit = TrRecord.getString(TrXUnit);
        auto x0 = TrRecord.extractLongReal(TrXStart);
        auto deltax = TrRecord.extractLongReal(TrXInterval);

        auto tr_data = read_data(datafile, TrRecord);
        
        std::ofstream outfile(filename, std::ios::binary | std::ios::out);
        if (!outfile)
        {
            throw std::runtime_error{ "could not create file" };
        }
        bool binexport = filename.extension() != ".npy";
        if (!writeNpy(outfile, tr_data, binexport))
        {
            throw std::runtime_error{ "error while writing npy file" };
        }

        if (createJSON) {
            filename.replace_extension("json");
            std::ofstream jsonfile(filename);
            if (!jsonfile) {
                throw std::runtime_error{ "could not create JSON file" };
            }
            jsonfile << std::scientific << "{ \"x_0\": " << x0 << ", \"delta_x\": " << deltax
                << ", \"numpnts\": " << tr_data.size() << ", \"unit_x\": \"" << xunit <<
                "\", \"unit_y\": \"" << yunit << "\", \"params\": { " <<
                "\"trace\": " << std::defaultfloat;
            formatParamListExportJSON(TrRecord, parametersTrace, jsonfile);
            jsonfile << ", \"sweep\": ";
            const auto sweep = TrRecord.getParent();
            formatParamListExportJSON(*sweep, parametersSweep, jsonfile);
            jsonfile << ", \"series\": ";
            const auto series = sweep->getParent();
            formatParamListExportJSON(*series, parametersSeries, jsonfile);
            jsonfile << ", \"group\": ";
            const auto group = series->getParent();
            formatParamListExportJSON(*group, parametersGroup, jsonfile);
            jsonfile << ", \"root\": ";
            const auto root = group->getParent();
            formatParamListExportJSON(*root, parametersRoot, jsonfile);
            jsonfile << " } }";
            if (!jsonfile) {
                throw std::runtime_error{ "error while writing JSON file" };
            }
        }
    }

    static int GetTraceID(const hkTreeNode& n)
    {
        return n.extractInt32(TrTraceID);
    }

    void NPYExportTreeSweepsAsArray(std::istream& datafile, const hkTreeView& tree, const std::string_view& path,
        const std::string_view& prefix, bool createJSON)
    {
        (void)createJSON;
        auto series_list = tree.GetViewListForLevel(hkTreeNode::LevelSeries);
        for (const auto* series : series_list) {
            // we need one array per Series and TraceID
            // containing all sweeps
            // get range of trace IDs
            int seriesID = series->p_node->extractInt32(SeSeriesCount);
            int groupID = series->p_node->getParent()->extractInt32(GrGroupCount);
            auto ID_min = std::numeric_limits<int>::max();
            auto ID_max = std::numeric_limits<int>::min();
            for (const auto& sweep : series->children) {
                for (const auto& trace : sweep.children) {
                    auto ID = GetTraceID(*trace.p_node);
                    ID_min = std::min(ID_min, ID);
                    ID_max = std::max(ID_max, ID);
                }
            }
            for (auto i = ID_min; i <= ID_max; ++i) {
                // gather traces
                std::vector<const hkTreeNode*> traces;
                for (const auto& sweep : series->children) {
                    for (const auto& trace : sweep.children) {
                        auto ID = GetTraceID(*trace.p_node);
                        if (ID == i) {
                            traces.push_back(trace.p_node);
                        }
                    }
                }
                std::vector<std::vector<double> > data;
                data.reserve(traces.size());
                for (const auto* p_trace : traces) {
                    data.emplace_back(read_data(datafile, *p_trace));
                }
                const auto& trace1 = *traces.front();
                std::string filename{ path };
                filename += prefix;
                filename += "_" + std::to_string(groupID) + "_" + std::to_string(seriesID) +
                    "_" + formTraceName(trace1, i) + ".npy";
                std::ofstream outfile(filename, std::ios::binary | std::ios::out);
                if (!outfile)
                {
                    throw std::runtime_error{ "could not create file " + filename };
                }
                writeNpyArray(outfile, data);
                if (createJSON) {
                    std::filesystem::path filepath(filename);
                    filepath.replace_extension("json");
                    std::ofstream jsonfile(filepath);
                    if (!jsonfile) {
                        throw std::runtime_error{ "could not create JSON file" };
                    }
                    auto yunit = trace1.getString(TrYUnit);
                    auto xunit = trace1.getString(TrXUnit);
                    auto x0 = trace1.extractLongReal(TrXStart);
                    auto deltax = trace1.extractLongReal(TrXInterval);
                    jsonfile << std::scientific << "{\n\"x_0\": " << x0 << ",\n\"delta_x\": " << deltax
                        << ",\n\"numpnts\": " << data.front().size() << ",\n\"unit_x\": \"" << xunit <<
                        "\",\n\"unit_y\": \"" << yunit << "\","
                        << std::defaultfloat;
                    jsonfile << "\n\"series\": ";
                    formatParamListExportJSON(*series->p_node, parametersSeries, jsonfile);
                    jsonfile << ",\n\"group\": ";
                    const auto group = series->p_node->getParent();
                    formatParamListExportJSON(*group, parametersGroup, jsonfile);
                    jsonfile << ",\n\"root\": ";
                    const auto root = group->getParent();
                    formatParamListExportJSON(*root, parametersRoot, jsonfile);
                    jsonfile << ",\n\"sweeps\": [\n";
                    bool is_first = true;
                    for (auto trace : traces) {
                        auto sweep = trace->getParent();
                        if (is_first) {
                            is_first = false;
                        }
                        else {
                            jsonfile << ",\n";
                        }
                        jsonfile << "{\"trace\": ";
                        formatParamListExportJSON(*trace, parametersTrace, jsonfile);
                        jsonfile << ",\n\"sweep\": ";
                        formatParamListExportJSON(*sweep, parametersSweep, jsonfile);
                        jsonfile << "\n}";
                    }
                    jsonfile << "]\n";
                    /*
                    formatParamListExportJSON(trace1, parametersTrace, jsonfile);
                    jsonfile << ", \"sweep\": ";
                    formatParamListExportJSON(*sweep, parametersSweep, jsonfile);

                    */
                    jsonfile << "\n}";
                    if (!jsonfile) {
                        throw std::runtime_error{ "error while writing JSON file" };
                    }
                }
            }
        } // end series loop
    }

    void NPYExportAllTraces(std::istream& datafile, DatFile& datf, const std::string& path, const std::string& prefix)
    {
        auto& root = datf.GetPulTree().GetRootNode();
        int groupcount = 0;
        for (auto& group : root.Children) {
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
                        std::stringstream wavename;
                        wavename << prefix << "_" << groupcount << "_" << seriescount << "_" << sweepcount << "_";
                        wavename << formTraceName(trace, tracecount);
                        std::string filename = path + wavename.str() + ".npy";
                        NPYorBINExportTrace(datafile, trace, filename, true);
                    }
                }
            }
        }
    }

}
