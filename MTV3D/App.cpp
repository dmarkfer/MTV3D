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
#include "App.h"


App* App::appPointer = nullptr;
PAINTSTRUCT App::ps = {};
HDC App::hdc = nullptr;


App::App() {
	App::appPointer = this;
}


INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


int App::run(HINSTANCE hInstance, int& nCmdShow) {
	this->hCurrentInst = hInstance;
	this->createWndClasses();

	this->hSplashWnd = std::make_unique<SplashWindow>(this->hCurrentInst);
	ShowWindow(this->hSplashWnd->getHandle(), nCmdShow);
	SetTimer(this->hSplashWnd->getHandle(), IDT_TIMER_SPLASH, SPLASH_TTL, nullptr);

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MTV3D));
	MSG msg;

	while(GetMessage(&msg, nullptr, 0, 0)) {
		if(!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}


LRESULT CALLBACK App::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	LPTSTR lpWndClassName = new wchar_t[WCHAR_ARR_MAX];
	GetClassName(hWnd, lpWndClassName, WCHAR_ARR_MAX);
	WndClass::Type wcType = WndClass::retrieveWndClassType(std::wstring(lpWndClassName));
	delete[] lpWndClassName;

	switch (message) {
	case WM_TIMER: {
		switch (wParam) {
		case IDT_TIMER_SPLASH:
			DestroyWindow(hWnd);

			App::appPointer->hMainWnd = std::make_unique<MainWindow>(App::appPointer->hCurrentInst);
			ShowWindow(App::appPointer->hMainWnd->getHandle(), SW_SHOWMAXIMIZED);

			break;
		}
		break;
	}
	case WM_COMMAND: {
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId) {
		case IDM_NEW_PROJ: {
			DialogBox(nullptr, L"New project", hWnd, nullptr);
			break;
		}
		case IDM_EXIT: {
			DestroyWindow(hWnd);
			break;
		}
		case IDM_CHANGE_LANG: {
			DialogBox(nullptr, L"Chage Language", hWnd, nullptr);
			break;
		}
		case IDM_UPDATE: {
			DialogBox(nullptr, L"Update", hWnd, nullptr);
			break;
		}
		case IDM_DOC: {
			DialogBox(nullptr, L"Documentation", hWnd, nullptr);
			break;
		}
		case IDM_ABOUT: {
			DialogBox(nullptr, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		}
		default: {
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		}
		break;
	}
	case WM_PAINT: {
		switch (wcType) {
		case WndClass::Type::SPLASH: {
			hdc = BeginPaint(hWnd, &ps);

			App::appPointer->hSplashWnd->loadSplash(hdc);
			
			EndPaint(hWnd, &ps);
			DeleteDC(hdc);
			break;
		}
		case WndClass::Type::MAIN: {
			hdc = BeginPaint(hWnd, &ps);

			App::appPointer->hMainWnd->loadLogo(hdc);

			EndPaint(hWnd, &ps);
			DeleteDC(hdc);
			break;
		}
		}
		break;
	}
	case WM_DESTROY: {
		switch (wcType) {
		case WndClass::Type::SPLASH: {
			App::appPointer->hSplashWnd.reset();
			break;
		}
		case WndClass::Type::MAIN: {
			App::appPointer->hMainWnd.reset();
		}
		default: {
			PostQuitMessage(0);
		}
		}
		break;
	}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}


INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}



void App::createWndClasses() {
	this->wndClassTypeStruct.clear();

	this->wndClassTypeStruct[WndClass::Type::SPLASH] = {
		sizeof(WNDCLASSEXW),
		CS_HREDRAW | CS_VREDRAW,
		App::WndProc,
		0,
		0,
		this->hCurrentInst,
		LoadIcon(this->hCurrentInst, MAKEINTRESOURCE(IDI_MTV3D)),
		LoadCursor(this->hCurrentInst, IDC_ARROW),
		CreateSolidBrush(DARK_GRAY),
		nullptr,
		L"Splash",
		LoadIcon(this->hCurrentInst, MAKEINTRESOURCE(IDI_MTV3D))
	};

	this->wndClassTypeStruct[WndClass::Type::MAIN] = {
		sizeof(WNDCLASSEXW),
		CS_HREDRAW | CS_VREDRAW,
		App::WndProc,
		0,
		0,
		this->hCurrentInst,
		LoadIcon(this->hCurrentInst, MAKEINTRESOURCE(IDI_MTV3D)),
		LoadCursor(this->hCurrentInst, IDC_ARROW),
		CreateSolidBrush(DARK_GRAY),
		MAKEINTRESOURCEW(IDC_MTV3D),
		L"Main",
		LoadIcon(this->hCurrentInst, MAKEINTRESOURCE(IDI_MTV3D))
	};

	RegisterClassExW(&this->wndClassTypeStruct[WndClass::Type::SPLASH]);
	RegisterClassExW(&this->wndClassTypeStruct[WndClass::Type::MAIN]);
}
