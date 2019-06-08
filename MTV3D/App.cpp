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


App::App() {
	App::appPointer = this;

	this->projectCounter = 0;
	this->numberOfOpenProjects = 0;

	VisComponent::mainThreadId = GetCurrentThreadId();
}


App::~App() {
	delete[] Graphics::vertexShaderBlob;
	delete[] Graphics::pixelShaderBlob;
}


int App::run(HINSTANCE hInstance, int& nCmdShow) {
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

	this->hCurrentInst = hInstance;

	CursorData::cursorHandNoGrab = LoadCursorFromFile(L"Hand_Move_No_Grab_v2.cur");
	CursorData::cursorHandGrab = LoadCursorFromFile(L"Hand_Move_Grab_v2.cur");

	this->readD3DShaders();

	this->createWndClasses();

	this->hSplashWnd = std::make_unique<SplashWindow>(this->hCurrentInst);
	ShowWindow(this->hSplashWnd->getHandle(), nCmdShow);
	SetTimer(this->hSplashWnd->getHandle(), IDT_TIMER_SPLASH, SPLASH_TTL, nullptr);

	this->hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MTV3D));
	MSG msg;

	while (GetMessage(&msg, nullptr, 0, 0)) {
		if (!TranslateAccelerator(msg.hwnd, this->hAccelTable, &msg)) {
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

	Gdiplus::GdiplusShutdown(gdiplusToken);

	return (int)msg.wParam;
}


void App::readD3DShaders() {
	std::ifstream ifStreamShader(L"VertexShader.cso", std::ifstream::in | std::ifstream::binary);
	ifStreamShader.seekg(0, std::ios::end);
	Graphics::vertexShaderFileSize = (unsigned)ifStreamShader.tellg();
	Graphics::vertexShaderBlob = new char[Graphics::vertexShaderFileSize];
	ifStreamShader.seekg(0, std::ios::beg);
	ifStreamShader.read(Graphics::vertexShaderBlob, Graphics::vertexShaderFileSize);
	ifStreamShader.close();

	ifStreamShader.open(L"PixelShader.cso", std::ifstream::in | std::ifstream::binary);
	ifStreamShader.seekg(0, std::ios::end);
	Graphics::pixelShaderFileSize = (unsigned)ifStreamShader.tellg();
	Graphics::pixelShaderBlob = new char[Graphics::pixelShaderFileSize];
	ifStreamShader.seekg(0, std::ios::beg);
	ifStreamShader.read(Graphics::pixelShaderBlob, Graphics::pixelShaderFileSize);
	ifStreamShader.close();
}


LRESULT CALLBACK App::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	WndClass::Type wcType = WndClass::typeByWndHandle(hWnd);

	PAINTSTRUCT ps;
	HDC hdc;

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
	case WM_SETCURSOR: {
		if (wcType == WndClass::Type::VIS_DISPLAY) {
			SetCursor(CursorData::cursorGrabInteractionProject == hWnd ? CursorData::cursorHandGrab : CursorData::cursorHandNoGrab);
		}
		else if (wcType == WndClass::Type::EDITABLE) {
			SetCursor(LoadCursor(nullptr, IDC_IBEAM));
		}
		else {
			CursorData::cursorGrabInteractionProject = nullptr;
			SetCursor(LoadCursor(nullptr, IDC_ARROW));
		}
		return true;
		break;
	}
	case WM_LBUTTONDOWN: {
		if (wcType == WndClass::Type::VIS_DISPLAY) {
			CursorData::cursorGrabInteractionProject = hWnd;
			SetCursor(CursorData::cursorHandGrab);
			CursorData::clickPosX = GET_X_LPARAM(lParam);
			CursorData::clickPosY = GET_Y_LPARAM(lParam);
		}
		break;
	}
	case WM_LBUTTONUP: {
		if (wcType == WndClass::Type::VIS_DISPLAY) {
			CursorData::cursorGrabInteractionProject = nullptr;
			SetCursor(CursorData::cursorHandNoGrab);
		}
		break;
	}
	case WM_COMMAND: {
		switch (LOWORD(wParam)) {
		case BUTTON_LINK: {
			system("start www.github.com/dmarkfer/MTV3D");
			break;
		}
		case BUTTON_LINK_LIC: {
			system("start www.gnu.org/licenses/agpl-3.0.en.html");
			break;
		}
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

				if (!projectLoaded) {
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

			while (true) {
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
		case BUTTON_CREATE_LINE: {
			PlanePreview::flagLinePrevCreation = true;
			break;
		}
		case BUTTON_CREATE_PLANE: {
			VisComponent::flagPlanePrevCreation = true;
			break;
		}
		case CHECK_GRID: {
			if (HIWORD(wParam) == BN_CLICKED) {
				if (SendDlgItemMessage(hWnd, CHECK_GRID, BM_GETCHECK, 0, 0)) {
					VisComponent::gridActive = true;
					PlanePreview::gridActive = true;
				}
				else {
					VisComponent::gridActive = false;
					PlanePreview::gridActive = false;
					CheckDlgButton(hWnd, CHECK_AX_VAL, BST_UNCHECKED);
					VisComponent::axesValsActive = false;
					PlanePreview::axesValsActive = false;
				}
			}
			break;
		}
		case CHECK_AX_VAL: {
			if (HIWORD(wParam) == BN_CLICKED) {
				if (SendDlgItemMessage(hWnd, CHECK_AX_VAL, BM_GETCHECK, 0, 0)) {
					VisComponent::axesValsActive = true;
					PlanePreview::axesValsActive = true;
					CheckDlgButton(hWnd, CHECK_GRID, BST_CHECKED);
					VisComponent::gridActive = true;
					PlanePreview::gridActive = true;
				}
				else {
					VisComponent::axesValsActive = false;
					PlanePreview::axesValsActive = false;
				}
			}
			break;
		}
		case IDM_CLOSE:
		case IDM_EXIT: {
			DestroyWindow(hWnd);
			break;
		}
		case IDM_DOC: {
			RECT rect;
			HWND hWndPrimaryDesktop = GetDesktopWindow();
			GetWindowRect(hWndPrimaryDesktop, &rect);

			HWND docWnd = CreateWindow(L"About", L"Documentation", WS_VISIBLE | WS_OVERLAPPEDWINDOW,
				rect.right / 2 - 250, rect.bottom / 2 - 150, 500, 300,
				nullptr, nullptr, App::appPointer->hCurrentInst, nullptr);
			LONG style = GetWindowLong(docWnd, GWL_STYLE);
			style &= ~(WS_MAXIMIZEBOX | WS_MINIMIZEBOX);
			SetWindowLong(docWnd, GWL_STYLE, style);

			hdc = BeginPaint(docWnd, &ps);

			App::appPointer->hMainWnd->loadLogo(hdc, 0.15f, 18, 9);

			EndPaint(docWnd, &ps);
			DeleteDC(hdc);

			CreateWindow(L"STATIC", L"Software documentation available at:", WS_VISIBLE | WS_CHILD,
				30, 120, 250, 20,
				docWnd, nullptr, App::appPointer->hCurrentInst, nullptr);
			CreateWindow(L"STATIC", L"github.com/dmarkfer/MTV3D", WS_VISIBLE | WS_CHILD,
				200, 150, 200, 20,
				docWnd, nullptr, App::appPointer->hCurrentInst, nullptr);

			CreateWindow(L"BUTTON", L"Project repository",
				WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
				220, 190, 180, 25,
				docWnd, (HMENU)BUTTON_LINK, App::appPointer->hCurrentInst, nullptr);

			break;
		}
		case IDM_ABOUT: {
			RECT rect;
			HWND hWndPrimaryDesktop = GetDesktopWindow();
			GetWindowRect(hWndPrimaryDesktop, &rect);

			HWND docWnd = CreateWindow(L"About", L"About MTV3D", WS_VISIBLE | WS_OVERLAPPEDWINDOW,
				rect.right / 2 - 400, rect.bottom / 2 - 250, 800, 500,
				nullptr, nullptr, App::appPointer->hCurrentInst, nullptr);
			LONG style = GetWindowLong(docWnd, GWL_STYLE);
			style &= ~(WS_MAXIMIZEBOX | WS_MINIMIZEBOX);
			SetWindowLong(docWnd, GWL_STYLE, style);

			hdc = BeginPaint(docWnd, &ps);
			HDC hMemDC = CreateCompatibleDC(hdc);

			constexpr int margin = 10;
			constexpr int mtvLogoDim = static_cast<int>(SPLASH_DIM * 0.4f);

			HBITMAP hMTVBitmap = (HBITMAP)LoadImage(nullptr, L"MTV3D.bmp", IMAGE_BITMAP,
				mtvLogoDim, mtvLogoDim, LR_LOADFROMFILE);
			SelectObject(hMemDC, hMTVBitmap);
			BitBlt(hdc, 50, 2 * margin, mtvLogoDim, mtvLogoDim,
				hMemDC, 0, 0, SRCCOPY);
			DeleteObject(hMTVBitmap);


			constexpr int unizgLogoDim = static_cast<int>(300 * 0.4f * 0.8f);

			HBITMAP hUNIZGBitmap = (HBITMAP)LoadImage(nullptr, L"UniZg_logo.bmp", IMAGE_BITMAP,
				unizgLogoDim, unizgLogoDim, LR_LOADFROMFILE);
			SelectObject(hMemDC, hUNIZGBitmap);
			BitBlt(hdc, 300, 4 * margin, unizgLogoDim, unizgLogoDim,
				hMemDC, 0, 0, SRCCOPY);
			DeleteObject(hUNIZGBitmap);

			constexpr int ferLogoDimWidth = static_cast<int>(694 * 0.25f * 0.8f);
			constexpr int ferLogoDimHeight = static_cast<int>(300 * 0.25f * 0.8f);

			HBITMAP hFERBitmap = (HBITMAP)LoadImage(nullptr, L"FER_logo.bmp", IMAGE_BITMAP,
				ferLogoDimWidth, ferLogoDimHeight, LR_LOADFROMFILE);
			SelectObject(hMemDC, hFERBitmap);
			BitBlt(hdc, 300 + unizgLogoDim + 12 * margin, 6 * margin, ferLogoDimWidth, ferLogoDimHeight,
				hMemDC, 0, 0, SRCCOPY);
			DeleteObject(hFERBitmap);

			DeleteDC(hMemDC);


			EndPaint(docWnd, &ps);
			DeleteDC(hdc);


			CreateWindow(L"STATIC", L"University of Zagreb", WS_VISIBLE | WS_CHILD,
				285, 150, 200, 20,
				docWnd, nullptr, App::appPointer->hCurrentInst, nullptr);
			CreateWindow(L"STATIC", L"Faculty of electrical engineering", WS_VISIBLE | WS_CHILD,
				485, 150, 300, 20,
				docWnd, nullptr, App::appPointer->hCurrentInst, nullptr);
			CreateWindow(L"STATIC", L"and computing", WS_VISIBLE | WS_CHILD,
				535, 165, 200, 20,
				docWnd, nullptr, App::appPointer->hCurrentInst, nullptr);


			CreateWindow(L"STATIC", L"Software developed by Domagoj Markota as Bachelor's Thesis project (mentor: docent Mario Matijević, Ph.D.).\n\nSoftware is published under GNU Affero General Public License v 3 .", WS_VISIBLE | WS_CHILD,
				100, 220, 500, 80,
				docWnd, nullptr, App::appPointer->hCurrentInst, nullptr);


			CreateWindow(L"STATIC", L"Project available at:", WS_VISIBLE | WS_CHILD,
				300, 320, 250, 20,
				docWnd, nullptr, App::appPointer->hCurrentInst, nullptr);
			CreateWindow(L"STATIC", L"github.com/dmarkfer/MTV3D", WS_VISIBLE | WS_CHILD,
				400, 340, 200, 20,
				docWnd, nullptr, App::appPointer->hCurrentInst, nullptr);

			CreateWindow(L"BUTTON", L"Project repository",
				WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
				420, 380, 180, 25,
				docWnd, (HMENU)BUTTON_LINK, App::appPointer->hCurrentInst, nullptr);

			CreateWindow(L"BUTTON", L"GNU Affero GPL v3",
				WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
				180, 380, 180, 25,
				docWnd, (HMENU)BUTTON_LINK_LIC, App::appPointer->hCurrentInst, nullptr);

			break;
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

			App::appPointer->hMainWnd->loadLogo(hdc, 0.2f, 30, 12);

			EndPaint(hWnd, &ps);
			DeleteDC(hdc);
			break;
		}
		case WndClass::Type::VIS_LINE: {
			LinePreviewWnd::drawChart(&ps, hWnd);
			break;
		}
		case WndClass::Type::VIS_LEGEND: {
			break;
		}
		case WndClass::Type::ABOUT: {
			break;
		}
		}
		break;
	}
	case WM_CTLCOLORSTATIC: {
		SetTextColor((HDC)wParam, WHITE);
		SetBkColor((HDC)wParam, DARK_GRAY);

		return (BOOL)GetStockObject(NULL_BRUSH);
		break;
	}
	case WM_MOUSEWHEEL: {
		WORD fwKeys = GET_KEYSTATE_WPARAM(wParam);
		WORD zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		WORD xPos = GET_X_LPARAM(lParam);
		WORD yPos = GET_Y_LPARAM(lParam);

		// 2^16 = 65536
		if (zDelta <= 32000) { //   0  to  2^16 / 2
			VisComponent::scaleBase *= 1.05f;
			PlanePreview::scaleBase *= 1.05f;
		}
		else { //   2^16 / 2  to  2^16
			VisComponent::scaleBase *= 0.95f;
			PlanePreview::scaleBase *= 0.95f;
		}

		return 0;
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
		case WndClass::Type::VIS_MERGED: {
			PostQuitMessage(0);
			break;
		}
		case WndClass::Type::VIS_MERGED_PLANE: {
			PostQuitMessage(0);
			break;
		}
		}
	}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
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
		App::wndProc,
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

	this->wndClassTypeStruct[WndClass::Type::VIS_MERGED_PLANE] = {
		sizeof(WNDCLASSEXW),
		CS_HREDRAW | CS_VREDRAW,
		App::wndProc,
		0,
		0,
		this->hCurrentInst,
		LoadIcon(this->hCurrentInst, MAKEINTRESOURCE(IDI_MTV3D)),
		LoadCursor(this->hCurrentInst, IDC_ARROW),
		CreateSolidBrush(DARK_GRAY),
		MAKEINTRESOURCEW(IDC_MTV3D_VIS),
		L"VisMergedPlane",
		LoadIcon(this->hCurrentInst, MAKEINTRESOURCE(IDI_MTV3D))
	};

	this->wndClassTypeStruct[WndClass::Type::VIS_LINE] = {
		sizeof(WNDCLASSEXW),
		CS_HREDRAW | CS_VREDRAW,
		App::wndProc,
		0,
		0,
		this->hCurrentInst,
		LoadIcon(this->hCurrentInst, MAKEINTRESOURCE(IDI_MTV3D)),
		LoadCursor(this->hCurrentInst, IDC_ARROW),
		CreateSolidBrush(DARK_GRAY),
		MAKEINTRESOURCEW(IDC_MTV3D_VIS),
		L"VisLine",
		LoadIcon(this->hCurrentInst, MAKEINTRESOURCE(IDI_MTV3D))
	};

	this->wndClassTypeStruct[WndClass::Type::VIS_RESULT] = {
		sizeof(WNDCLASSEXW),
		CS_HREDRAW | CS_VREDRAW,
		App::wndProc,
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
		App::wndProc,
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
		App::wndProc,
		0,
		0,
		this->hCurrentInst,
		nullptr,
		CursorData::cursorHandNoGrab,
		CreateSolidBrush(WHITE),
		nullptr,
		L"VisDisplay",
		nullptr
	};

	this->wndClassTypeStruct[WndClass::Type::VIS_LEGEND] = {
		sizeof(WNDCLASSEXW),
		CS_HREDRAW | CS_VREDRAW,
		App::wndProc,
		0,
		0,
		this->hCurrentInst,
		nullptr,
		LoadCursor(this->hCurrentInst, IDC_ARROW),
		CreateSolidBrush(WHITE),
		nullptr,
		L"VisLegend",
		nullptr
	};

	this->wndClassTypeStruct[WndClass::Type::EDITABLE] = {
		sizeof(WNDCLASSEXW),
		CS_HREDRAW | CS_VREDRAW,
		App::wndProc,
		0,
		0,
		this->hCurrentInst,
		nullptr,
		LoadCursor(this->hCurrentInst, IDC_IBEAM),
		CreateSolidBrush(WHITE),
		nullptr,
		L"Editable",
		nullptr
	};

	this->wndClassTypeStruct[WndClass::Type::ABOUT] = {
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
		L"About",
		LoadIcon(this->hCurrentInst, MAKEINTRESOURCE(IDI_MTV3D))
	};


	RegisterClassExW(&this->wndClassTypeStruct[WndClass::Type::SPLASH]);
	RegisterClassExW(&this->wndClassTypeStruct[WndClass::Type::MAIN]);
	RegisterClassExW(&this->wndClassTypeStruct[WndClass::Type::VIS_MERGED]);
	RegisterClassExW(&this->wndClassTypeStruct[WndClass::Type::VIS_MERGED_PLANE]);
	RegisterClassExW(&this->wndClassTypeStruct[WndClass::Type::VIS_LINE]);
	RegisterClassExW(&this->wndClassTypeStruct[WndClass::Type::VIS_RESULT]);
	RegisterClassExW(&this->wndClassTypeStruct[WndClass::Type::VIS_RELERR]);
	RegisterClassExW(&this->wndClassTypeStruct[WndClass::Type::VIS_DISPLAY]);
	RegisterClassExW(&this->wndClassTypeStruct[WndClass::Type::VIS_LEGEND]);
	RegisterClassExW(&this->wndClassTypeStruct[WndClass::Type::EDITABLE]);
	RegisterClassExW(&this->wndClassTypeStruct[WndClass::Type::ABOUT]);
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
	if (!GetFileInformationByHandle(hFile, &fileInfo)) {
		CloseHandle(hFile);
		return false;
	}


	LPCH fileRawContent = new char[fileSize + 1];
	DWORD bytesRead;

	BOOL retVal = ReadFile(hFile, fileRawContent, fileSize, &bytesRead, nullptr);
	CloseHandle(hFile);

	if (!retVal) {
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
			else {
				break;
			}
		}
	}


	std::vector<Graphics::Point3D> visPoints;
	Graphics::Point3D point;
	LPCWSTR fileLineWCStr;

	if (isPhoton) {
		while (std::getline(fileContentStringStream, fileLine)) {
			fileLineWCStr = fileLine.c_str();
			swscanf_s(fileLineWCStr, L"%*s %f %f %f %lf %lf", &point.x, &point.y, &point.z, &point.value, &point.relError);
			visPoints.push_back(point);
		}
	}
	else {
		while (std::getline(fileContentStringStream, fileLine)) {
			fileLineWCStr = fileLine.c_str();
			swscanf_s(fileLineWCStr, L"%f %f %f %lf %lf", &point.x, &point.y, &point.z, &point.value, &point.relError);
			visPoints.push_back(point);
		}
	}

	int visPointsDataSize = visPoints.size();
	Graphics::Point3D* visPointsData = new Graphics::Point3D[visPointsDataSize];
	memcpy(visPointsData, visPoints.data(), visPointsDataSize * sizeof(Graphics::Point3D));


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

	std::vector<LPWSTR> lvData{ fileAbsolutePath, fileSizeStr, fileCreatedStr, fileModifiedStr };
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
				this->hCurrentInst, this->hAccelTable, newProjectId, fileAbsolutePath, visPointsDataSize, visPointsData
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

	if (!this->numberOfOpenProjects) {
		EnableWindow(this->hMainWnd->getHandleBtnCloseAll(), FALSE);
	}

	for (LPWSTR fiEl : this->openProjects[index].second.first) {
		delete[] fiEl;
	}
	this->openProjects.erase(this->openProjects.begin() + index);


	this->recreateListView();

	if (App::quitFlag) {
		if (!this->numberOfOpenProjects) {
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
