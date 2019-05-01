#include "stdafx.h"
#include "App.h"


int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow
) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	try {
		return std::make_unique<App>()->run(hInstance, nCmdShow);
	}
	catch (...) {
		return -1;
	}

	return 0;
}
