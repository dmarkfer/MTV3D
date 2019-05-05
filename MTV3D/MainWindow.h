#pragma once


class MainWindow {
private:
	HWND hWnd;

	HWND hBtnNewProj;
	HWND hBtnCloseProj;
	HWND hBtnCloseAll;
public:
	MainWindow(HINSTANCE hInst);
	~MainWindow() = default;

	void loadLogo(HDC hdc);

	HWND getHandle();
};
