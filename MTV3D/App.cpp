#include "stdafx.h"
#include "App.h"
#include "Resource.h"


int App::run(HINSTANCE hInstance, int nCmdShow) {
	/*LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_MTV3D, szWindowClass, MAX_LOADSTRING);*/

	this->hCurrentInst = hInstance;
	this->createWndClasses();

	/*this->hSplashWnd = std::make_unique<Window>(this->hCurrentInst, WndClassType::SPLASH);
	ShowWindow(this->hSplashWnd->getHandle(), nCmdShow);*/

	this->hMainWnd = std::make_unique<Window>(this->hCurrentInst, WndClassType::MAIN);
	ShowWindow(this->hMainWnd->getHandle(), nCmdShow);
	UpdateWindow(this->hMainWnd->getHandle());

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


void App::createWndClasses() {
	this->wndClassTypeStruct.clear();

	this->wndClassTypeStruct["Main"] = {
		sizeof(WNDCLASSEXW),
		CS_HREDRAW | CS_VREDRAW,
		Window::WndProc,
		0,
		0,
		this->hCurrentInst,
		LoadIcon(this->hCurrentInst, MAKEINTRESOURCE(IDI_MTV3D)),
		LoadCursor(this->hCurrentInst, IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		MAKEINTRESOURCEW(IDC_MTV3D),
		L"Main",
		LoadIcon(this->hCurrentInst, MAKEINTRESOURCE(IDI_MTV3D))
	};

	RegisterClassExW(&this->wndClassTypeStruct["Main"]);
}
