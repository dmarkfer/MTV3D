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
#include "WndClass.h"
#include "VisMergedWindow.h"


class VisComponent {
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
	struct Point {
		float x, y, z;
		long double value, relError;
	};

	static DWORD mainThreadId;
	static HCURSOR cursorHandNoGrab;
	static HCURSOR cursorHandGrab;
	static HWND cursorGrabInteractionProject;
	static int clickPosX;
	static int clickPosY;
	static float scaleBase;

	static unsigned vertexShaderFileSize;
	static char* vertexShaderBlob;
	static const D3D11_INPUT_ELEMENT_DESC inputElementDesc[];
	static unsigned pixelShaderFileSize;
	static char* pixelShaderBlob;
private:
	HINSTANCE hCurrentInst;
	HACCEL hAccelTable;

	int projectId;
	LPWSTR fileAbsolutePath;
	LPWSTR windowTitle;
	std::vector<Point> visPoints;
	std::vector<std::vector<std::vector<VisComponent::Point>>> vis3DDataModel;
	std::vector<Vertex> vertices;
	std::vector<unsigned> indices;

	Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3dDeviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChainResultDisplay;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChainRelErrDisplay;

	std::unique_ptr<VisMergedWindow> hVisMerWnd;

	std::vector<LegendColorLevel> resultLegend;
	std::vector<LegendColorLevel> relerrLegend;
public:
	VisComponent() = default;
	~VisComponent() = default;

	void run(HINSTANCE hCurrentInst, HACCEL hAccelTable, int projectIndex, LPWSTR fileAbsolutePath, int visPointsDataSize, Point* visPointsData);
	static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
private:
	void initDirect3D();
	CustomColor getResultColor(long double resultValue);
	CustomColor getRelErrColor(long double relerrValue);
};
