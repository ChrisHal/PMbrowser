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
#include <vector>
#include <string>
#include <string_view>
#include <memory>
#include <limits>
#include <cstring>
#include <cstdint>
#include "helpers.h"

constexpr uint32_t MagicNumber = 0x054726565, SwappedMagicNumber = 0x65657254;

/// <summary>
/// A node in the tree (pul., pgf, amp, etc. tree)
/// </summary>
struct hkTreeNode {
private:
    template<typename T> T extractValueNoCheck(std::size_t offset) const
    {
        T t{};
        std::memcpy(&t, Data.get() + offset, sizeof(T));
        if (isSwapped) {
            return swap_bytes(t);
        }
        else {
            return t;
        }
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
    template<std::size_t N> const std::string_view getString(std::size_t offset) const
    {
        if (len < offset + N) {
            throw std::out_of_range("offset to large while accessing tree node");
        }
        return std::string_view(Data.get() + offset, N);
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
    bool InitFromBuffer(char* buffer, size_t len);
    hkTreeNode& GetRootNode() { return RootNode; };
    size_t GetNumLevels() { return LevelSizes.size(); };    //!< return number of levels this tree has
    bool getIsSwapped() { return isSwapped; };
};

