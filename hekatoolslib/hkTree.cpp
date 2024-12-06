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
#include <cstddef>
#include <cassert>
#include <cstring>
#include "DatFile.h"
#include "hkTree.h"
#include "helpers.h"

namespace hkLib {

	void hkTreeNode::setAsTime0() {
		assert(tree->getID() == ExtPul);
		switch (level) {
		case LevelRoot:
			tree->time0 = extractValue(RoStartTime, 0.0);
			break;
		case LevelGroup:
			// resort to root time
			tree->time0 = Parent->extractValue(RoStartTime, 0.0);
			break;
		case LevelSeries:
			tree->time0 = extractValue(SeTime, 0.0);
			break;
		case LevelSweep:
			tree->time0 = extractValue(SwTime, 0.0);
			break;
		case LevelTrace:
			// resort to sweep time
			tree->time0 = Parent->extractValue(SwTime, 0.0);
			break;
		default:
			throw std::runtime_error("unexpected tree level");
		}
	}

	double hkTreeNode::getTime0() const
	{
		return tree->time0;
	}

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
		node.tree = this;
		auto size = static_cast<std::size_t>(LevelSizes.at(level));
		node.level = level;
		//node.len = size;
		node.isSwapped = isSwapped;
		node.Parent = parent;
		node.Data = std::span( *pdata, size );
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

	bool hkTree::InitFromStream(const std::string_view& id, std::istream& infile, int offset, unsigned int len)
	{
		assert(!!infile);
		if (offset <= 0 || len == 0) throw std::runtime_error("invalid tree data offset or length");
		Data = std::make_unique<char[]>(len);
		infile.seekg(offset).read(Data.get(), len);
		if (!infile) {
			infile.clear();
			return false;
		}
		bool res = this->InitFromBuffer(id, Data.get(), len);
		return res;
	}

	bool hkTree::InitFromBuffer(const std::string_view& id, char* buffer, std::size_t len)
	{
		ID = id;
		TreeRoot* root = reinterpret_cast<TreeRoot*>(buffer); // we assume buffer is correctly aligned
		isSwapped = false;
		if (root->Magic == SwappedMagicNumber) {
			isSwapped = true;
		}
		else if (root->Magic != MagicNumber) {
			throw std::runtime_error("magic number does not match, wrong filetype?");
		}
		LevelSizes.clear();
		if (isSwapped) {
			swapInPlace(root->nLevels);
		}
		for (std::size_t i = 0; i < root->nLevels; ++i) {
			if (isSwapped) { swapInPlace(root->LevelSizes[i]); }
			LevelSizes.push_back(root->LevelSizes[i]);
		}
		char* data = buffer + offsetof(TreeRoot, LevelSizes) + sizeof(uint32_t) * root->nLevels; // start of first tree node
		LoadToNode(nullptr, RootNode, &data, 0);
		if (data - buffer != static_cast<std::ptrdiff_t>(len)) {
			throw std::runtime_error("bytes read != bytes in buffer");
		}
		return true;
	}

	bool hkTree::isValid()
	{
		return LevelSizes.size()!=0 && !RootNode.Data.empty();
	}

	char hkTreeNode::getChar(std::size_t offset) const
	{
		if (Data.size() < offset + sizeof(char)) {
			throw std::out_of_range("offset too large while accessing tree node");
		}
		return Data[offset];
	}

	const std::optional<UserParamDescr> hkTreeNode::getUserParamDescr(std::size_t offset) const
	{
		if (Data.size() < offset + UserParamDescr::Size) {
			//throw std::out_of_range("offset too large while accessing tree node");
			return std::nullopt;
		}
		else {
			return { UserParamDescr{
				getString<UserParamDescr::SizeName>(offset),
				getString<UserParamDescr::SizeUnit>(offset + UserParamDescr::SizeName) }
			};
		}
	}

	const std::string_view hkTreeNode::getString(std::size_t offset) const
	{
		if (Data.size() <= offset) {
			throw std::out_of_range("offset too large while accessing tree node");
		}
		return std::string_view(Data.data() + offset);
	}

	std::ostream& operator<<(std::ostream& os, const UserParamDescr& p)
	{
		os << '(' << p.Name << ',' << p.Unit << ')';
		return os;
	}

}
