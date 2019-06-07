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
#include "VisMergedWindow.h"


class PlanePreview {
private:
	struct CustomColor {
		float r, g, b;
	};

	struct LegendColorLevel {
		CustomColor color;
		long double value;
	};

	struct ScreenVector {
		float x, y;
	};

	struct Vertex {
		float x, y, z;
		CustomColor color;
	};

	struct ConstBufferStruct {
		DirectX::XMMATRIX transform;
	};
public:
	struct Point2D {
		float axisOne, axisTwo;
		long double value, relError;
	};
private:
	HINSTANCE hCurrentInst;
	HACCEL hAccelTable;
	char axis;
	float axisValue;
	LPWSTR fileAbsolutePath;
	LPWSTR windowTitle;

	std::unique_ptr<VisMergedWindow> hVisMerWnd;
public:
	PlanePreview(char axis, float axisValue);
	~PlanePreview() = default;

	void run(DWORD callingThreadId, HINSTANCE hCurrentInst, HACCEL hAccelTable, LPWSTR fileAbsolutePath, int planePointsDataSize, Point2D* planePointsData);
};
