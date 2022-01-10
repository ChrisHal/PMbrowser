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

#include <iostream>
#include <cstdint>
#include <cassert>
#include <cstring>
#include "DatFile.h"
#include "hkTree.h"
#include "helpers.h"

/// <summary>
/// header of tree as stored in file
/// </summary>
struct TreeRoot {
	uint32_t Magic, //!< magic nummber MagicNumber or SwappedMagicNumber
		nLevels,	//!< number of tree levels
		LevelSizes[1];	//!< variable length array (nLevels entries with size of level data in bytes)
};

void hkTree::LoadToNode(hkTreeNode* parent, hkTreeNode& node, char** pdata, int level)
{
	auto size = LevelSizes.at(level);
	node.level = level;
	node.len = size;
	node.isSwapped = isSwapped;
	node.Data = std::make_unique<char[]>(size);
	node.Parent = parent;
	std::memcpy(node.Data.get(), *pdata, size);
	*pdata += size;
	uint32_t nchildren;
	std::memcpy(&nchildren, *pdata, sizeof(uint32_t));
	if (isSwapped) { swapInPlace(nchildren); }
	*pdata += sizeof(uint32_t);
	node.Children.resize(nchildren);
	for (auto& child : node.Children) {
		LoadToNode(&node, child, pdata, level + 1);
	}
}

bool hkTree::InitFromStream(std::istream& infile, int offset, int len)
{
	assert(!!infile);
	auto buffer = std::make_unique<char[]>(len);
	infile.seekg(offset).read(buffer.get(), len);
	if (!infile) {
		infile.clear();
		return false;
	}
	bool res = this->InitFromBuffer(buffer.get(), len);
	return res;
}

bool hkTree::InitFromBuffer(char* buffer, size_t len)
{
    TreeRoot* root = reinterpret_cast<TreeRoot*>(buffer);
	isSwapped = false;
	if (root->Magic == SwappedMagicNumber) {
		isSwapped = true;
	} else if (root->Magic != MagicNumber) {
		throw std::runtime_error("magic number does not match, wrong filetype?");
	}
	LevelSizes.clear();
	if (isSwapped) {
		swapInPlace(root->nLevels);
	}
	for (unsigned i = 0; i < root->nLevels; ++i) {
		if (isSwapped) { swapInPlace(root->LevelSizes[i]); }
		LevelSizes.push_back(root->LevelSizes[i]);
	}
	char* data = buffer + sizeof(uint32_t) * (2ull + root->nLevels); // start of first tree node
	LoadToNode(nullptr, RootNode, &data, 0);
	if (data - buffer != len) {
		throw std::runtime_error("bytes read != bytes in buffer");
	}
	return true;
}

char hkTreeNode::getChar(size_t offset) const
{
	if (len < offset + sizeof(char)) {
		throw std::out_of_range("offset to large while accessing tree node");
	}
	return Data[offset];
}

const std::string_view hkTreeNode::getString(size_t offset) const
{
	if (len <= offset) {
		throw std::out_of_range("offset to large while accessing tree node");
	}
	return std::string_view(Data.get() + offset);
}
