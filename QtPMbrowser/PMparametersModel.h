#pragma once

/*
	Copyright 2020 - 2026 Christian R. Halaszovich

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

#include <QAbstractTableModel>
#include <QSpan>
#include "PMparameters.h"

class PMparametersModel  : public QAbstractTableModel
{
	Q_OBJECT

private:
	static constexpr std::array<const char*, 2> lables{ "export", "print" };
	QSpan<hkLib::PMparameter> parameters;
	bool hide_export{ false };
	int countCheckedPrint() const;
	int countCheckedExport() const;

public:
	explicit PMparametersModel(QSpan<hkLib::PMparameter> PMparameters, QObject *parent = nullptr);
	~PMparametersModel();
	void hideExport(bool flag = true) { hide_export = flag; };
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;
};

