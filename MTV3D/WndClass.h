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
#pragma once

#include "stdafx.h"


class WndClass {
public:
	enum Type {
		SPLASH,
		MAIN,
		VIS_MERGED,
		VIS_RESULT,
		VIS_RELERR,
		VIS_DISPLAY,
		VIS_LEGEND,
		EDITABLE,
		OTHER
	};

	static Type typeByWndClassName(const std::wstring& wndClassName);
	static Type typeByWndHandle(HWND hWnd);
};
