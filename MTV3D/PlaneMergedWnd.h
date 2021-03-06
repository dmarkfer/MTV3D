/*********************************************************************************
**********************************************************************************
***
*** MTV3D - Mesh Tally Visualization in 3D
*** Copyright (C) 2019  Domagoj Markota <domagoj.markota@gmail.com>
***
*** This program is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Affero General Public License as published
*** by the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** This program is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*** GNU Affero General Public License for more details.
***
*** You should have received a copy of the GNU Affero General Public License
*** along with this program.  If not, see <https://www.gnu.org/licenses/>.
***
**********************************************************************************
**********************************************************************************/
#pragma once

#include "VisMergedWndBase.h"


class PlaneMergedWnd: public VisMergedWndBase {
private:
	char axis;
	HWND hPlaneTitle;
	HWND elevationScalingTypeGroupWnd;
	HWND elevationScalingTypeGroup;
	HWND radioButtonLin;
	HWND radioButtonLog;
	HWND hAxisBtnReplacementVal;
	HWND hBtnResetPlaneModel;
	HWND hBtnCreateLine;
public:
	PlaneMergedWnd(HINSTANCE hInst, LPWSTR windowTitle);
	~PlaneMergedWnd() = default;

	void setAxis(char axis);
	char getAxis();
	void setPlaneTitle(HWND hWnd);
	HWND getPlaneTitle();
	HWND getRadioButtonLin();
	HWND getRadioButtonLog();
	void setAxisBtnReplacementVal(HWND hWnd);
	HWND getAxisBtnReplacementVal();
	HWND getBtnResetPlaneModel();
	HWND getBtnCreateLine();
	virtual void resize() override;
};
