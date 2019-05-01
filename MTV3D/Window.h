#pragma once

#include "stdafx.h"
#include "WndClassType.h"


class Window {
private:
	HWND hWnd;
public:
	Window(HINSTANCE hInst, WndClassType wndClassType);
	Window(HINSTANCE hInst, WndClassType wndClassType, std::string& filename);
	~Window() = default;

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	HWND getHandle();
};
