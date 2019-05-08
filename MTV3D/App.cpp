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


LRESULT CALLBACK App::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	WndClass::Type wcType = WndClass::typeByWndHandle(hWnd);

	switch (message) {
	case WM_TIMER: {
		switch (wParam) {
		case IDT_TIMER_SPLASH:
			DestroyWindow(hWnd);

			App::appPointer->hMainWnd = std::make_unique<MainWindow>(App::appPointer->hCurrentInst);
			App::appPointer->hMainWnd->initListView(App::appPointer->hCurrentInst);
			ShowWindow(App::appPointer->hMainWnd->getHandle(), SW_SHOWMAXIMIZED);

			break;
		}
		break;
	}
	case WM_NOTIFY: {
		LPNMHDR lpnmh = (LPNMHDR)lParam;
		HWND hwndListView = GetDlgItem(hWnd, ID_LISTVIEW);

		switch (lpnmh->code) {
		case LVN_GETDISPINFO: {
			LV_DISPINFO *lpdi = (LV_DISPINFO*)lParam;

			if (lpdi->item.iSubItem) {
				if (lpdi->item.mask & LVIF_TEXT) {
					lpdi->item.pszText = App::appPointer->getListViewString(lpdi->item.iItem, lpdi->item.iSubItem);
				}
			}
			else {
				if (lpdi->item.mask & LVIF_TEXT) {
					lpdi->item.pszText = App::appPointer->getListViewString(lpdi->item.iItem, lpdi->item.iSubItem);
				}
			}
		}
		}
		break;
	}
	case WM_SIZE: {
		switch (wcType) {
		case WndClass::Type::MAIN: {
			if (App::appPointer->hMainWnd) {
				App::appPointer->hMainWnd->resizeListView();
			}
			break;
		}
		}
		break;
	}
	case WM_COMMAND: {
		int wmId = LOWORD(wParam);
		
		switch (wmId) {
		case IDM_NEW_PROJ:
		case BUTTON_NEW_PROJ: {
			OPENFILENAME ofnObj;
			
			ZeroMemory(&ofnObj, sizeof(OPENFILENAME));
			
			ofnObj.lStructSize = sizeof(OPENFILENAME);
			ofnObj.hwndOwner = App::appPointer->hMainWnd->getHandle();
			ofnObj.hInstance = App::appPointer->hCurrentInst;
			ofnObj.lpstrFile = new WCHAR[FILEPATH_MAX_LENGTH];
			ofnObj.lpstrFile[0] = '\0';
			ofnObj.nMaxFile = FILEPATH_MAX_LENGTH;
			ofnObj.lpstrTitle = L"Select file";
			ofnObj.Flags = OFN_FILEMUSTEXIST;

			if (GetOpenFileName(&ofnObj)) {
				SetCursor(LoadCursor(nullptr, IDC_WAIT));

				bool projectLoaded = App::appPointer->loadFile(ofnObj.lpstrFile);

				SetCursor(LoadCursor(nullptr, IDC_ARROW));

				if (! projectLoaded) {
					delete[] ofnObj.lpstrFile;

					MessageBox(App::appPointer->hMainWnd->getHandle(), L"Wrong file type!", nullptr, MB_ICONERROR);
				}
			}
			else {
				
				delete[] ofnObj.lpstrFile;
			}
			
			break;
		}
		case IDM_EXIT: {
			DestroyWindow(hWnd);
			break;
		}
		case IDM_CHANGE_LANG: {
			DialogBox(nullptr, L"Chage language", hWnd, nullptr);
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
			break;
		}
		case WndClass::Type::MAIN: {
			//break;
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
		App::wndProc,
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
		App::wndProc,
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


bool App::loadFile(LPWSTR fileAbsolutePath) {
	HANDLE hFile = CreateFile(fileAbsolutePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (hFile == INVALID_HANDLE_VALUE) {
		return false;
	}
	
	BY_HANDLE_FILE_INFORMATION fileInfo;
	GetFileInformationByHandle(hFile, &fileInfo);

	DWORD fileSize;
	fileSize = GetFileSize(hFile, nullptr);
	
	if (fileSize == INVALID_FILE_SIZE) {
		CloseHandle(hFile);
		return false;
	}


	LPCH fileBinaryContent = new char[fileSize + 1];
	DWORD bytesRead;

	BOOL retVal = ReadFile(hFile, fileBinaryContent, fileSize, &bytesRead, nullptr);
	CloseHandle(hFile);

	if(! retVal) {
		return false;
	}
	
	
	fileBinaryContent[fileSize] = '\0';

	DWORD numberOfUTF8Characters = utf8CharacterCounter(fileBinaryContent);
	LPWSTR fileUTF8Content = new WCHAR[numberOfUTF8Characters + 1];

	try {
		MultiByteToWideChar(
			CP_UTF8, 0,
			fileBinaryContent, fileSize,
			fileUTF8Content, numberOfUTF8Characters
		);
	}
	catch (...) {
		delete[] fileBinaryContent;
		delete[] fileUTF8Content;
		return false;
	}

	fileUTF8Content[numberOfUTF8Characters] = '\0';
	delete[] fileBinaryContent;
	std::wistringstream fileContentStringStream(fileUTF8Content);
	delete[] fileUTF8Content;

	std::wstring fileLine;

	for (int cnt = 0; cnt < 15; ++cnt) {
		std::getline(fileContentStringStream, fileLine);
	}
	
	
	bool isPhoton = false;

	for (WCHAR& ch : fileLine) {
		if (ch != ' '  &&  ch != '\t') {
			if (ch != 'X') {
				isPhoton = true;
				break;
			}
		}
	}

	
	std::vector<VisComponent::Point> visPoints;
	LPCWSTR fileLineWCStr;
	VisComponent::Point point;

	while (std::getline(fileContentStringStream, fileLine)) {
		fileLineWCStr = fileLine.c_str();

		if (isPhoton) {
			swscanf_s(fileLineWCStr, L"%*s %lf %lf %lf %lf %lf", &point.x, &point.y, &point.z, &point.value, &point.relError);
		}
		else {
			swscanf_s(fileLineWCStr, L"%lf %lf %lf %lf %lf", &point.x, &point.y, &point.z, &point.value, &point.relError);
		}

		visPoints.push_back(point);
	}


	LPWSTR fileSizeStr = new WCHAR[WCHAR_ARR_MAX];
	LPWSTR fileCreatedStr = new WCHAR[WCHAR_ARR_MAX];
	LPWSTR fileModifiedStr = new WCHAR[WCHAR_ARR_MAX];

	swprintf_s(fileSizeStr, 20, L"%d\0", fileSize);

	SYSTEMTIME stUTC, stLocal;

	FileTimeToSystemTime(&fileInfo.ftCreationTime, &stUTC);
	SystemTimeToTzSpecificLocalTime(nullptr, &stUTC, &stLocal);
	swprintf_s(fileCreatedStr, 20, L"%02d/%02d/%d  %02d:%02d\0", stLocal.wMonth, stLocal.wDay, stLocal.wYear, stLocal.wHour, stLocal.wMinute);

	FileTimeToSystemTime(&fileInfo.ftLastWriteTime, &stUTC);
	SystemTimeToTzSpecificLocalTime(nullptr, &stUTC, &stLocal);
	swprintf_s(fileModifiedStr, 20, L"%02d/%02d/%d  %02d:%02d\0", stLocal.wMonth, stLocal.wDay, stLocal.wYear, stLocal.wHour, stLocal.wMinute);

	std::vector<LPWSTR> lvData { fileAbsolutePath, fileSizeStr, fileCreatedStr, fileModifiedStr };
	VisComponent newProject;
	this->openProjects.push_back(std::make_pair(-1, std::make_pair(lvData, newProject)));


	LVITEM lvItem;
	ZeroMemory(&lvItem, sizeof(LVITEM));
	lvItem.mask = LVIF_PARAM | LVIF_TEXT;
	lvItem.cchTextMax = MAX_PATH;

	int lvIndex = SendMessage(this->hMainWnd->getHandleListView(), LVM_INSERTITEM, 0, (LPARAM)&lvItem);

	if (lvIndex == -1) {
		delete[] fileSizeStr;
		delete[] fileCreatedStr;
		delete[] fileModifiedStr;
		this->openProjects.pop_back();
		
		return false;
	}

	this->openProjects[this->openProjects.size() - 1].first = lvIndex;

	return true;
}


DWORD App::utf8CharacterCounter(LPCH fileBinaryContent) {
	DWORD counter = 0;

	for (int i = 0; fileBinaryContent[i] != '\0'; ++i) {
		if ((fileBinaryContent[i] & 0xC0) != 0x80) {
			++counter;
		}
	}

	return counter;
}


LPWSTR App::getListViewString(int itemIndex, int subitemIndex) {
	return this->openProjects[itemIndex].second.first[subitemIndex];
}
