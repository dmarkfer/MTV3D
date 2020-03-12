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
#include "LinePreviewWnd.h"


class PlanePreview {
private:
	HINSTANCE hCurrentInst;
	HACCEL hAccelTable;
	char axis;
	float axisValue;
	LPWSTR fileAbsolutePath;
	LPWSTR windowTitle;

	std::vector<Graphics::Vertex> verticesResult;
	std::vector<Graphics::Vertex> verticesRelErr;
	std::vector<unsigned> indices;

	Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3dDeviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChainResultDisplay;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChainRelErrDisplay;


	std::vector<Graphics::LegendColorLevel> resultLegend;
	std::vector<Graphics::LegendColorLevel> relerrLegend;

public:
	std::unique_ptr<PlaneMergedWnd> hPlaneMerWnd;
	std::map<std::pair<std::pair<char, float>, std::pair<char, float>>, std::unique_ptr<LinePreviewWnd>> openLinePreviews;

public:
	thread_local static float scaleBase;
	thread_local static bool gridActive;
	thread_local static bool axesValsActive;
	thread_local static bool flagLinePrevCreation;
public:
	PlanePreview(char axis, float axisValue);
	~PlanePreview() = default;

	char getAxis();
	void run(DWORD callingThreadId, HINSTANCE hCurrentInst, HACCEL hAccelTable, LPWSTR fileAbsolutePath, int planePointsDataSize, Graphics::Point2D* planePointsData);
private:
	void initDirect3D();
	Graphics::CustomColor getResultColor(long double resultValue);
	Graphics::CustomColor getRelErrColor(long double resultValue, long double relerrValue);
};
