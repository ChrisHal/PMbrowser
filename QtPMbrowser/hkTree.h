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

#pragma once

#include <istream>
#include <ostream>
#include <vector>
#include <string>
#include <string_view>
#include <algorithm>
#include <memory>
#include <limits>
#include <type_traits>
#include <cstring>
#include <cstdint>
#include "helpers.h"

constexpr uint32_t MagicNumber = 0x54726565, SwappedMagicNumber = 0x65657254;

struct UserParamDescr {
    enum {
        SizeName = 32,
        SizeUnit = 8,
        Size = 40
    };
    std::string_view Name, Unit;
};
std::ostream& operator<<(std::ostream& os, const UserParamDescr&);


/// <summary>
/// A node in the tree (pul., pgf, amp, etc. tree)
/// </summary>
struct hkTreeNode {
private:
    template<typename T> T extractValueNoCheck(std::size_t offset) const
    {
        static_assert(std::is_arithmetic_v<T>, "must be arithmetic type");
        T t;
        auto src = Data.get() + offset;
        if (!isSwapped) {
            std::copy(src, src + sizeof t, reinterpret_cast<char*>(&t));
        }
        else {
            std::reverse_copy(src, src + sizeof t, reinterpret_cast<char*>(&t));
        }
        return t;
    }
public:
    hkTreeNode() : Parent{ nullptr }, isSwapped{ false }, Data{ nullptr }, level{ -1 }, len{ 0 }, Children{} {};

    /// <summary>
    /// extract a value from record data, swaps bytes if needed
    /// throws out_of_range exception
    /// if offset is too large
    /// </summary>
    /// <typeparam name="T">type of value to be extracted</typeparam>
    /// <param name="offset">offset in record data (needn't be aligned)</param>
    /// <returns>extracted value</returns>
    template<typename T> T extractValue(std::size_t offset) const
    {
        if (len < offset + sizeof(T)) {
            throw std::out_of_range("offset to large while accessing tree node");
        }
        return extractValueNoCheck<T>(offset);
    }

    /// <summary>
    /// extract a value from record data, swaps bytes if needed,
    /// return default value
    /// if offset is too large
    /// </summary>
    /// <typeparam name="T">type of value to be extracted</typeparam>
    /// <param name="offset">offset in record data (needn't be aligned)</param>
    /// <param name="defaultValue">default to be returned if offset is too large</param>
    /// <returns>extracted value or default value</returns>
    template<typename T> T extractValue(std::size_t offset, T defaultValue) const
    {
        if (len < offset + sizeof(T)) {
            return defaultValue;
        }
        return extractValueNoCheck<T>(offset);
    }
    enum TreeLevel {
        LevelRoot = 0,
        LevelGroup = 1,
        LevelSeries = 2,
        LevelSweep = 3,
        LevelTrace = 4
    };
    int32_t extractInt32(std::size_t offset) const { return extractValue<int32_t>(offset); };
    uint16_t extractUInt16(std::size_t offset) const { return extractValue<uint16_t>(offset); };
    double extractLongReal(std::size_t offset) const { return extractValue<double>(offset); };
    double extractLongRealNoThrow(std::size_t offset) const //! instead of throwing an exception, returns NaN if out of range
    {
        return extractValue(offset, std::numeric_limits<double>::quiet_NaN());
    };
    char getChar(std::size_t offset) const;
    const std::string_view getString(std::size_t offset) const;
    const UserParamDescr getUserParamDescr(std::size_t offset) const;
    template<std::size_t N> const std::string_view getString(std::size_t offset) const
    {
        if (len < offset + N) {
            throw std::out_of_range("offset to large while accessing tree node");
        }
        const auto *p = Data.get() + offset;
        if (p[N - 1]) {
            // in theory, string is not zero terminated
            // unfortunately, some PM version mess this up
            // by not prperly zero-initializing the cahr array
            return std::string_view(p, std::min(std::strlen(p), N));
        }
        else {
            return std::string_view(p);
        }
    };
    hkTreeNode* getParent() const { return Parent; };
    bool getIsSwapped() const { return isSwapped; };
    int getLevel() const { return level; };

    hkTreeNode* Parent;
    bool isSwapped;
    std::unique_ptr<char[]> Data;
    int level;
    std::size_t len; //!< Length (in bytes) of data
    std::vector<hkTreeNode> Children;
    friend class hkTree;
};

class hkTree
{
    std::vector<int32_t> LevelSizes;
    hkTreeNode RootNode;
    bool isSwapped;
    void LoadToNode(hkTreeNode* parent, hkTreeNode& node, char** pdata, int level);
public:
    hkTree() : LevelSizes{}, RootNode{}, isSwapped{ false } {};

    /// <summary>
    /// Initialize tree from istream
    /// </summary>
    /// <param name="infile">input stream (usually a filestream)</param>
    /// <param name="offset">offset of start of tree in infile stream</param>
    /// <param name="len">length in bytes of tree data in file (this data contains the total of the tree)</param>
    /// <returns>true on success</returns>
    bool InitFromStream(std::istream& infile, int offset, int len);

    /// <summary>
    /// Initialize tree from data buffered in memory
    /// </summary>
    /// <param name="buffer">pointer to data stored in memory</param>
    /// <param name="len">length in bytes of tree data in file (this data contains the totoal of the tree)</param>
    /// <returns>true on success</returns>
    bool InitFromBuffer(char* buffer, std::size_t len);
    hkTreeNode& GetRootNode() { return RootNode; };
    std::size_t GetNumLevels() { return LevelSizes.size(); };    //!< return number of levels this tree has
    bool getIsSwapped() { return isSwapped; };
};

