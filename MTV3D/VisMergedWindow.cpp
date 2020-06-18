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


VisMergedWindow::VisMergedWindow(HINSTANCE hInst, LPWSTR windowTitle): VisMergedWndBase(hInst, windowTitle, 'P') {
	SendMessage(radioButtonZ, BM_SETCHECK, BST_CHECKED, 0);

	this->hBtnReset3DModel = CreateWindow(L"BUTTON", L"Reset model",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		180, this->displayDim + 25, 100, 30,
		this->hWnd, (HMENU)BUTTON_RESET_3D_MODEL, hInst, nullptr);

	this->hBtnCreatePlane = CreateWindow(L"BUTTON", L"Create plane preview",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		wndRect.right / 4 + dialogWidth * 2 / 3, this->displayDim + dialogHeight * 2 / 3, 180, 30,
		this->hWnd, (HMENU)BUTTON_CREATE_PLANE, hInst, nullptr);
}


HWND VisMergedWindow::getBtnReset3DModel() {
	return this->hBtnReset3DModel;
}


HWND VisMergedWindow::getBtnCreatePlane() {
	return this->hBtnCreatePlane;
}


void VisMergedWindow::resize() {
	this->VisMergedWndBase::resize();

	MoveWindow(hBtnReset3DModel, 180, this->displayDim + 25, 100, 30, TRUE);
	MoveWindow(hBtnCreatePlane, wndRect.right / 4 + dialogWidth * 2 / 3, this->displayDim + dialogHeight * 2 / 3, 180, 30, TRUE);
}
