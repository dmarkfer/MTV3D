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


class VisMergedWndBase {
protected:
	HWND hWnd;
	HWND hResultDisplay;
	HWND hRelErrDisplay;
	HWND hResultLegend;
	HWND hRelErrLegend;
	HWND hCheckGrid;
	HWND hCheckAxesVals;

	HWND hPlanePreviewSelection;
	HWND hOrthAxisGroup;
	HWND radioButtonX;
	HWND radioButtonY;
	HWND radioButtonZ;
	HWND hEnterAxisVal;
	HWND hAxisValueBoxContainer;
	HWND hAxisValueBox;

	RECT wndRect;
	int displayDim;
	int dialogWidth;
	int dialogHeight;
public:
	VisMergedWndBase(HINSTANCE hInst, LPWSTR windowTitle, char wClass);
	virtual ~VisMergedWndBase() = default;

	HWND getHandle();
	HWND getResultDisplay();
	HWND getRelErrDisplay();
	HWND getResultLegend();
	HWND getRelErrLegend();
	HWND getRadioButtonX();
	HWND getRadioButtonY();
	HWND getRadioButtonZ();
	HWND getAxisValueBox();
	RECT getWndRect();
	int getDisplayDim();
	int getDialogHeight();
};
