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

#include "stdafx.h"
#include "PlanePreview.h"
#include "VisMergedWindow.h"


class VisComponent {
public:
	static DWORD mainThreadId;
	thread_local static float scaleBase;
	thread_local static bool gridActive;
	thread_local static bool axesValsActive;
	thread_local static bool flagPlanePrevCreation;
private:
	HINSTANCE hCurrentInst;
	HACCEL hAccelTable;

	int projectId;
	LPWSTR fileAbsolutePath;
	LPWSTR windowTitle;
	std::vector<Graphics::Point3D> visPoints;
	std::vector<std::vector<std::vector<Graphics::Point3D>>> vis3DDataModel;
	std::vector<Graphics::Vertex> verticesResult;
	std::vector<Graphics::Vertex> verticesRelErr;
	std::vector<unsigned> indices;

	Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3dDeviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChainResultDisplay;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChainRelErrDisplay;

	std::unique_ptr<VisMergedWindow> hVisMerWnd;

	std::vector<Graphics::LegendColorLevel> resultLegend;
	std::vector<Graphics::LegendColorLevel> relerrLegend;

	std::map<std::pair<char, float>, std::unique_ptr<PlanePreview>> openPlanePreviews;
	std::map<std::pair<char, float>, std::thread> planePreviewsThreads;
public:
	VisComponent() = default;
	~VisComponent() = default;

	void run(HINSTANCE hCurrentInst, HACCEL hAccelTable, int projectIndex, LPWSTR fileAbsolutePath, int visPointsDataSize, Graphics::Point3D* visPointsData);
private:
	void initDirect3D();
	Graphics::CustomColor getResultColor(long double resultValue);
	Graphics::CustomColor getRelErrColor(long double resultValue, long double relerrValue);
};
