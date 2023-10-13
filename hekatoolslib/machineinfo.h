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
namespace hkLib {

    /// <summary>
    /// check endianess of machine
    /// (will be optimized to a constant by compiler)
    /// </summary>
    /// <returns>true if machine is little-endian</returns>
    inline bool MachineIsLittleEndian()
    {
        unsigned t = 1;
        auto p = reinterpret_cast<char*>(&t);
        return *p == 1;
    }
}
#endif // !MACHINEINFO_H
