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
#include "SplashWindow.h"


SplashWindow::SplashWindow(HINSTANCE hInst) {
	RECT primaryMonitor;
	HWND hWndPrimaryDesktop = GetDesktopWindow();
	GetWindowRect(hWndPrimaryDesktop, &primaryMonitor);
	
	this->hWnd = CreateWindowEx(WS_EX_TOOLWINDOW, L"Splash", nullptr, 0,
		primaryMonitor.right/2-SPLASH_DIM, (primaryMonitor.bottom-SPLASH_DIM)/2, 2*SPLASH_DIM, SPLASH_DIM,
		nullptr, nullptr, hInst, nullptr);
	
	LONG style = GetWindowLong(this->hWnd, GWL_STYLE);
	style &= ~(WS_BORDER | WS_CAPTION | WS_SYSMENU);
	SetWindowLong(this->hWnd, GWL_STYLE, style);
}


void SplashWindow::loadSplash(HDC hdc) {
	HDC hMemDC = CreateCompatibleDC(hdc);

	HBITMAP hMTVBitmap = (HBITMAP)LoadImage(nullptr, L"MTV3D.bmp", IMAGE_BITMAP,
		SPLASH_DIM, SPLASH_DIM, LR_LOADFROMFILE);
	SelectObject(hMemDC, hMTVBitmap);
	BitBlt(hdc, 0, 0, SPLASH_DIM, SPLASH_DIM,
		hMemDC, 0, 0, SRCCOPY);
	DeleteObject(hMTVBitmap);

	constexpr int margin = 10;

	constexpr int unizgLogoDim = static_cast<int>(300 * 0.4f);

	HBITMAP hUNIZGBitmap = (HBITMAP)LoadImage(nullptr, L"UniZg_logo.bmp", IMAGE_BITMAP,
		unizgLogoDim, unizgLogoDim, LR_LOADFROMFILE);
	SelectObject(hMemDC, hUNIZGBitmap);
	BitBlt(hdc, SPLASH_DIM, 6 * margin, unizgLogoDim, unizgLogoDim,
		hMemDC, 0, 0, SRCCOPY);
	DeleteObject(hUNIZGBitmap);

	constexpr int ferLogoDimWidth = static_cast<int>(694 * 0.25f);
	constexpr int ferLogoDimHeight = static_cast<int>(300 * 0.25f);

	HBITMAP hFERBitmap = (HBITMAP)LoadImage(nullptr, L"FER_logo.bmp", IMAGE_BITMAP,
		ferLogoDimWidth, ferLogoDimHeight, LR_LOADFROMFILE);
	SelectObject(hMemDC, hFERBitmap);
	BitBlt(hdc, SPLASH_DIM + unizgLogoDim + 4 * margin, 9 * margin, ferLogoDimWidth, ferLogoDimHeight,
		hMemDC, 0, 0, SRCCOPY);
	DeleteObject(hFERBitmap);

	DeleteDC(hMemDC);


	RECT textRect;
	HFONT hFont;
	hFont = CreateFont(16, 6, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, EASTEUROPE_CHARSET,
		OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Times New Roman"));
	SelectObject(hdc, hFont);
	SetBkColor(hdc, DARK_GRAY);
	SetTextColor(hdc, WHITE);

	SetRect(&textRect, SPLASH_DIM, 7 * margin + unizgLogoDim, SPLASH_DIM + unizgLogoDim, 7 * margin + unizgLogoDim + 20);
	DrawText(hdc, TEXT("University of Zagreb"), -1, &textRect, DT_NOCLIP | DT_CENTER | DT_VCENTER);

	SetRect(&textRect, SPLASH_DIM + unizgLogoDim + 4 * margin, 7 * margin + unizgLogoDim, SPLASH_DIM + unizgLogoDim + 4 * margin + ferLogoDimWidth, 7 * margin + unizgLogoDim + 40);
	DrawText(hdc, TEXT("Faculty of electrical engineering\n and computing"), -1, &textRect, DT_NOCLIP | DT_CENTER | DT_VCENTER);

	SetRect(&textRect, SPLASH_DIM, 15 * margin + unizgLogoDim, 2 * SPLASH_DIM - 6 * margin, 15 * margin + unizgLogoDim + 40);
	DrawText(hdc, TEXT("Software developed by Domagoj Markota as Bachelor's\nThesis project (mentor: docent Mario Matijević, Ph.D.)."), -1, &textRect, DT_NOCLIP | DT_CENTER | DT_VCENTER);
}


HWND SplashWindow::getHandle() {
	return this->hWnd;
}
