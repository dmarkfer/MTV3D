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
	char wClass;

	HWND hWnd;
	HWND hResultDisplay;
	HWND hRelErrDisplay;
	HWND hDataWnd;
	HWND hResultLegend;
	HWND hRelErrLegend;
	HWND hResultTitle;
	HWND hRelErrTitle;
	HWND hResultLegendVal[7];
	HWND hRelErrLegendVal[7];

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
	RECT dataWndRect;
	int displayDim;
	int dialogWidth;
	int dialogHeight;
public:
	VisMergedWndBase(HINSTANCE hInst, LPWSTR windowTitle, char wClass);
	virtual ~VisMergedWndBase() = default;

	void setResultTitle(HWND hWnd);
	void setRelErrTitle(HWND hWnd);
	void setResultLegendVal(unsigned index, HWND hWnd);
	void setRelErrLegendVal(unsigned index, HWND hWnd);

	char getWClass();
	HWND getHandle();
	HWND getResultDisplay();
	HWND getRelErrDisplay();
	HWND getDataWnd();
	HWND getResultLegend();
	HWND getRelErrLegend();
	HWND getResultTitle();
	HWND getRelErrTitle();
	HWND getResultLegendVal(unsigned index);
	HWND getRelErrLegendVal(unsigned index);
	HWND getRadioButtonX();
	HWND getRadioButtonY();
	HWND getRadioButtonZ();
	HWND getAxisValueBox();
	RECT getWndRect();
	RECT getDataWndRect();
	int getDisplayDim();
	int getDialogHeight();
	virtual void resize();
};
