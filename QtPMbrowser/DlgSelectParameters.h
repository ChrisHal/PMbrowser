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

#pragma once

#include <QDialog>
#include <QCheckBox>
#include <QLabel>
#include <QPalette>
#include <QGridLayout>
#include <QScrollArea>
#include <vector>
#include "PMparameters.h"
#include "PMparametersModel.h"

namespace Ui { class DlgSelectParameters; }

class DlgSelectParameters : public QDialog
{
	Q_OBJECT

public:
	DlgSelectParameters(QWidget *parent = Q_NULLPTR);
	~DlgSelectParameters();;

private:

	PMparametersModel m_root{ hkLib::parametersRoot },
		m_grp{ hkLib::parametersGroup },
		m_ser{ hkLib::parametersSeries },
		m_swp{ hkLib::parametersSweep },
		m_tr{ hkLib::parametersTrace },
		m_amp{ hkLib::parametersAmpplifierState },
		m_stim_stim{ hkLib::parametersStimulation },
		m_stim_ch{ hkLib::parametersChannel },
		m_stim_seg{ hkLib::parametersStimSegment };

	Ui::DlgSelectParameters *ui;
};
