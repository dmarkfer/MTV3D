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

#include "WindowFactory.h"
#include <vector>
#include <utility>
#include <map>

class App {
private:
	static App* appPointer;

	std::map<WndClass::Type, WNDCLASSEXW> wndClassTypeStruct;
	HWND hSplashWnd;
	HWND hMainWnd;
	std::vector<std::pair<HWND, HWND>> hVisWnd;
public:
	HINSTANCE hCurrentInst;

	App();
	~App() = default;

	int run(HINSTANCE hInstance, int& nCmdShow);
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
private:
	void createWndClasses();
};
