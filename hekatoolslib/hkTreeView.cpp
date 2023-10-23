/*
    Copyright 2023 Christian R. Halaszovich

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

#include "hkTreeView.h"

using hkLib::hkNodeView;
using hkLib::hkTreeNode;

static void collect_views(const hkNodeView& node, std::vector<const hkNodeView*>& res, int level)
{
    auto l = node.p_node->getLevel();
    if (l > level) return;
    if (l < level) {
        for (auto& child : node.children) {
            collect_views(child, res, level);
        }
    }
    else {
        res.push_back(&node);
    }
}


static void collect_nodes(const hkNodeView& root, std::vector<const hkTreeNode*>& res, int level)
{
    std::vector<const hkNodeView*> tmp;
    collect_views(root, tmp, level);
    for (const auto& e : tmp) {
        res.push_back(e->p_node);
    }
}


std::vector<const hkNodeView*> hkLib::hkTreeView::GetViewListForLevel(int level) const
{
    std::vector<const hkNodeView*> res;
    collect_views(root, res, level);
    return res;
}

std::vector<const hkTreeNode*> hkLib::hkTreeView::GetNodeListForLevel(int level) const
{
    std::vector<const hkTreeNode*> res;
    collect_nodes(root, res, level);
    return res;
}

