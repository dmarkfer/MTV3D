#pragma once

#include "stdafx.h"
#include "WndClass.h"
#include "VisMergedWindow.h"


class VisComponent {
public:
	struct Point {
		double x, y, z;
		long double value, relError;
	};
private:
	HINSTANCE hCurrentInst;
	static std::mutex mtx;

	LPWSTR fileAbsolutePath;
	std::map<WndClass::Type, WNDCLASSEXW> wndClassTypeStruct;
	VisMergedWindow hVisMerWnd;
public:
	VisComponent() = default;
	~VisComponent() = default;

	void run(HINSTANCE hCurrentInst, LPWSTR fileAbsolutePath);
private:
	static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void createWndClasses();
};
