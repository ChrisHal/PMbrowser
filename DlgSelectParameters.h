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

#include <QDialog>
#include <QCheckBox>
#include <QLabel>
#include "PMparameters.h"
#include "ui_DlgSelectParameters.h"

namespace Ui { class DlgSelectParameters; };

class DlgSelectParameters : public QDialog
{
	Q_OBJECT

public:
	DlgSelectParameters(QWidget *parent = Q_NULLPTR);
	~DlgSelectParameters();
	void storeParams();

private:
	template<std::size_t Nrows> void populateGrid(QGridLayout* grid,
		const std::array<PMparameter, Nrows>& ar)
	{
		for (int i = 0; i < int(ar.size()); ++i) {
			auto chk1 = new QCheckBox("export");
			chk1->setChecked(ar[i].exportIBW);
			//chk1->setFixedSize(50, 20);
			auto chk2 = new QCheckBox("print");
			chk2->setChecked(ar[i].print);
			//chk2->setFixedSize(50, 20);
			auto lb = new QLabel(ar[i].name);
			//lb->setFixedSize(100, 20);
			grid->addWidget(chk1, i, 0, Qt::AlignLeft | Qt::AlignVCenter);
			grid->addWidget(chk2, i, 1, Qt::AlignLeft | Qt::AlignVCenter);
			grid->addWidget(lb, i, 2, Qt::AlignLeft | Qt::AlignVCenter);
			//grid->setRowStretch(i,0);
			//grid->setRowMinimumHeight(i, 40);
		}
		grid->setColumnStretch(0, 1);
		grid->setColumnStretch(1, 1);
		grid->setColumnStretch(2, 2);
	};
	template<std::size_t Nrows> void readFromGrid(QGridLayout* grid,
		std::array<PMparameter, Nrows>& ar)
	{
		for (int i = 0; i < int(ar.size()); ++i) {
			ar[i].exportIBW = (dynamic_cast<QCheckBox*>(grid->itemAtPosition(i, 0)->widget()))->isChecked();
			ar[i].print = (dynamic_cast<QCheckBox*>(grid->itemAtPosition(i, 1)->widget()))->isChecked();
		}
	};

	Ui::DlgSelectParameters *ui;
};
