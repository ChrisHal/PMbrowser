/*
	Copyright 2020 - 2025 Christian R. Halaszovich

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

#include "ui_DlgSelectParameters.h"
#include "DlgSelectParameters.h"

DlgSelectParameters::DlgSelectParameters(QWidget *parent)
	: QDialog(parent)
{
	ui = new Ui::DlgSelectParameters();
	ui->setupUi(this);

	ui->tableViewRoot->setModel(&m_root);
	ui->tableViewGrp->setModel(&m_grp);
	ui->tableViewSer->setModel(&m_ser);
	ui->tableViewSwp->setModel(&m_swp);
	ui->tableViewTrace->setModel(&m_tr);
	ui->tableViewAmp->setModel(&m_amp);
	ui->tableViewStim->setModel(&m_stim_stim);
	ui->tableViewChannel->setModel(&m_stim_ch);
	ui->tableViewSegment->setModel(&m_stim_seg);

}

DlgSelectParameters::~DlgSelectParameters()
{
	delete ui;
}
