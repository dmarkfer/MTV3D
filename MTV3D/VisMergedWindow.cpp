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
#include "VisMergedWindow.h"


VisMergedWindow::VisMergedWindow(HINSTANCE hInst, LPWSTR windowTitle) {
	SetCursor(LoadCursor(nullptr, IDC_ARROW));

	this->hWnd = CreateWindowW(L"VisMerged", windowTitle, WS_OVERLAPPEDWINDOW | WS_MAXIMIZE,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInst, nullptr);


	RECT wndRect;
	GetClientRect(this->hWnd, &wndRect);
	this->displayDim = (wndRect.right - 300) / 2;
	int smallDisplayDim = wndRect.bottom - this->displayDim;

	this->hResultDisplay = CreateWindowEx(WS_EX_TOOLWINDOW, L"VisDisplay", nullptr, WS_VISIBLE | WS_CHILD,
		0, 0, this->displayDim, this->displayDim,
		this->hWnd, nullptr, hInst, nullptr);
	LONG displayStyle = GetWindowLong(this->hResultDisplay, GWL_STYLE);
	displayStyle &= ~(WS_BORDER | WS_CAPTION | WS_SYSMENU);
	SetWindowLong(this->hResultDisplay, GWL_STYLE, displayStyle);

	this->hResultDisplaySmall = CreateWindowEx(WS_EX_TOOLWINDOW, L"VisDisplay", nullptr, WS_VISIBLE | WS_CHILD,
		0, this->displayDim, smallDisplayDim, smallDisplayDim,
		this->hWnd, nullptr, hInst, nullptr);
	displayStyle = GetWindowLong(this->hResultDisplaySmall, GWL_STYLE);
	displayStyle &= ~(WS_BORDER | WS_CAPTION | WS_SYSMENU);
	SetWindowLong(this->hResultDisplaySmall, GWL_STYLE, displayStyle);


	this->hResultLegend = CreateWindowEx(WS_EX_TOOLWINDOW, L"VisLegend", nullptr, WS_VISIBLE | WS_CHILD,
		wndRect.right / 2 - this->displayDim / 25 - 10, 100, this->displayDim / 25, this->displayDim / 2,
		this->hWnd, nullptr, hInst, nullptr);
	displayStyle = GetWindowLong(this->hResultLegend, GWL_STYLE);
	displayStyle &= ~(WS_BORDER | WS_CAPTION | WS_SYSMENU);
	SetWindowLong(this->hResultLegend, GWL_STYLE, displayStyle);

	this->hRelErrLegend = CreateWindowEx(WS_EX_TOOLWINDOW, L"VisLegend", nullptr, WS_VISIBLE | WS_CHILD,
		wndRect.right / 2 + 10, 100, this->displayDim / 25, this->displayDim / 2,
		this->hWnd, nullptr, hInst, nullptr);
	displayStyle = GetWindowLong(this->hRelErrLegend, GWL_STYLE);
	displayStyle &= ~(WS_BORDER | WS_CAPTION | WS_SYSMENU);
	SetWindowLong(this->hRelErrLegend, GWL_STYLE, displayStyle);


	this->hRelErrDisplay = CreateWindowEx(WS_EX_TOOLWINDOW, L"VisDisplay", nullptr, WS_VISIBLE | WS_CHILD,
		wndRect.right - this->displayDim, 0, this->displayDim, this->displayDim,
		this->hWnd, nullptr, hInst, nullptr);
	displayStyle = GetWindowLong(this->hRelErrDisplay, GWL_STYLE);
	displayStyle &= ~(WS_BORDER | WS_CAPTION | WS_SYSMENU);
	SetWindowLong(this->hRelErrDisplay, GWL_STYLE, displayStyle);

	this->hRelErrDisplaySmall = CreateWindowEx(WS_EX_TOOLWINDOW, L"VisDisplay", nullptr, WS_VISIBLE | WS_CHILD,
		wndRect.right - smallDisplayDim, wndRect.bottom - smallDisplayDim, smallDisplayDim, smallDisplayDim,
		this->hWnd, nullptr, hInst, nullptr);
	displayStyle = GetWindowLong(this->hRelErrDisplaySmall, GWL_STYLE);
	displayStyle &= ~(WS_BORDER | WS_CAPTION | WS_SYSMENU);
	SetWindowLong(this->hRelErrDisplaySmall, GWL_STYLE, displayStyle);


	this->hPlanePreviewSelection = CreateWindow(L"BUTTON", L"Select plane", WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
		wndRect.right / 4, this->displayDim + 10, wndRect.right / 2, wndRect.bottom - this->displayDim - 20,
		this->hWnd, nullptr, hInst, nullptr);

	this->hOrthAxisGroup = CreateWindow(L"BUTTON", L"Orthogonal to axis: ", WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
		wndRect.right / 4 + 50, this->displayDim + 40, 220, 70,
		this->hWnd, nullptr, hInst, nullptr);
	this->radioButtonX = CreateWindow(L"BUTTON", L"X", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
		wndRect.right / 4 + 80, this->displayDim + 70, 40, 20,
		this->hWnd, nullptr, hInst, nullptr);
	this->radioButtonY = CreateWindow(L"BUTTON", L"Y", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
		wndRect.right / 4 + 140, this->displayDim + 70, 40, 20,
		this->hWnd, nullptr, hInst, nullptr);
	this->radioButtonZ = CreateWindow(L"BUTTON", L"Z", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
		wndRect.right / 4 + 200, this->displayDim + 70, 40, 20,
		this->hWnd, nullptr, hInst, nullptr);
	SendMessage(radioButtonZ, BM_SETCHECK, BST_CHECKED, 0);


	this->hEnterAxisVal = CreateWindow(L"STATIC", L"Enter axis value: ", WS_VISIBLE | WS_CHILD,
		wndRect.right / 4 + 320, this->displayDim + 50, 120, 20,
		this->hWnd, nullptr, hInst, nullptr);

	this->hAxisValueBoxContainer = CreateWindowEx(WS_EX_TOOLWINDOW, L"Editable", nullptr, WS_VISIBLE | WS_CHILD,
		wndRect.right / 4 + 480, this->displayDim + 50, 120, 20,
		this->hWnd, nullptr, hInst, nullptr);
	displayStyle = GetWindowLong(this->hAxisValueBoxContainer, GWL_STYLE);
	displayStyle &= ~(WS_BORDER | WS_CAPTION | WS_SYSMENU);
	SetWindowLong(this->hAxisValueBoxContainer, GWL_STYLE, displayStyle);

	this->hAxisValueBox = CreateWindow(L"EDIT", nullptr, WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_BORDER | ES_CENTER,
		0, 0, 120, 20,
		this->hAxisValueBoxContainer, nullptr, hInst, nullptr);

	this->hBtnCreatePlane = CreateWindow(L"BUTTON", L"Create plane preview",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | WS_DISABLED,
		wndRect.right / 4 + 450, this->displayDim + 90, 180, 30,
		this->hWnd, (HMENU)BUTTON_CREATE_PLANE, hInst, nullptr);
}


HWND VisMergedWindow::getHandle() {
	return this->hWnd;
}


HWND VisMergedWindow::getResultDisplay() {
	return this->hResultDisplay;
}


HWND VisMergedWindow::getResultDisplaySmall() {
	return this->hResultDisplaySmall;
}


HWND VisMergedWindow::getRelErrDisplay() {
	return this->hRelErrDisplay;
}


HWND VisMergedWindow::getRelErrDisplaySmall() {
	return this->hRelErrDisplaySmall;
}


HWND VisMergedWindow::getResultLegend() {
	return this->hResultLegend;
}


HWND VisMergedWindow::getRelErrLegend() {
	return this->hRelErrLegend;
}


int VisMergedWindow::getDisplayDim() {
	return this->displayDim;
}
