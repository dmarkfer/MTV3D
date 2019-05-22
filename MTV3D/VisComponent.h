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
public:
	struct Point {
		double x, y, z;
		long double value, relError;
	};

	static DWORD mainThreadId;
	static LPWSTR appRootDir;
private:
	static std::mutex mtx;

	HINSTANCE hCurrentInst;
	HACCEL hAccelTable;

	int projectId;
	LPWSTR fileAbsolutePath;
	LPWSTR windowTitle;
	std::vector<Point> visPoints;
	
	Microsoft::WRL::ComPtr<ID3D11Device>& d3dDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>& d3dDeviceContext;
	Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
	Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
	Microsoft::WRL::ComPtr<IDXGIFactory> dxgiFactory;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;

	std::unique_ptr<VisMergedWindow> hVisMerWnd;
public:
	VisComponent(Microsoft::WRL::ComPtr<ID3D11Device>& d3dDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext>& d3dDeviceContext);
	~VisComponent() = default;

	void run(HINSTANCE hCurrentInst, HACCEL hAccelTable, int projectIndex, LPWSTR fileAbsolutePath, int n, Point* visPoints);
	static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
private:
	void initDirect3D();
};
