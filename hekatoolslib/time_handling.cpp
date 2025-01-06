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

#include <cassert>
#include <cmath>
#include <ctime>
#include <cstring>
#include <string>
#include "time_handling.h"

constexpr std::time_t EPOCHDIFF_MAC_UNIX = 2082844800;

// the following values are from file format documentation
// Strangly, the actual time difference in time_t seconds
// is 126144000, but PM seems to caculate with the given difference.
constexpr double JanFirst1990MACTime = 1580970496.0;
constexpr auto HIGH_DWORD = 4294967296.0; // = 2^32

namespace hkLib {
    /// <summary>
    /// Convert PatchMaster time to unix time_t
    /// This follows the documentation, but the result is not excact
    /// </summary>
    /// <param name="t">time as stored in .dat file</param>
    /// <returns>unix time_t</returns>
    static std::time_t PMtime2time_t(double t)
    {
        t -= JanFirst1990MACTime;
        if (t < 0.0) {
            t += HIGH_DWORD; // why is this necessary?
        }
        return static_cast<std::time_t>(std::floor(t)) - EPOCHDIFF_MAC_UNIX;
    }

    constexpr std::size_t BUFF_SIZE = 128;

    static std::string formatPMtime(double t, const char* fmt_str)
    {
        auto unixtime = PMtime2time_t(t);
        char buffer[BUFF_SIZE]{};
        auto mtm = gmtime(&unixtime);
        if (mtm) {
            auto count = std::strftime(buffer, BUFF_SIZE, fmt_str, mtm);
            assert(count != 0);
            return std::string(buffer, count);
        }
        else {
            return "<conversion error>";
        }
    }

    std::string formatPMtimeDate(double t)
    {
        return formatPMtime(t, "%F");
    }

    std::string formatPMtimeUTC(double t)
    {
        return formatPMtime(t, "%FT%T");
    }
}
