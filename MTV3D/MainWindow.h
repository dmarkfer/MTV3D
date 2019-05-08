#pragma once


class MainWindow {
private:
	HWND hWnd;
	RECT wndRect;

	HWND hBtnNewProj;
	HWND hBtnCloseSel;
	HWND hBtnCloseAll;
	HWND hWndListView;

	HWND hWndListOfProjects;
	HWND hWndProjectDetails;
public:
	MainWindow() = default;
	MainWindow(HINSTANCE hInst);
	~MainWindow() = default;

	void loadLogo(HDC hdc);
	void initListView(HINSTANCE hInst);
	void resizeListView();

	HWND getHandle();
	HWND getHandleListView();
};
