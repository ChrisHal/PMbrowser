/*
	Copyright 2020 - 2024 Christian R. Halaszovich

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
#include <vector>
#include "PMparameters.h"
#include "ui_DlgSelectParameters.h"

namespace Ui { class DlgSelectParameters; }

class DlgSelectParameters : public QDialog
{
	Q_OBJECT

		struct chk_row {
		QCheckBox do_export, do_print;
		QLabel label;
	};

public:
	DlgSelectParameters(QWidget *parent = Q_NULLPTR);
	~DlgSelectParameters();
	void accept() override;
	void storeParams();

private:
	static constexpr int chkbox_width = 40;
	template<std::size_t Nrows> QGridLayout* createGrid(std::vector<chk_row>& v,
		const std::array<hkLib::PMparameter, Nrows>& ar)
	{
		auto grid = new QGridLayout;
		//v.resize(Nrows);
		for (int i = 0; i < int(ar.size()); ++i) {
			auto& row = v.at(i);
			auto* chk1 = &row.do_export;
			chk1->setChecked(ar[i].exportIBW);
			chk1->setMinimumWidth(chkbox_width);
			chk1->setMaximumWidth(chkbox_width);
			auto* chk2 = &row.do_print;
			chk2->setChecked(ar[i].print);
			chk2->setMinimumWidth(chkbox_width);
			chk2->setMaximumWidth(chkbox_width);
			auto* lb = &row.label;
			lb->setText(ar[i].name);
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
		return grid;
	}

	template<std::size_t Nrows> void setScrollArea(QScrollArea* area,
		std::vector<chk_row>& v,
		const std::array<hkLib::PMparameter, Nrows>& ar) {
		auto w = new QWidget;
		w->setLayout(createGrid(v, ar));
		w->setBackgroundRole(QPalette::Base);
		area->setWidget(w);
	}

	template<std::size_t Nrows> void readSelections(const std::vector<chk_row>& v,
		std::array<hkLib::PMparameter, Nrows>& ar)
	{
		for (std::size_t i = 0; i < ar.size(); ++i) {
			ar.at(i).exportIBW = v.at(i).do_export.isChecked();
			ar.at(i).print = v.at(i).do_print.isChecked();
		}
	}

	std::vector<chk_row> v_root{ hkLib::parametersRoot.size() },
		v_grp{ hkLib::parametersGroup.size() },
		v_ser{ hkLib::parametersSeries.size() },
		v_swp{ hkLib::parametersSweep.size() },
		v_tr{ hkLib::parametersTrace.size() };

	Ui::DlgSelectParameters *ui;
};
