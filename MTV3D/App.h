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

#include "SplashWindow.h"
#include "MainWindow.h"
#include "VisComponent.h"


class App {
private:
	HINSTANCE hCurrentInst;
	HACCEL hAccelTable;
	static App* appPointer;
	static PAINTSTRUCT ps;
	static HDC hdc;

	std::map<WndClass::Type, WNDCLASSEXW> wndClassTypeStruct;
	std::unique_ptr<SplashWindow> hSplashWnd;
	std::unique_ptr<MainWindow> hMainWnd;

	int projectCounter;
	std::vector<std::pair<int, std::pair<std::vector<LPWSTR>, std::unique_ptr<VisComponent>>>> openProjects;
	int numberOfOpenProjects;
	std::vector<std::pair<int, std::thread>> projectsThreads;

	Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3dDeviceContext;
public:
	App();
	~App() = default;

	int run(HINSTANCE hInstance, int& nCmdShow);
	static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
private:
	void createDirect3DDevice();
	int giveNewProjectId();
	void createWndClasses();
	bool loadFile(LPWSTR fileAbsolutePath);
	DWORD utf8CharacterCounter(LPCH fileBinaryContent);
	LPWSTR getListViewString(int itemIndex, int subitemIndex);
	int retrieveProjectIndexWithinContainer(int projectId);
	void closeProject(int projectId);
	void closeAllProjects();
	void recreateListView();
	int insertItemIntoListView();
};
