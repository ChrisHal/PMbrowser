#include <iostream>
#include <cstdint>
#include <cassert>
#include <cstring>
#include <limits>
#include "DatFile.h"
#include "hkTree.h"
#include "helpers.h"

struct TreeRoot {
	int32_t Magic, nLevels, LevelSizes[1];
};

void hkTree::LoadToNode(hkTreeNode* parent, hkTreeNode& node, char** pdata, int level)
{
	auto size = LevelSizes.at(level);
	node.len = size;
	node.isSwapped = isSwapped;
	node.Data = new char[size];
	node.Parent = parent;
	std::memcpy(node.Data, *pdata, size);
	*pdata += size;
	// cave: can we be certain that *pdata is 4 byte aligned?
	int32_t nchildren = *reinterpret_cast<int32_t*>(*pdata);
	if (isSwapped) { swapInPlace(nchildren); }
	*pdata += sizeof(int32_t);
	// std::cout << "level " << level << "\tnchildren " << nchildren << std::endl;
	node.Children.resize(nchildren);
	for (auto& child : node.Children) {
		LoadToNode(&node, child, pdata, level + 1);
	}
}

void hkTree::FreeNodeMemory(hkTreeNode& node)
{
	delete[] node.Data;
	node.Data = nullptr;
	for (auto& child : node.Children) {
		FreeNodeMemory(child);
	}
}

bool hkTree::InitFromStream(std::istream& infile, int offset, int len)
{
	assert(!!infile);
	auto buffer = new char[len];
	infile.seekg(offset).read(buffer, len);
	if (!infile) {
		infile.clear();
		return false;
	}
	bool res = this->InitFromBuffer(buffer, len);
	delete[] buffer;
	return res;
}

bool hkTree::InitFromBuffer(char* buffer, size_t len)
{
	TreeRoot* root = reinterpret_cast<TreeRoot*>(buffer);
	isSwapped = false;
	if (root->Magic == SwappedMagicNumber) {
		isSwapped = true;
#ifdef _DEBUG
		std::cerr << "INFO: tree is swapped" << std::endl;
#endif
		//throw std::runtime_error("tree file has unsuported byte order");
	} else if (root->Magic != MagicNumber) {

		throw std::runtime_error("magic number does not match, wrong filetype?");
	}
	LevelSizes.clear();
	if (isSwapped) {
		swapInPlace(root->nLevels);
	}
	for (int i = 0; i < root->nLevels; ++i) {
		if (isSwapped) { swapInPlace(root->LevelSizes[i]); }
		LevelSizes.push_back(root->LevelSizes[i]);
#ifdef _DEBUG
		std::cerr << "level " << i << " size: " << root->LevelSizes[i] << "\n";
#endif
	}
	char* data = buffer + sizeof(int32_t) * (2ll + root->nLevels); // start of first tree node
	LoadToNode(nullptr, RootNode, &data, 0);
	return true;
}

hkTreeNode& hkTree::GetNode(const std::vector<int>& nodeid)
{
	if (nodeid.empty()) {
		throw std::runtime_error("no nodeid provided");
	}
	auto n = &RootNode;
	for (int index : nodeid) {
		n = &n->Children.at(0);
	}
	return *n;
}

hkTree::~hkTree()
{
	if(RootNode.Data != nullptr)
		FreeNodeMemory(RootNode);
}

int32_t hkTreeNode::extractInt32(size_t offset)
{
	if (len < offset + sizeof(int32_t)) {
		throw std::out_of_range("offset to large while accessing tree node");
	}
	if (!isSwapped) {
		return ::extractInt32(Data, offset);
	}
	else {
		return swap_bytes(::extractInt32(Data, offset));
	}
}

uint16_t hkTreeNode::extractUInt16(size_t offset)
{
	if (len < offset + sizeof(uint16_t)) {
		throw std::out_of_range("offset to large while accessing tree node");
	}
	if (!isSwapped) {
	return ::extractUInt16(Data, offset);
	}
	else {
		return swap_bytes(::extractUInt16(Data, offset));
	}
}

double hkTreeNode::extractLongReal(size_t offset)
{
	if (len < offset + sizeof(double)) {
		throw std::out_of_range("offset to large while accessing tree node");
	}
	if (!isSwapped) {
		return ::extractLongReal(Data, offset);
	}
	else {
		return swap_bytes(::extractLongReal(Data, offset));
	}
}

double hkTreeNode::extractLongRealNoThrow(size_t offset)
{
	if (len < offset + sizeof(double)) {
		return std::numeric_limits<double>::quiet_NaN();
	}
	if (!isSwapped) {
		return ::extractLongReal(Data, offset);
	}
	else {
		return swap_bytes(::extractLongReal(Data, offset));
	}
}

char hkTreeNode::getChar(size_t offset)
{
	if (len < offset + sizeof(char)) {
		throw std::out_of_range("offset to large while accessing tree node");
	}
	return Data[offset];
}

std::string hkTreeNode::getString(size_t offset)
{
	if (len <= offset) {
		throw std::out_of_range("offset to large while accessing tree node");
	}
	return std::string(Data + offset);
}
