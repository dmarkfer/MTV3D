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
#include "WndClass.h"


WndClass::Type WndClass::typeByWndClassName(const std::wstring& wndClassName) {
	if (wndClassName == L"Splash") return WndClass::Type::SPLASH;
	if (wndClassName == L"Main") return WndClass::Type::MAIN;
	if (wndClassName == L"VisMerged") return WndClass::Type::VIS_MERGED;
	if (wndClassName == L"VisMergedPlane") return WndClass::Type::VIS_MERGED_PLANE;
	if (wndClassName == L"VisLine") return WndClass::Type::VIS_LINE;
	if (wndClassName == L"VisResult") return WndClass::Type::VIS_RESULT;
	if (wndClassName == L"VisRelErr") return WndClass::Type::VIS_RELERR;
	if (wndClassName == L"VisDisplay") return WndClass::Type::VIS_DISPLAY;
	if (wndClassName == L"DataWndClass") return WndClass::Type::DATA_WND_CLASS;
	if (wndClassName == L"VisLegend") return WndClass::Type::VIS_LEGEND;
	if (wndClassName == L"ElevScalTypeGrp") return WndClass::Type::ELEV_SCAL_TYPE_GRP;
	if (wndClassName == L"Editable") return WndClass::Type::EDITABLE;
	if (wndClassName == L"About") return WndClass::Type::ABOUT;
	return WndClass::Type::OTHER;
}


WndClass::Type WndClass::typeByWndHandle(HWND hWnd) {
	LPTSTR lpWndClassName = new wchar_t[WCHAR_ARR_MAX];
	GetClassName(hWnd, lpWndClassName, WCHAR_ARR_MAX);
	WndClass::Type wcType = WndClass::typeByWndClassName(std::wstring(lpWndClassName));
	delete[] lpWndClassName;

	return wcType;
}
