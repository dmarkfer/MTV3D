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
#include "LinePreviewWnd.h"


LinePreviewWnd::LinePreviewWnd(HINSTANCE hCurrentInst, LPWSTR fileAbsolutePath, std::pair<std::pair<char, float>, std::pair<char, float>> pr, int linePointsDataSize, Graphics::Point1D* linePointsData) {
	SetCursor(LoadCursor(nullptr, IDC_ARROW));

	this->windowTitle = new WCHAR[WCHAR_ARR_MAX];
	swprintf_s(windowTitle, WCHAR_ARR_MAX - 1, L"MTV3D - %s ( %c = %1.3f , %c = %1.3f )", fileAbsolutePath, pr.first.first, pr.first.second, pr.second.first, pr.second.second);

	this->linePoints.assign(linePointsData, linePointsData + linePointsDataSize);
	delete[] linePointsData;

	this->reCreate();
}


LinePreviewWnd::~LinePreviewWnd() {
	DestroyWindow(this->hWnd);
	delete[] this->windowTitle;
}


HWND LinePreviewWnd::getHandle() {
	return this->hWnd;
}


void LinePreviewWnd::reCreate() {
	this->hWnd = CreateWindowW(L"VisLine", this->windowTitle, WS_OVERLAPPEDWINDOW | WS_MAXIMIZE,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, this->hCurrentInst, nullptr);
	ShowWindow(this->hWnd, SW_SHOW);
}
