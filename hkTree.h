/*
    Copyright 2020 Christian R. Halaszovich

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
#include <cstring>
#include <cstdint>
#include "helpers.h"

constexpr int32_t MagicNumber = 0x054726565, SwappedMagicNumber = 0x65657254;
struct hkTreeNode {
public:
    hkTreeNode() : Parent{ nullptr }, isSwapped{ false }, Data{ nullptr }, level{ -1 }, len{ 0 }, Children{} {};
    template<typename T> T extractValue(size_t offset)
    {
        if (len < offset + sizeof(T)) {
            throw std::out_of_range("offset to large while accessing tree node");
        }
        T t;
        std::memcpy(&t, Data + offset, sizeof(T));
        if (isSwapped) {
            return swap_bytes(t);
        }
        else {
            return t;
        }
    }
    int32_t extractInt32(size_t offset) { return extractValue<int32_t>(offset); };
    uint16_t extractUInt16(size_t offset) { return extractValue<uint16_t>(offset); };
    double extractLongReal(size_t offset) { return extractValue<double>(offset); };
    double extractLongRealNoThrow(size_t offset); // instead of throwing an exception, returns NaN if out of range
    char getChar(size_t offset);
    std::string getString(size_t offset);
    hkTreeNode* getParent() { return Parent; };
    bool getIsSwapped() { return isSwapped; };
    int getLevel() { return level; };
private:
    hkTreeNode* Parent;
    bool isSwapped;
    char* Data;
    int level;
    int32_t len;
public:
    std::vector<hkTreeNode> Children;
    friend class hkTree;
};

class hkTree
{
    std::vector<int32_t> LevelSizes;
    hkTreeNode RootNode;
    bool isSwapped;
    void LoadToNode(hkTreeNode* parent, hkTreeNode& node, char** pdata, int level);
    void FreeNodeMemory(hkTreeNode& node);
public:
    hkTree() : LevelSizes{}, RootNode{}, isSwapped{ false } {};
    bool InitFromStream(std::istream& infile, int offset, int len);
    bool InitFromBuffer(char* buffer, size_t len);
    hkTreeNode& GetNode(const std::vector<int>& nodeid); // nodeid contains "coordinates" of desired node, cannot access root node!
    hkTreeNode& GetRootNode() { return RootNode; };
    size_t GetNumLevels() { return LevelSizes.size(); };
    bool getIsSwapped() { return isSwapped; };
    ~hkTree();
};

