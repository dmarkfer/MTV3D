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
	int displayDim = (wndRect.right - 300) / 2;
	int smallDisplayDim = wndRect.bottom - displayDim;

	this->hResultDisplay = CreateWindowEx(WS_EX_TOOLWINDOW, L"VisDisplay", nullptr, WS_VISIBLE | WS_CHILD,
		0, 0, displayDim, displayDim,
		this->hWnd, nullptr, hInst, nullptr);
	LONG displayStyle = GetWindowLong(this->hResultDisplay, GWL_STYLE);
	displayStyle &= ~(WS_BORDER | WS_CAPTION | WS_SYSMENU);
	SetWindowLong(this->hResultDisplay, GWL_STYLE, displayStyle);

	this->hResultDisplaySmall = CreateWindowEx(WS_EX_TOOLWINDOW, L"VisDisplay", nullptr, WS_VISIBLE | WS_CHILD,
		0, displayDim, smallDisplayDim, smallDisplayDim,
		this->hWnd, nullptr, hInst, nullptr);
	displayStyle = GetWindowLong(this->hResultDisplaySmall, GWL_STYLE);
	displayStyle &= ~(WS_BORDER | WS_CAPTION | WS_SYSMENU);
	SetWindowLong(this->hResultDisplaySmall, GWL_STYLE, displayStyle);


	this->hRelErrDisplay = CreateWindowEx(WS_EX_TOOLWINDOW, L"VisDisplay", nullptr, WS_VISIBLE | WS_CHILD,
		wndRect.right - displayDim, 0, displayDim, displayDim,
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
