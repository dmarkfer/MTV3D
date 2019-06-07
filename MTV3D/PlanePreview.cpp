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
#include "PlanePreview.h"


PlanePreview::PlanePreview(char axis, float axisValue): axis(axis), axisValue(axisValue) {
}


void PlanePreview::run(DWORD callingThreadId, HINSTANCE hCurrentInst, HACCEL hAccelTable, LPWSTR fileAbsolutePath, int planePointsDataSize, Point2D* planePointsData) {
	this->hCurrentInst = hCurrentInst;
	this->hAccelTable = hAccelTable;
	this->fileAbsolutePath = fileAbsolutePath;
	this->windowTitle = new WCHAR[WCHAR_ARR_MAX];
	swprintf_s(this->windowTitle, WCHAR_ARR_MAX - 1, L"MTV3D - %s - %c = %1.3f", this->fileAbsolutePath, this->axis, this->axisValue);

	this->hVisMerWnd = std::make_unique<VisMergedWindow>(this->hCurrentInst, this->windowTitle);
	ShowWindow(this->hVisMerWnd->getHandle(), SW_SHOWMAXIMIZED);

	SetCursor(LoadCursor(nullptr, IDC_ARROW));


	MSG msg;
	bool quitFlag = false;

	while (true) {
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				quitFlag = true;
				break;
			}

			if (!TranslateAccelerator(msg.hwnd, this->hAccelTable, &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		if (quitFlag) {
			break;
		}
	}

	delete[] this->windowTitle;

	float* customLParam = new float[2];
	customLParam[0] = float((int)this->axis);
	customLParam[1] = this->axisValue;

	PostThreadMessage(callingThreadId, WM_THREAD_DONE, 0, (LPARAM)customLParam);
}
