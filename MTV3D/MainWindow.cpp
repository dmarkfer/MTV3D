#include "stdafx.h"
#include "MainWindow.h"


MainWindow::MainWindow(HINSTANCE hInst) {
	this->hWnd = CreateWindowW(L"Main", L"Mesh Tally Visualization in 3D", WS_OVERLAPPEDWINDOW | WS_MAXIMIZE,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInst, nullptr);

	this->hBtnNewProj = CreateWindow(L"BUTTON", L"New project",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		200, 300, 150, 30,
		this->hWnd, nullptr, hInst, nullptr);
	this->hBtnCloseProj = CreateWindow(L"BUTTON", L"Close project",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		200, 350, 150, 30,
		this->hWnd, nullptr, hInst, nullptr);
	this->hBtnCloseAll = CreateWindow(L"BUTTON", L"Close all",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		200, 400, 150, 30,
		this->hWnd, nullptr, hInst, nullptr);
}


void MainWindow::loadLogo(HDC hdc) {
	HDC hMemDC = CreateCompatibleDC(hdc);

	constexpr int margin = 10;

	constexpr int mtvLogoDim = static_cast<int>(SPLASH_DIM * 0.2f);

	HBITMAP hMTVBitmap = (HBITMAP)LoadImage(nullptr, L"MTV3D.bmp", IMAGE_BITMAP,
		mtvLogoDim, mtvLogoDim, LR_LOADFROMFILE);
	SelectObject(hMemDC, hMTVBitmap);
	BitBlt(hdc, 2 * margin, 2 * margin, mtvLogoDim, mtvLogoDim,
		hMemDC, 0, 0, SRCCOPY);
	DeleteObject(hMTVBitmap);

	DeleteDC(hMemDC);


	RECT textRect;
	HFONT hFont;
	hFont = CreateFont(30, 12, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, EASTEUROPE_CHARSET,
		OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Arial"));
	SelectObject(hdc, hFont);
	SetBkColor(hdc, DARK_GRAY);
	SetTextColor(hdc, THEME_BLUE);

	SetRect(&textRect, mtvLogoDim + 2 * margin, 3 * margin, 400, 3 * margin + 35);
	DrawText(hdc, TEXT("Mesh Tally Visualization\nin 3D"), -1, &textRect, DT_NOCLIP | DT_CENTER | DT_VCENTER);
}


HWND MainWindow::getHandle() {
	return this->hWnd;
}
