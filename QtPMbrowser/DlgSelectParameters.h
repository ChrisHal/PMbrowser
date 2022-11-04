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
#include <QPalette>
#include <QGridLayout>
#include "PMparameters.h"
#include "ui_DlgSelectParameters.h"

namespace Ui { class DlgSelectParameters; }

class DlgSelectParameters : public QDialog
{
	Q_OBJECT

public:
	DlgSelectParameters(QWidget *parent = Q_NULLPTR);
	~DlgSelectParameters();
	void storeParams();

private:
	static constexpr int chkbox_width = 40;
	template<std::size_t Nrows> void populateGrid(QGridLayout* grid,
		const std::array<PMparameter, Nrows>& ar)
	{
		for (int i = 0; i < int(ar.size()); ++i) {
			auto chk1 = new QCheckBox();// ("export");
			chk1->setChecked(ar[i].exportIBW);
			chk1->setMinimumWidth(chkbox_width);
			chk1->setMaximumWidth(chkbox_width);
			auto chk2 = new QCheckBox();// ("print");
			chk2->setChecked(ar[i].print);
			chk2->setMinimumWidth(chkbox_width);
			chk2->setMaximumWidth(chkbox_width);
			auto lb = new QLabel(ar[i].name);
			grid->addWidget(chk1, i, 0);// , Qt::AlignLeft | Qt::AlignVCenter);
			grid->addWidget(chk2, i, 1);// , Qt::AlignLeft | Qt::AlignVCenter);
			grid->addWidget(lb, i, 2);// , Qt::AlignLeft | Qt::AlignVCenter);
		}
		grid->addItem(new QSpacerItem(0, 0), static_cast<int>(ar.size()), 0, 1, 3);
		grid->setRowStretch(static_cast<int>(ar.size()), 1);
		grid->setColumnStretch(0, 0);
		grid->setColumnStretch(1, 0);
		grid->setColumnStretch(2, 1);
		grid->setHorizontalSpacing(1);
	}

	template<std::size_t Nrows> void readFromGrid(QGridLayout* grid,
		std::array<PMparameter, Nrows>& ar)
	{
		for (int i = 0; i < static_cast<int>(ar.size()); ++i) {
			auto chkExport = qobject_cast<QCheckBox*>(grid->itemAtPosition(i, 0)->widget());
			assert(chkExport);
			ar[i].exportIBW = chkExport->isChecked();
			auto chkPrint = qobject_cast<QCheckBox*>(grid->itemAtPosition(i, 1)->widget());
			assert(chkPrint);
			ar[i].print = chkPrint->isChecked();
		}
	}

	QGridLayout *gridLayoutGrp{ new QGridLayout },
		*gridLayoutSer{ new QGridLayout },
		*gridLayoutSwp{ new QGridLayout },
		*gridLayoutTr{ new QGridLayout };

	Ui::DlgSelectParameters *ui;
};
