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
#include "WndClass.h"
#include "PlaneMergedWnd.h"
#include "Graphics.h"
#include "CursorData.h"


class PlanePreview {
private:
	HINSTANCE hCurrentInst;
	HACCEL hAccelTable;
	char axis;
	float axisValue;
	LPWSTR fileAbsolutePath;
	LPWSTR windowTitle;

	std::unique_ptr<PlaneMergedWnd> hVisMerWnd;

	std::vector<Graphics::LegendColorLevel> resultLegend;
	std::vector<Graphics::LegendColorLevel> relerrLegend;
public:
	PlanePreview(char axis, float axisValue);
	~PlanePreview() = default;

	void run(DWORD callingThreadId, HINSTANCE hCurrentInst, HACCEL hAccelTable, LPWSTR fileAbsolutePath, int planePointsDataSize, Graphics::Point2D* planePointsData);
private:
	Graphics::CustomColor getResultColor(long double resultValue);
	Graphics::CustomColor getRelErrColor(long double relerrValue);
};