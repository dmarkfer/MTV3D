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
#include "VisMergedWndBase.h"


VisMergedWndBase::VisMergedWndBase(HINSTANCE hInst, LPWSTR windowTitle, char wClass): wClass(wClass) {
	SetCursor(LoadCursor(nullptr, IDC_ARROW));

	this->hWnd = CreateWindowW(wClass == 'P' ? L"VisMerged" : L"VisMergedPlane", windowTitle, WS_OVERLAPPEDWINDOW | WS_MAXIMIZE,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInst, nullptr);


	GetClientRect(this->hWnd, &wndRect);
	GetClientRect(this->hDataWnd, &dataWndRect);
	this->displayDim = int(0.8f * wndRect.right) / 2;

	this->hResultDisplay = CreateWindowEx(WS_EX_TOOLWINDOW, L"VisDisplay", nullptr, WS_VISIBLE | WS_CHILD,
		0, 0, this->displayDim, this->displayDim,
		this->hWnd, nullptr, hInst, nullptr);
	LONG displayStyle = GetWindowLong(this->hResultDisplay, GWL_STYLE);
	displayStyle &= ~(WS_BORDER | WS_CAPTION | WS_SYSMENU);
	SetWindowLong(this->hResultDisplay, GWL_STYLE, displayStyle);


	this->hDataWnd = CreateWindowEx(WS_EX_TOOLWINDOW, L"DataWndClass", nullptr, WS_VISIBLE | WS_CHILD,
		this->displayDim, 0, wndRect.right - 2 * this->displayDim, this->displayDim,
		this->hWnd, nullptr, hInst, nullptr);
	displayStyle = GetWindowLong(this->hDataWnd, GWL_STYLE);
	displayStyle &= ~(WS_BORDER | WS_CAPTION | WS_SYSMENU);
	SetWindowLong(this->hDataWnd, GWL_STYLE, displayStyle);


	this->hResultLegend = CreateWindowEx(WS_EX_TOOLWINDOW, L"VisLegend", nullptr, WS_VISIBLE | WS_CHILD,
		dataWndRect.right / 2 - int(0.05f * this->displayDim), this->displayDim / 5 + ((wClass == 'P') ? 0 : 20), this->displayDim / 25, this->displayDim / 2,
		this->hDataWnd, nullptr, hInst, nullptr);
	displayStyle = GetWindowLong(this->hResultLegend, GWL_STYLE);
	displayStyle &= ~(WS_BORDER | WS_CAPTION | WS_SYSMENU);
	SetWindowLong(this->hResultLegend, GWL_STYLE, displayStyle);

	this->hRelErrLegend = CreateWindowEx(WS_EX_TOOLWINDOW, L"VisLegend", nullptr, WS_VISIBLE | WS_CHILD,
		dataWndRect.right / 2 + int(0.01f * this->displayDim), this->displayDim / 5 + ((wClass == 'P') ? 0 : 20), this->displayDim / 25, this->displayDim / 2,
		this->hDataWnd, nullptr, hInst, nullptr);
	displayStyle = GetWindowLong(this->hRelErrLegend, GWL_STYLE);
	displayStyle &= ~(WS_BORDER | WS_CAPTION | WS_SYSMENU);
	SetWindowLong(this->hRelErrLegend, GWL_STYLE, displayStyle);

	this->hCheckGrid = CreateWindow(L"BUTTON", L"Grid", BS_AUTOCHECKBOX | WS_TABSTOP | WS_VISIBLE | WS_CHILD,
		50, this->displayDim + 20, 100, 20,
		this->hWnd, (HMENU)CHECK_GRID, hInst, nullptr);
	this->hCheckAxesVals = CreateWindow(L"BUTTON", L"Axes values", BS_AUTOCHECKBOX | WS_TABSTOP | WS_VISIBLE | WS_CHILD,
		50, this->displayDim + 50, 100, 20,
		this->hWnd, (HMENU)CHECK_AX_VAL, hInst, nullptr);
	CheckDlgButton(this->hWnd, CHECK_GRID, BST_CHECKED);
	CheckDlgButton(this->hWnd, CHECK_AX_VAL, BST_CHECKED);


	this->hRelErrDisplay = CreateWindowEx(WS_EX_TOOLWINDOW, L"VisDisplay", nullptr, WS_VISIBLE | WS_CHILD,
		wndRect.right - this->displayDim, 0, this->displayDim, this->displayDim,
		this->hWnd, nullptr, hInst, nullptr);
	displayStyle = GetWindowLong(this->hRelErrDisplay, GWL_STYLE);
	displayStyle &= ~(WS_BORDER | WS_CAPTION | WS_SYSMENU);
	SetWindowLong(this->hRelErrDisplay, GWL_STYLE, displayStyle);


	this->dialogWidth = wndRect.right / 2;
	this->dialogHeight = wndRect.bottom - this->displayDim - 20;

	this->hPlanePreviewSelection = CreateWindow(L"BUTTON", wClass == 'P' ? L" Select plane " : L" Select line ", WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
		wndRect.right / 4, this->displayDim + 10, dialogWidth, dialogHeight,
		this->hWnd, nullptr, hInst, nullptr);

	this->hOrthAxisGroup = CreateWindow(L"BUTTON", L" Orthogonal to axis ", WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
		wndRect.right / 4 + 30, this->displayDim + dialogHeight / 3, 280, 70,
		this->hWnd, nullptr, hInst, nullptr);
	this->radioButtonX = CreateWindow(L"BUTTON", L"X", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
		wndRect.right / 4 + 70, this->displayDim + dialogHeight / 3 + 30, 40, 20,
		this->hWnd, nullptr, hInst, nullptr);
	this->radioButtonY = CreateWindow(L"BUTTON", L"Y", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
		wndRect.right / 4 + 150, this->displayDim + dialogHeight / 3 + 30, 40, 20,
		this->hWnd, nullptr, hInst, nullptr);
	this->radioButtonZ = CreateWindow(L"BUTTON", L"Z", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
		wndRect.right / 4 + 230, this->displayDim + dialogHeight / 3 + 30, 40, 20,
		this->hWnd, nullptr, hInst, nullptr);
	

	this->hEnterAxisVal = CreateWindow(L"STATIC", L"Enter axis value: ", WS_VISIBLE | WS_CHILD,
		wndRect.right / 4 + 350, this->displayDim + 10 + dialogHeight / 3, 120, 20,
		this->hWnd, nullptr, hInst, nullptr);

	this->hAxisValueBoxContainer = CreateWindowEx(WS_EX_TOOLWINDOW, L"Editable", nullptr, WS_VISIBLE | WS_CHILD,
		wndRect.right / 4 + 480, this->displayDim + 10 + dialogHeight / 3, 120, 20,
		this->hWnd, nullptr, hInst, nullptr);
	displayStyle = GetWindowLong(this->hAxisValueBoxContainer, GWL_STYLE);
	displayStyle &= ~(WS_BORDER | WS_CAPTION | WS_SYSMENU);
	SetWindowLong(this->hAxisValueBoxContainer, GWL_STYLE, displayStyle);

	this->hAxisValueBox = CreateWindow(L"EDIT", nullptr, WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_BORDER | ES_CENTER,
		0, 0, 120, 20,
		this->hAxisValueBoxContainer, nullptr, hInst, nullptr);
}


void VisMergedWndBase::setResultTitle(HWND hWnd) {
	this->hResultTitle = hWnd;
}


void VisMergedWndBase::setRelErrTitle(HWND hWnd) {
	this->hRelErrTitle = hWnd;
}


void VisMergedWndBase::setResultLegendVal(unsigned index, HWND hWnd) {
	this->hResultLegendVal[index] = hWnd;
}


void VisMergedWndBase::setRelErrLegendVal(unsigned index, HWND hWnd) {
	this->hRelErrLegendVal[index] = hWnd;
}


char VisMergedWndBase::getWClass() {
	return this->wClass;
}


HWND VisMergedWndBase::getHandle() {
	return this->hWnd;
}


HWND VisMergedWndBase::getResultDisplay() {
	return this->hResultDisplay;
}


HWND VisMergedWndBase::getRelErrDisplay() {
	return this->hRelErrDisplay;
}


HWND VisMergedWndBase::getDataWnd() {
	return this->hDataWnd;
}


HWND VisMergedWndBase::getResultLegend() {
	return this->hResultLegend;
}


HWND VisMergedWndBase::getRelErrLegend() {
	return this->hRelErrLegend;
}


HWND VisMergedWndBase::getResultTitle() {
	return this->hResultTitle;
}


HWND VisMergedWndBase::getRelErrTitle() {
	return this->hRelErrTitle;
}


HWND VisMergedWndBase::getResultLegendVal(unsigned index) {
	return this->hResultLegendVal[index];
}


HWND VisMergedWndBase::getRelErrLegendVal(unsigned index) {
	return this->hRelErrLegendVal[index];
}


HWND VisMergedWndBase::getRadioButtonX() {
	return this->radioButtonX;
}


HWND VisMergedWndBase::getRadioButtonY() {
	return this->radioButtonY;
}


HWND VisMergedWndBase::getRadioButtonZ() {
	return this->radioButtonZ;
}


HWND VisMergedWndBase::getAxisValueBox() {
	return this->hAxisValueBox;
}


RECT VisMergedWndBase::getWndRect() {
	return this->wndRect;
}


RECT VisMergedWndBase::getDataWndRect() {
	return this->dataWndRect;
}


int VisMergedWndBase::getDisplayDim() {
	return this->displayDim;
}


int VisMergedWndBase::getDialogHeight() {
	return this->dialogHeight;
}


void VisMergedWndBase::resize() {
	GetClientRect(this->hWnd, &wndRect);
	this->displayDim = int(0.8f * wndRect.right) / 2;
	this->dialogWidth = wndRect.right / 2;
	this->dialogHeight = wndRect.bottom - this->displayDim - 20;

	MoveWindow(hResultDisplay, 0, 0, this->displayDim, this->displayDim, TRUE);
	MoveWindow(hRelErrDisplay, wndRect.right - this->displayDim, 0, this->displayDim, this->displayDim, TRUE);

	MoveWindow(hDataWnd, this->displayDim, 0, wndRect.right - 2 * this->displayDim, this->displayDim, TRUE);
	GetClientRect(this->hDataWnd, &dataWndRect);

	MoveWindow(hResultLegend, dataWndRect.right / 2 - int(0.05f * this->displayDim), this->displayDim / 5 + ((wClass == 'P') ? 0 : 20), this->displayDim / 25, this->displayDim / 2, TRUE);
	MoveWindow(hRelErrLegend, dataWndRect.right / 2 + int(0.01f * this->displayDim), this->displayDim / 5 + ((wClass == 'P') ? 0 : 20), this->displayDim / 25, this->displayDim / 2, TRUE);

	MoveWindow(hResultTitle, dataWndRect.right / 2 - int(0.05f * this->displayDim) - 80, this->displayDim / 5 - 60 + ((wClass == 'P') ? 0 : 20), 120, 20, TRUE);
	MoveWindow(hRelErrTitle, dataWndRect.right / 2 + int(0.1f * this->displayDim), this->displayDim / 5 - 60 + ((wClass == 'P') ? 0 : 20), 120, 40, TRUE);

	for (int i = 0; i < 6; ++i) {
		MoveWindow(hResultLegendVal[i], dataWndRect.right / 2 - 100 - int(0.025f * this->displayDim), int(this->displayDim * (2.f + i) / 10) - 10 + ((wClass == 'P') ? 0 : 20), 120, 20, TRUE);
		MoveWindow(hRelErrLegendVal[i], dataWndRect.right / 2 + int(0.05f * this->displayDim), int(this->displayDim * (2.f + i) / 10) - 10 + ((wClass == 'P') ? 0 : 20), 120, 20, TRUE);
	}

	MoveWindow(hResultLegendVal[6], dataWndRect.right / 2 - 100 - int(0.025f * this->displayDim), int(this->displayDim * 0.8f) + ((wClass == 'P') ? 0 : 20), 120, 20, TRUE);
	MoveWindow(hRelErrLegendVal[6], dataWndRect.right / 2 + int(0.05f * this->displayDim), int(this->displayDim * 0.8f) + ((wClass == 'P') ? 0 : 20), 120, 20, TRUE);


	MoveWindow(hCheckGrid, 50, this->displayDim + 20, 100, 20, TRUE);
	MoveWindow(hCheckAxesVals, 50, this->displayDim + 50, 100, 20, TRUE);


	MoveWindow(hPlanePreviewSelection, wndRect.right / 4, this->displayDim + 10, dialogWidth, dialogHeight, TRUE);

	MoveWindow(hOrthAxisGroup, wndRect.right / 4 + 30, this->displayDim + dialogHeight / 3, 280, 70, TRUE);
	MoveWindow(radioButtonX, wndRect.right / 4 + 70, this->displayDim + dialogHeight / 3 + 30, 40, 20, TRUE);
	MoveWindow(radioButtonY, wndRect.right / 4 + 150, this->displayDim + dialogHeight / 3 + 30, 40, 20, TRUE);
	MoveWindow(radioButtonZ, wndRect.right / 4 + 230, this->displayDim + dialogHeight / 3 + 30, 40, 20, TRUE);

	MoveWindow(hEnterAxisVal, wndRect.right / 4 + 350, this->displayDim + 10 + dialogHeight / 3, 120, 20, TRUE);
	MoveWindow(hAxisValueBoxContainer, wndRect.right / 4 + 480, this->displayDim + 10 + dialogHeight / 3, 120, 20, TRUE);
	MoveWindow(hAxisValueBox, 0, 0, 120, 20, TRUE);
}
