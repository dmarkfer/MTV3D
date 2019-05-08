#include "stdafx.h"
#include "MainWindow.h"


MainWindow::MainWindow(HINSTANCE hInst) {
	this->hWnd = CreateWindowW(L"Main", L"Mesh Tally Visualization in 3D", WS_OVERLAPPEDWINDOW | WS_MAXIMIZE,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInst, nullptr);

	GetClientRect(this->hWnd, &wndRect);

	this->hBtnNewProj = CreateWindow(L"BUTTON", L"New project",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		wndRect.right - 300, 300, 150, 30,
		this->hWnd, (HMENU) BUTTON_NEW_PROJ, hInst, nullptr);

	this->hBtnCloseSel = CreateWindow(L"BUTTON", L"Close selected",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | WS_DISABLED,
		wndRect.right - 300, 350, 150, 30,
		this->hWnd, (HMENU) BUTTON_CLOSE_SEL, hInst, nullptr);
	
	this->hBtnCloseAll = CreateWindow(L"BUTTON", L"Close all",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | WS_DISABLED,
		wndRect.right - 300, 400, 150, 30,
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
	MoveWindow(this->hWndListView, 100, 180, this->wndRect.right - 500, this->wndRect.bottom - 300, TRUE);
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
	HFONT hFont = CreateFont(30, 12, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, EASTEUROPE_CHARSET,
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


HWND MainWindow::getHandleListView() {
	return this->hWndListView;
}
