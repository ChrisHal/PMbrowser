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
#ifndef MACHINEINFO_H
#define MACHINEINFO_H

#pragma once

#include <bit>

namespace hkLib {

    /// <summary>
    /// check endianess of machine
    /// (will be optimized to a constant by compiler)
    /// </summary>
    /// <returns>true if machine is little-endian</returns>
    consteval bool MachineIsLittleEndian()
    {
        return std::endian::native == std::endian::little;
    }
}
#endif // !MACHINEINFO_H
