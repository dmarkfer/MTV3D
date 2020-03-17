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
#include "stdafx.h"
#include "PlaneMergedWnd.h"


PlaneMergedWnd::PlaneMergedWnd(HINSTANCE hInst, LPWSTR windowTitle): VisMergedWndBase(hInst, windowTitle, 'L') {
	this->hBtnCreateLine = CreateWindow(L"BUTTON", L"Create line preview",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		wndRect.right / 4 + dialogWidth * 2 / 3, this->displayDim + dialogHeight * 2 / 3, 180, 30,
		this->hWnd, (HMENU)BUTTON_CREATE_LINE, hInst, nullptr);

	this->elevationScalingTypeGroupWnd = CreateWindowEx(WS_EX_TOOLWINDOW, L"ElevScalTypeGrp", nullptr, WS_VISIBLE | WS_CHILD,
		50, this->displayDim + 80, 180, 50,
		this->hWnd, nullptr, hInst, nullptr);
	this->elevationScalingTypeGroup = CreateWindow(L"BUTTON", L" Elevation scaling ", WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
		0, 0, 180, 50,
		this->elevationScalingTypeGroupWnd, nullptr, hInst, nullptr);
	this->radioButtonLin = CreateWindow(L"BUTTON", L"Lin", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
		20, 20, 50, 20,
		this->elevationScalingTypeGroupWnd, nullptr, hInst, nullptr);
	this->radioButtonLog = CreateWindow(L"BUTTON", L"Log", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
		90, 20, 50, 20,
		this->elevationScalingTypeGroupWnd, nullptr, hInst, nullptr);

	SendMessage(radioButtonLin, BM_SETCHECK, BST_CHECKED, 0);
}


void PlaneMergedWnd::setAxis(char axis) {
	this->axis = axis;
}


char PlaneMergedWnd::getAxis() {
	return this->axis;
}


void PlaneMergedWnd::setPlaneTitle(HWND hWnd) {
	this->hPlaneTitle = hWnd;
}


HWND PlaneMergedWnd::getPlaneTitle() {
	return this->hPlaneTitle;
}


HWND PlaneMergedWnd::getRadioButtonLin() {
	return this->radioButtonLin;
}


HWND PlaneMergedWnd::getRadioButtonLog() {
	return this->radioButtonLog;
}


void PlaneMergedWnd::setAxisBtnReplacementVal(HWND hWnd) {
	this->hAxisBtnReplacementVal = hWnd;
}


HWND PlaneMergedWnd::getAxisBtnReplacementVal() {
	return this->hAxisBtnReplacementVal;
}


HWND PlaneMergedWnd::getBtnCreateLine() {
	return this->hBtnCreateLine;
}


void PlaneMergedWnd::resize() {
	this->VisMergedWndBase::resize();

	MoveWindow(hPlaneTitle, dataWndRect.right / 2 - 60, displayDim / 5 - 70, 150, 20, TRUE);

	MoveWindow(elevationScalingTypeGroupWnd, 50, this->displayDim + 80, 180, 50, TRUE);
	MoveWindow(elevationScalingTypeGroup, 0, 0, 180, 50, TRUE);
	MoveWindow(radioButtonLin, 20, 20, 50, 20, TRUE);
	MoveWindow(radioButtonLog, 90, 20, 50, 20, TRUE);

	if (this->axis == 'X') {
		MoveWindow(hAxisBtnReplacementVal, wndRect.right / 4 + 50, displayDim + dialogHeight / 3 + 30, 100, 20, TRUE);
	}
	else if (this->axis == 'Y') {
		MoveWindow(hAxisBtnReplacementVal, wndRect.right / 4 + 120, displayDim + dialogHeight / 3 + 30, 100, 20, TRUE);
	}
	else {
		MoveWindow(hAxisBtnReplacementVal, wndRect.right / 4 + 200, displayDim + dialogHeight / 3 + 30, 100, 20, TRUE);
	}
	
	MoveWindow(hBtnCreateLine, wndRect.right / 4 + dialogWidth * 2 / 3, this->displayDim + dialogHeight * 2 / 3, 180, 30, TRUE);
}
