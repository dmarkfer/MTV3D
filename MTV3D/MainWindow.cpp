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
#include "MainWindow.h"


MainWindow::MainWindow(HINSTANCE hInst) {
	SetCursor(LoadCursor(nullptr, IDC_ARROW));

	this->hWnd = CreateWindowW(L"Main", L"Mesh Tally Visualization in 3D", WS_OVERLAPPEDWINDOW | WS_MAXIMIZE,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInst, nullptr);

	GetClientRect(this->hWnd, &wndRect);

	this->hBtnNewProj = CreateWindow(L"BUTTON", L"New project",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		int(0.8 * wndRect.right), int(0.45f * wndRect.bottom), 150, 30,
		this->hWnd, (HMENU) BUTTON_NEW_PROJ, hInst, nullptr);

	this->hBtnCloseSel = CreateWindow(L"BUTTON", L"Close selected",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | WS_DISABLED,
		int(0.8 * wndRect.right), int(0.45f * wndRect.bottom) + 50, 150, 30,
		this->hWnd, (HMENU) BUTTON_CLOSE_SEL, hInst, nullptr);
	
	this->hBtnCloseAll = CreateWindow(L"BUTTON", L"Close all",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | WS_DISABLED,
		int(0.8 * wndRect.right), int(0.45f * wndRect.bottom) + 100, 150, 30,
		this->hWnd, (HMENU) BUTTON_CLOSE_ALL, hInst, nullptr);
}

void MainWindow::initListView(HINSTANCE hInst) {
	InitCommonControls();
	
	this->hWndListView = CreateWindowEx(WS_EX_CLIENTEDGE | LVS_EX_FULLROWSELECT, WC_LISTVIEW, nullptr,
		WS_TABSTOP | WS_CHILD | WS_BORDER | WS_VISIBLE | LVS_AUTOARRANGE | LVS_REPORT | LVS_OWNERDATA,
		0, 0, 0, 0,
		this->hWnd, (HMENU)ID_LISTVIEW, hInst, nullptr);
	SendMessage(hWndListView, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	this->resizeListView();


	WCHAR columns[4][10] = { L"Filename", L"Size", L"Created", L"Modified" };
	RECT lwRect;
	GetClientRect(hWndListView, &lwRect);
	LV_COLUMN lvColumn;
	ZeroMemory(&lvColumn, sizeof(LVCOLUMN));

	lvColumn.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvColumn.cx = lwRect.right / 2;
	lvColumn.pszText = columns[0];
	ListView_InsertColumn(this->hWndListView, 0, &lvColumn);

	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvColumn.fmt = LVCFMT_CENTER;
	lvColumn.cx = lwRect.right / 6;
	lvColumn.pszText = columns[1];
	ListView_InsertColumn(this->hWndListView, 1, &lvColumn);

	lvColumn.pszText = columns[2];
	ListView_InsertColumn(this->hWndListView, 2, &lvColumn);

	lvColumn.pszText = columns[3];
	ListView_InsertColumn(this->hWndListView, 3, &lvColumn);
	
	ListView_DeleteAllItems(this->hWndListView);
}


void MainWindow::resizeListView() {
	GetClientRect(this->hWnd, &wndRect);
	MoveWindow(this->hWndListView, int(0.07f * wndRect.right), int(0.25f * wndRect.bottom), int(0.65f * this->wndRect.right), int(0.6f * this->wndRect.bottom), TRUE);
}


void MainWindow::resizeButtons() {
	MoveWindow(this->hBtnNewProj, int(0.8 * wndRect.right), int(0.45f * wndRect.bottom), 150, 30, TRUE);
	MoveWindow(this->hBtnCloseSel, int(0.8 * wndRect.right), int(0.45f * wndRect.bottom) + 50, 150, 30, TRUE);
	MoveWindow(this->hBtnCloseAll, int(0.8 * wndRect.right), int(0.45f * wndRect.bottom) + 100, 150, 30, TRUE);
}


void MainWindow::loadLogo(HDC hdc, LPWSTR rootDir, float qLogo, int fontHeight, int fontWidth) {
	HDC hMemDC = CreateCompatibleDC(hdc);

	constexpr int margin = 10;

	int mtvLogoDim = static_cast<int>(SPLASH_DIM * qLogo);

	HBITMAP hMTVBitmap = (HBITMAP)LoadImage(nullptr, (std::wstring(rootDir) + L"\\MTV3D.bmp").c_str(), IMAGE_BITMAP,
		mtvLogoDim, mtvLogoDim, LR_LOADFROMFILE);
	SelectObject(hMemDC, hMTVBitmap);
	BitBlt(hdc, 2 * margin, 2 * margin, mtvLogoDim, mtvLogoDim,
		hMemDC, 0, 0, SRCCOPY);
	DeleteObject(hMTVBitmap);

	DeleteDC(hMemDC);


	RECT textRect;
	HFONT hFont = CreateFont(fontHeight, fontWidth, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, EASTEUROPE_CHARSET,
		OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Arial"));
	SelectObject(hdc, hFont);
	SetBkColor(hdc, DARK_GRAY);
	SetTextColor(hdc, THEME_BLUE);

	SetRect(&textRect, mtvLogoDim + 2 * margin, 3 * margin, fontWidth * 35, int(fontHeight * 1.5f * margin + 35));
	DrawText(hdc, TEXT("Mesh Tally Visualization\nin 3D"), -1, &textRect, DT_NOCLIP | DT_CENTER | DT_VCENTER);
}


void MainWindow::deleteListViewItem(int index) {
	if (index == -1) {
		SendMessage(this->hWndListView, LVM_DELETEALLITEMS, 0, 0);
	}
	else {
		SendMessage(this->hWndListView, LVM_DELETEITEM, index, 0);
	}
}


HWND MainWindow::getHandle() {
	return this->hWnd;
}


HWND MainWindow::getHandleBtnNewProj() {
	return this->hBtnNewProj;
}


HWND MainWindow::getHandleBtnCloseSel() {
	return this->hBtnCloseSel;
}


HWND MainWindow::getHandleBtnCloseAll() {
	return this->hBtnCloseAll;
}


HWND MainWindow::getHandleListView() {
	return this->hWndListView;
}
