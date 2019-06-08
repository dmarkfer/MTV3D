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

#include "Graphics.h"


class LinePreviewWnd {
private:
	HWND hWnd;
	HINSTANCE hCurrentInst;
	LPWSTR windowTitle;

	std::pair<std::pair<char, float>, std::pair<char, float>> pr;

	int linePointsDataSize;
	std::vector<Graphics::Point1D> linePoints;

	std::vector<HWND> childWnds;

	static std::set<LinePreviewWnd*> lineWndSet;
public:
	LinePreviewWnd(HINSTANCE hCurrentInst, LPWSTR fileAbsolutePath, std::pair<std::pair<char, float>, std::pair<char, float>> pr, int linePointsDataSize, Graphics::Point1D* linePointsData);
	~LinePreviewWnd();

	HWND getHandle();
	void reCreate();
	static void drawChart(PAINTSTRUCT* ps, HWND hWnd);
};
