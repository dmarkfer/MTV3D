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


bool App::quitFlag = false;
App* App::appPointer = nullptr;
PAINTSTRUCT App::ps = {};
HDC App::hdc = nullptr;


App::App() {
	App::appPointer = this;

	this->projectCounter = 0;
	this->numberOfOpenProjects = 0;

	VisComponent::mainThreadId = GetCurrentThreadId();
}


App::~App() {
	delete[] VisComponent::vertexShaderBlob;
	delete[] VisComponent::pixelShaderBlob;
}


INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


int App::run(HINSTANCE hInstance, int& nCmdShow) {
	this->hCurrentInst = hInstance;
	
	this->readD3DShaders();

	this->createWndClasses();

	this->hSplashWnd = std::make_unique<SplashWindow>(this->hCurrentInst);
	ShowWindow(this->hSplashWnd->getHandle(), nCmdShow);
	SetTimer(this->hSplashWnd->getHandle(), IDT_TIMER_SPLASH, SPLASH_TTL, nullptr);

	this->hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MTV3D));
	MSG msg;

	while(GetMessage(&msg, nullptr, 0, 0)) {
		if(! TranslateAccelerator(msg.hwnd, this->hAccelTable, &msg)) {
			TranslateMessage(&msg);

			if (msg.message == WM_THREAD_DONE) {
				int* projectId = (int*)msg.lParam;
				this->closeProject(*projectId);
				delete projectId;
			}
			else {
				DispatchMessage(&msg);
			}
		}
	}

	return (int)msg.wParam;
}


void App::readD3DShaders() {
	std::ifstream ifStreamShader(L"VertexShader.cso", std::ifstream::in | std::ifstream::binary);
	ifStreamShader.seekg(0, std::ios::end);
	VisComponent::vertexShaderFileSize = (unsigned)ifStreamShader.tellg();
	VisComponent::vertexShaderBlob = new char[VisComponent::vertexShaderFileSize];
	ifStreamShader.seekg(0, std::ios::beg);
	ifStreamShader.read(VisComponent::vertexShaderBlob, VisComponent::vertexShaderFileSize);
	ifStreamShader.close();

	ifStreamShader.open(L"PixelShader.cso", std::ifstream::in | std::ifstream::binary);
	ifStreamShader.seekg(0, std::ios::end);
	VisComponent::pixelShaderFileSize = (unsigned)ifStreamShader.tellg();
	VisComponent::pixelShaderBlob = new char[VisComponent::pixelShaderFileSize];
	ifStreamShader.seekg(0, std::ios::beg);
	ifStreamShader.read(VisComponent::pixelShaderBlob, VisComponent::pixelShaderFileSize);
	ifStreamShader.close();
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
		case LVN_ITEMCHANGED: {
			if (SendMessage(App::appPointer->hMainWnd->getHandleListView(), LVM_GETSELECTEDCOUNT, 0, 0)) {
				EnableWindow(App::appPointer->hMainWnd->getHandleBtnCloseSel(), TRUE);
			}
			else {
				EnableWindow(App::appPointer->hMainWnd->getHandleBtnCloseSel(), FALSE);
			}
			break;
		}
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
		case BUTTON_CLOSE_SEL: {

			int id = -1;
			
			while(true) {
				id = SendMessage(App::appPointer->hMainWnd->getHandleListView(), LVM_GETNEXTITEM, id, LVNI_SELECTED);

				if (id > -1) {
					PostThreadMessage(
						GetThreadId(App::appPointer->projectsThreads[id].second.native_handle()),
						WM_QUIT, 0, 0
					);
				}
				else {
					break;
				}
			}
			break;
		}
		case BUTTON_CLOSE_ALL: {
			App::appPointer->closeAllProjects();
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
		case WndClass::Type::MAIN: {
			if (App::appPointer->numberOfOpenProjects) {
				App::quitFlag = true;
				App::appPointer->closeAllProjects();
			}
			else {
				PostQuitMessage(0);
			}

			break;
		}
		default: {
			break;
		}
		}
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

	this->wndClassTypeStruct[WndClass::Type::VIS_MERGED] = {
		sizeof(WNDCLASSEXW),
		CS_HREDRAW | CS_VREDRAW,
		VisComponent::wndProc,
		0,
		0,
		this->hCurrentInst,
		LoadIcon(this->hCurrentInst, MAKEINTRESOURCE(IDI_MTV3D)),
		LoadCursor(this->hCurrentInst, IDC_ARROW),
		CreateSolidBrush(DARK_GRAY),
		MAKEINTRESOURCEW(IDC_MTV3D_VIS),
		L"VisMerged",
		LoadIcon(this->hCurrentInst, MAKEINTRESOURCE(IDI_MTV3D))
	};

	this->wndClassTypeStruct[WndClass::Type::VIS_RESULT] = {
		sizeof(WNDCLASSEXW),
		CS_HREDRAW | CS_VREDRAW,
		VisComponent::wndProc,
		0,
		0,
		this->hCurrentInst,
		LoadIcon(this->hCurrentInst, MAKEINTRESOURCE(IDI_MTV3D)),
		LoadCursor(this->hCurrentInst, IDC_ARROW),
		CreateSolidBrush(DARK_GRAY),
		MAKEINTRESOURCEW(IDC_MTV3D_VIS),
		L"VisResult",
		LoadIcon(this->hCurrentInst, MAKEINTRESOURCE(IDI_MTV3D))
	};

	this->wndClassTypeStruct[WndClass::Type::VIS_RELERR] = {
		sizeof(WNDCLASSEXW),
		CS_HREDRAW | CS_VREDRAW,
		VisComponent::wndProc,
		0,
		0,
		this->hCurrentInst,
		LoadIcon(this->hCurrentInst, MAKEINTRESOURCE(IDI_MTV3D)),
		LoadCursor(this->hCurrentInst, IDC_ARROW),
		CreateSolidBrush(DARK_GRAY),
		MAKEINTRESOURCEW(IDC_MTV3D_VIS),
		L"VisRelErr",
		LoadIcon(this->hCurrentInst, MAKEINTRESOURCE(IDI_MTV3D))
	};

	this->wndClassTypeStruct[WndClass::Type::VIS_DISPLAY] = {
		sizeof(WNDCLASSEXW),
		CS_HREDRAW | CS_VREDRAW,
		VisComponent::wndProc,
		0,
		0,
		this->hCurrentInst,
		nullptr,
		LoadCursor(this->hCurrentInst, IDC_HAND),
		CreateSolidBrush(WHITE),
		nullptr,
		L"VisDisplay",
		nullptr
	};


	RegisterClassExW(&this->wndClassTypeStruct[WndClass::Type::SPLASH]);
	RegisterClassExW(&this->wndClassTypeStruct[WndClass::Type::MAIN]);
	RegisterClassExW(&this->wndClassTypeStruct[WndClass::Type::VIS_MERGED]);
	RegisterClassExW(&this->wndClassTypeStruct[WndClass::Type::VIS_RESULT]);
	RegisterClassExW(&this->wndClassTypeStruct[WndClass::Type::VIS_RELERR]);
	RegisterClassExW(&this->wndClassTypeStruct[WndClass::Type::VIS_DISPLAY]);
}


bool App::loadFile(LPWSTR fileAbsolutePath) {
	HANDLE hFile = CreateFile(fileAbsolutePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (hFile == INVALID_HANDLE_VALUE) {
		return false;
	}


	DWORD fileSize;
	fileSize = GetFileSize(hFile, nullptr);
	
	if (fileSize == INVALID_FILE_SIZE) {
		CloseHandle(hFile);
		return false;
	}


	BY_HANDLE_FILE_INFORMATION fileInfo;
	if (! GetFileInformationByHandle(hFile, &fileInfo)) {
		CloseHandle(hFile);
		return false;
	}


	LPCH fileRawContent = new char[fileSize + 1];
	DWORD bytesRead;

	BOOL retVal = ReadFile(hFile, fileRawContent, fileSize, &bytesRead, nullptr);
	CloseHandle(hFile);

	if(! retVal) {
		return false;
	}
	
	
	fileRawContent[fileSize] = '\0';

	DWORD numberOfUTF8Characters = utf8CharacterCounter(fileRawContent);
	LPWSTR fileUTF8Content = new WCHAR[numberOfUTF8Characters + 1];

	try {
		MultiByteToWideChar(
			CP_UTF8, 0,
			fileRawContent, fileSize,
			fileUTF8Content, numberOfUTF8Characters
		);
	}
	catch (...) {
		delete[] fileRawContent;
		delete[] fileUTF8Content;
		return false;
	}

	fileUTF8Content[numberOfUTF8Characters] = '\0';
	delete[] fileRawContent;
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
	/*VisComponent::Point point;
	LPCWSTR fileLineWCStr;

	if (isPhoton) {
		while (std::getline(fileContentStringStream, fileLine)) {
			fileLineWCStr = fileLine.c_str();
			swscanf_s(fileLineWCStr, L"%*s %lf %lf %lf %lf %lf", &point.x, &point.y, &point.z, &point.value, &point.relError);
			visPoints.push_back(point);
		}
	}
	else {
		while (std::getline(fileContentStringStream, fileLine)) {
			fileLineWCStr = fileLine.c_str();
			swscanf_s(fileLineWCStr, L"%lf %lf %lf %lf %lf", &point.x, &point.y, &point.z, &point.value, &point.relError);
			visPoints.push_back(point);
		}
	}*/


	LPWSTR fileSizeStr = new WCHAR[WCHAR_ARR_MAX];
	LPWSTR fileCreatedStr = new WCHAR[WCHAR_ARR_MAX];
	LPWSTR fileModifiedStr = new WCHAR[WCHAR_ARR_MAX];

	swprintf_s(fileSizeStr, 20, L"%d", fileSize);

	SYSTEMTIME stUTC, stLocal;

	FileTimeToSystemTime(&fileInfo.ftCreationTime, &stUTC);
	SystemTimeToTzSpecificLocalTime(nullptr, &stUTC, &stLocal);
	swprintf_s(fileCreatedStr, 20, L"%02d/%02d/%d  %02d:%02d", stLocal.wMonth, stLocal.wDay, stLocal.wYear, stLocal.wHour, stLocal.wMinute);

	FileTimeToSystemTime(&fileInfo.ftLastWriteTime, &stUTC);
	SystemTimeToTzSpecificLocalTime(nullptr, &stUTC, &stLocal);
	swprintf_s(fileModifiedStr, 20, L"%02d/%02d/%d  %02d:%02d", stLocal.wMonth, stLocal.wDay, stLocal.wYear, stLocal.wHour, stLocal.wMinute);

	std::vector<LPWSTR> lvData { fileAbsolutePath, fileSizeStr, fileCreatedStr, fileModifiedStr };
	int newProjectId = this->giveNewProjectId();
	this->openProjects.push_back(std::make_pair(newProjectId, std::make_pair(lvData, nullptr)));
	

	if (insertItemIntoListView() == -1) {
		delete[] fileSizeStr;
		delete[] fileCreatedStr;
		delete[] fileModifiedStr;
		this->openProjects.pop_back();
		
		return false;
	}

	++this->numberOfOpenProjects;
	
	this->openProjects[this->numberOfOpenProjects - 1].second.second = std::make_unique<VisComponent>();

	this->projectsThreads.push_back(
		std::make_pair(
			newProjectId,
			std::thread(
				&VisComponent::run,
				this->openProjects[this->numberOfOpenProjects - 1].second.second.get(),
				this->hCurrentInst, this->hAccelTable, newProjectId, fileAbsolutePath, visPoints.size(), visPoints.data()
			)
		)
	);

	EnableWindow(this->hMainWnd->getHandleBtnCloseAll(), TRUE);
	
	return true;
}


DWORD App::utf8CharacterCounter(LPCH fileRawContent) {
	DWORD counter = 0;

	for (int i = 0; fileRawContent[i] != '\0'; ++i) {
		if ((fileRawContent[i] & 0xC0) != 0x80) {
			++counter;
		}
	}

	return counter;
}


LPWSTR App::getListViewString(int itemIndex, int subitemIndex) {
	return this->openProjects[itemIndex].second.first[subitemIndex];
}


void App::closeProject(int projectId) {
	int index = this->retrieveProjectIndexWithinContainer(projectId);

	this->projectsThreads[index].second.join();
	this->projectsThreads.erase(this->projectsThreads.begin() + index);

	
	this->hMainWnd->deleteListViewItem(index);
	

	--this->numberOfOpenProjects;


	EnableWindow(this->hMainWnd->getHandleBtnCloseSel(), FALSE);

	if (! this->numberOfOpenProjects) {
		EnableWindow(this->hMainWnd->getHandleBtnCloseAll(), FALSE);
	}

	for (LPWSTR fiEl : this->openProjects[index].second.first) {
		delete[] fiEl;
	}
	this->openProjects.erase(this->openProjects.begin() + index);


	this->recreateListView();

	if (App::quitFlag) {
		if (! this->numberOfOpenProjects) {
			PostQuitMessage(0);
		}
	}
}


void App::recreateListView() {
	this->hMainWnd->deleteListViewItem(-1);

	ListView_SetItemCount(this->hMainWnd->getHandleListView(), this->numberOfOpenProjects);
}


void App::closeAllProjects() {
	for (int i = 0; i < this->numberOfOpenProjects; ++i) {
		PostThreadMessage(
			GetThreadId(this->projectsThreads[i].second.native_handle()),
			WM_QUIT, 0, 0
		);
	}
}


int App::insertItemIntoListView() {
	LVITEM lvItem;

	ZeroMemory(&lvItem, sizeof(LVITEM));

	lvItem.iItem = SendMessage(this->hMainWnd->getHandleListView(), LVM_GETITEMCOUNT, 0, 0);
	lvItem.iSubItem = 0;
	lvItem.mask = LVIF_PARAM | LVIF_TEXT;
	lvItem.cchTextMax = MAX_PATH;

	int index = SendMessage(this->hMainWnd->getHandleListView(), LVM_INSERTITEM, 0, (LPARAM)&lvItem);

	ListView_SetItemState(this->hMainWnd->getHandleListView(), index, LVIS_FOCUSED, LVIS_FOCUSED);

	return index;
}


int App::giveNewProjectId() {
	++this->projectCounter;
	return this->projectCounter - 1;
}


int App::retrieveProjectIndexWithinContainer(int projectId) {
	int index = 0;
	for (; index < this->numberOfOpenProjects; ++index) {
		if (this->openProjects[index].first == projectId) {
			break;
		}
	}
	return index;
}
