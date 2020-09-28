#pragma once
#include <istream>
#include <vector>
#include <string>
#include <cstdint>
#include "helpers.h"

constexpr int32_t MagicNumber = 0x054726565, SwappedMagicNumber = 0x65657254;
struct hkTreeNode {
public:
    hkTreeNode() : Parent{ nullptr }, isSwapped { false }, Data{ nullptr }, len{ 0 }, Children{} {};
    int32_t extractInt32(size_t offset);
    uint16_t extractUInt16(size_t offset);
    double extractLongReal(size_t offset);
    double extractLongRealNoThrow(size_t offset); // instead of throwing an exception, returns NaN if out of range
    char getChar(size_t offset);
    std::string getString(size_t offset);
    hkTreeNode* getParent() { return Parent; };
    bool getIsSwapped() { return isSwapped; };
private:
    hkTreeNode* Parent;
    bool isSwapped;
    char* Data;
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

