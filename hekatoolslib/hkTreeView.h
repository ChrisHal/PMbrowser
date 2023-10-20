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

#pragma once

#ifndef HKTREEVIEW_H
#define HKTREEVIEW_H

#include "hkTree.h"

namespace hkLib {
    struct hkNodeView {
        const hkTreeNode* p_node{ nullptr };
        std::vector<hkNodeView> children{};
    };

    struct hkTreeView {
        hkNodeView root{};
        /// <summary>
        /// Get views to all nodes with specified level
        /// </summary>
        /// <param name="level">desired level</param>
        /// <returns></returns>
        std::vector<const hkNodeView*> GetViewListForLevel(int level);
        std::vector<const hkTreeNode*> GetNodeListForLevel(int level);
    };
}

#endif // !HKTREEVIEW_H
