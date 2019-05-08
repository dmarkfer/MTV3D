#include "stdafx.h"
#include "VisComponent.h"


void VisComponent::run(HINSTANCE hCurrentInst, LPWSTR fileAbsolutePath) {
}

LRESULT CALLBACK VisComponent::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	return 0;
}


void VisComponent::createWndClasses() {
	this->wndClassTypeStruct.clear();

	this->wndClassTypeStruct[WndClass::Type::VIS_MERGED] = {
		sizeof(WNDCLASSEXW),
		CS_HREDRAW | CS_VREDRAW,
		VisComponent::wndProc,
		0,
		0,
		this->hCurrentInst,
		LoadIcon(this->hCurrentInst, MAKEINTRESOURCE(IDI_MTV3D)),
		LoadCursor(this->hCurrentInst, IDC_ARROW),
		CreateSolidBrush(DARK_GRAY),
		MAKEINTRESOURCEW(IDC_MTV3D),
		L"VisMerged",
		LoadIcon(this->hCurrentInst, MAKEINTRESOURCE(IDI_MTV3D))
	};

	this->wndClassTypeStruct[WndClass::Type::VIS_RESULT] = {
		sizeof(WNDCLASSEXW),
		CS_HREDRAW | CS_VREDRAW,
		VisComponent::wndProc,
		0,
		0,
		this->hCurrentInst,
		LoadIcon(this->hCurrentInst, MAKEINTRESOURCE(IDI_MTV3D)),
		LoadCursor(this->hCurrentInst, IDC_ARROW),
		CreateSolidBrush(DARK_GRAY),
		MAKEINTRESOURCEW(IDC_MTV3D),
		L"VisResult",
		LoadIcon(this->hCurrentInst, MAKEINTRESOURCE(IDI_MTV3D))
	};

	this->wndClassTypeStruct[WndClass::Type::VIS_RELERR] = {
		sizeof(WNDCLASSEXW),
		CS_HREDRAW | CS_VREDRAW,
		VisComponent::wndProc,
		0,
		0,
		this->hCurrentInst,
		LoadIcon(this->hCurrentInst, MAKEINTRESOURCE(IDI_MTV3D)),
		LoadCursor(this->hCurrentInst, IDC_ARROW),
		CreateSolidBrush(DARK_GRAY),
		MAKEINTRESOURCEW(IDC_MTV3D),
		L"VisRelErr",
		LoadIcon(this->hCurrentInst, MAKEINTRESOURCE(IDI_MTV3D))
	};


	RegisterClassExW(&this->wndClassTypeStruct[WndClass::Type::VIS_MERGED]);
	RegisterClassExW(&this->wndClassTypeStruct[WndClass::Type::VIS_RESULT]);
	RegisterClassExW(&this->wndClassTypeStruct[WndClass::Type::VIS_RELERR]);
}
