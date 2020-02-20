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
#include "LinePreviewWnd.h"


std::set<LinePreviewWnd*> LinePreviewWnd::lineWndSet;


LinePreviewWnd::LinePreviewWnd(HINSTANCE hCurrentInst, LPWSTR fileAbsolutePath, std::pair<std::pair<char, float>, std::pair<char, float>> pr, int linePointsDataSize, Graphics::Point1D* linePointsData) {
	SetCursor(LoadCursor(nullptr, IDC_ARROW));

	this->windowTitle = new WCHAR[WCHAR_ARR_MAX];
	swprintf_s(windowTitle, WCHAR_ARR_MAX - 1, L"MTV3D - %s ( %c = %1.3f , %c = %1.3f )", fileAbsolutePath, pr.first.first, pr.first.second, pr.second.first, pr.second.second);

	this->pr = pr;

	this->linePointsDataSize = linePointsDataSize;
	this->linePoints.assign(linePointsData, linePointsData + linePointsDataSize);
	delete[] linePointsData;

	this->reCreate();
}


LinePreviewWnd::~LinePreviewWnd() {
	DestroyWindow(this->hWnd);
	delete[] this->windowTitle;
}


HWND LinePreviewWnd::getHandle() {
	return this->hWnd;
}


void LinePreviewWnd::reCreate() {
	LinePreviewWnd::lineWndSet.insert(this);

	this->hWnd = CreateWindowW(L"VisLine", this->windowTitle, WS_OVERLAPPEDWINDOW | WS_MAXIMIZE,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, this->hCurrentInst, nullptr);
	ShowWindow(this->hWnd, SW_SHOW);
}


void LinePreviewWnd::drawChart(PAINTSTRUCT* ps, HWND hWnd) {
	LinePreviewWnd* lineWnd = nullptr;
	for (auto iter : LinePreviewWnd::lineWndSet) {
		if (iter->getHandle() == hWnd) {
			lineWnd = iter;
		}
	}

	if (lineWnd == nullptr) {
		return;
	}

	for (auto childWnd : lineWnd->childWnds) {
		DestroyWindow(childWnd);
	}
	lineWnd->childWnds.clear();

	HDC hdc = BeginPaint(lineWnd->getHandle(), ps);

	Gdiplus::Graphics graphics(hdc);
	Gdiplus::Pen penThick(Gdiplus::Color(255, 0, 0, 0), 5);
	Gdiplus::Pen penThin(Gdiplus::Color(255, 0, 0, 0), 1);

	RECT rect;
	GetClientRect(lineWnd->getHandle(), &rect);


	graphics.DrawLine(&penThick, rect.right / 2 - rect.right / 3, rect.bottom / 2 - rect.bottom / 3,
								 rect.right / 2 - rect.right / 3, rect.bottom / 2 + rect.bottom / 3);

	graphics.DrawLine(&penThick, rect.right / 2 + rect.right / 3, rect.bottom / 2 - rect.bottom / 3,
								 rect.right / 2 + rect.right / 3, rect.bottom / 2 + rect.bottom / 3);

	graphics.DrawLine(&penThick, rect.right / 2 - rect.right / 3 - 2, rect.bottom / 2 - rect.bottom / 3,
								 rect.right / 2 + rect.right / 3 + 3, rect.bottom / 2 - rect.bottom / 3);

	graphics.DrawLine(&penThick, rect.right / 2 - rect.right / 3 - 2, rect.bottom / 2 + rect.bottom / 3,
								 rect.right / 2 + rect.right / 3 + 3, rect.bottom / 2 + rect.bottom / 3);

	int widthTenth = int((rect.right * 2. / 3.) / 10.);
	int heightTenth = int((rect.bottom * 2. / 3.) / 10.);

	WCHAR axisValWStr[101];

	char lineAxis = 'X';

	if (lineWnd->pr.first.first == lineAxis || lineWnd->pr.second.first == lineAxis) {
		lineAxis = 'Y';

		if (lineWnd->pr.first.first == lineAxis || lineWnd->pr.second.first == lineAxis) {
			lineAxis = 'Z';
		}
	}


	swprintf_s(axisValWStr, L"Line ( %c = %1.3f , %c = %1.3f )", lineWnd->pr.first.first, lineWnd->pr.first.second, lineWnd->pr.second.first, lineWnd->pr.second.second);
	lineWnd->childWnds.push_back(CreateWindow(L"STATIC", axisValWStr, WS_VISIBLE | WS_CHILD,
		rect.right / 2 - 120, 50, 300, 20,
		lineWnd->getHandle(), nullptr, lineWnd->hCurrentInst, nullptr));

	swprintf_s(axisValWStr, L"axis %c", lineAxis);
	lineWnd->childWnds.push_back(CreateWindow(L"STATIC", axisValWStr, WS_VISIBLE | WS_CHILD,
		rect.right / 2 - rect.right / 3 + 5 * widthTenth - 30, rect.bottom / 2 + rect.bottom / 3 + 50, 120, 20,
		lineWnd->getHandle(), nullptr, lineWnd->hCurrentInst, nullptr));

	swprintf_s(axisValWStr, L"%1.3f", lineWnd->linePoints[0].axis);
	lineWnd->childWnds.push_back(CreateWindow(L"STATIC", axisValWStr, WS_VISIBLE | WS_CHILD,
		rect.right / 2 - rect.right / 3 - 30, rect.bottom / 2 + rect.bottom / 3 + 15, 120, 20,
		lineWnd->getHandle(), nullptr, lineWnd->hCurrentInst, nullptr));

	swprintf_s(axisValWStr, L"%1.3f", lineWnd->linePoints[lineWnd->linePointsDataSize - 1].axis);
	lineWnd->childWnds.push_back(CreateWindow(L"STATIC", axisValWStr, WS_VISIBLE | WS_CHILD,
		rect.right / 2 - rect.right / 3 + 10 * widthTenth - 30, rect.bottom / 2 + rect.bottom / 3 + 15, 120, 20,
		lineWnd->getHandle(), nullptr, lineWnd->hCurrentInst, nullptr));

	for (int i = 1; i <= 9; ++i) {
		swprintf_s(axisValWStr, L"%1.3f", lineWnd->linePoints[i * lineWnd->linePointsDataSize / 10].axis);
		lineWnd->childWnds.push_back(CreateWindow(L"STATIC", axisValWStr, WS_VISIBLE | WS_CHILD,
			rect.right / 2 - rect.right / 3 + i * widthTenth - 30, rect.bottom / 2 + rect.bottom / 3 + 15, 120, 20,
			lineWnd->getHandle(), nullptr, lineWnd->hCurrentInst, nullptr));

		graphics.DrawLine(&penThin, rect.right / 2 - rect.right / 3 + i * widthTenth, rect.bottom / 2 - rect.bottom / 3,
									rect.right / 2 - rect.right / 3 + i * widthTenth, rect.bottom / 2 + rect.bottom / 3);

		graphics.DrawLine(&penThin, rect.right / 2 - rect.right / 3, rect.bottom / 2 - rect.bottom / 3 + i * heightTenth,
									rect.right / 2 + rect.right / 3, rect.bottom / 2 - rect.bottom / 3 + i * heightTenth);
	}

	
	lineWnd->childWnds.push_back(CreateWindow(L"STATIC", L"Result", WS_VISIBLE | WS_CHILD,
		rect.right / 2 - rect.right / 3 - 100, rect.bottom / 2 - rect.bottom / 3 - 50, 120, 20,
		lineWnd->getHandle(), nullptr, lineWnd->hCurrentInst, nullptr));

	lineWnd->childWnds.push_back(CreateWindow(L"STATIC", L"Rel Error", WS_VISIBLE | WS_CHILD,
		rect.right / 2 + rect.right / 3 + 50, rect.bottom / 2 - rect.bottom / 3 - 50, 120, 20,
		lineWnd->getHandle(), nullptr, lineWnd->hCurrentInst, nullptr));



	long double resultMinValue = std::numeric_limits<long double>::infinity();
	long double resultMaxValue = 0.L;
	long double relerrMinValue = 1.L;
	long double relerrMaxValue = 0.L;

	for (auto el : lineWnd->linePoints) {
		resultMinValue = min(resultMinValue, el.value);
		resultMaxValue = max(resultMaxValue, el.value);
		relerrMinValue = min(relerrMinValue, el.relError);
		relerrMaxValue = max(relerrMaxValue, el.relError);
	}

	long double resDiff = resultMaxValue - resultMinValue;
	long double relerrDiff = relerrMaxValue - relerrMinValue;
	float axDiff = lineWnd->linePoints[lineWnd->linePointsDataSize - 1].axis - lineWnd->linePoints[0].axis;

	std::vector<Gdiplus::Point> curveResPts;
	std::vector<Gdiplus::Point> curveRelErrPts;

	for (auto el : lineWnd->linePoints) {
		curveResPts.push_back({
			int(rect.right / 2 - rect.right / 3 + (rect.right * 2. / 3.) * (el.axis - lineWnd->linePoints[0].axis) / axDiff),
			int(rect.bottom / 2 + rect.bottom / 3 - (rect.bottom * 2. / 3.) * (el.value - resultMinValue) / resDiff)
		});
		
		curveRelErrPts.push_back({
			int(rect.right / 2 - rect.right / 3 + (rect.right * 2. / 3.) * (el.axis - lineWnd->linePoints[0].axis) / axDiff),
			int(rect.bottom / 2 + rect.bottom / 3 - (rect.bottom * 2. / 3.) * (el.relError - relerrMinValue) / relerrDiff)
		});
	}

	Gdiplus::Pen resPen(Gdiplus::Color(255, 255, 153, 0), 3);
	Gdiplus::Pen relErrPen(Gdiplus::Color(255, 204, 0, 153), 3);

	graphics.DrawCurve(&resPen, curveResPts.data(), curveResPts.size(), 0.);
	graphics.DrawCurve(&relErrPen, curveRelErrPts.data(), curveRelErrPts.size(), 0.);

	long double resTenth = resDiff / 10.L;
	long double relerrTenth = relerrDiff / 10.L;


	for (int i = 0; i < 10; ++i) {
		swprintf_s(axisValWStr, L"%1.5e", resultMaxValue - i * resTenth);
		lineWnd->childWnds.push_back(CreateWindow(L"STATIC", axisValWStr, WS_VISIBLE | WS_CHILD,
			rect.right / 2 - rect.right / 3 - 120, rect.bottom / 2 - rect.bottom / 3 + i * heightTenth - 10, 100, 20,
			lineWnd->getHandle(), nullptr, lineWnd->hCurrentInst, nullptr));

		swprintf_s(axisValWStr, L"%1.5e", relerrMaxValue - i * relerrTenth);
		lineWnd->childWnds.push_back(CreateWindow(L"STATIC", axisValWStr, WS_VISIBLE | WS_CHILD,
			rect.right / 2 + rect.right / 3 + 40, rect.bottom / 2 - rect.bottom / 3 + i * heightTenth - 10, 100, 20,
			lineWnd->getHandle(), nullptr, lineWnd->hCurrentInst, nullptr));
	}

	swprintf_s(axisValWStr, L"%1.5e", resultMinValue);
	lineWnd->childWnds.push_back(CreateWindow(L"STATIC", axisValWStr, WS_VISIBLE | WS_CHILD,
		rect.right / 2 - rect.right / 3 - 120, rect.bottom / 2 - rect.bottom / 3 + 10 * heightTenth - 10, 100, 20,
		lineWnd->getHandle(), nullptr, lineWnd->hCurrentInst, nullptr));

	swprintf_s(axisValWStr, L"%1.5e", relerrMinValue);
	lineWnd->childWnds.push_back(CreateWindow(L"STATIC", axisValWStr, WS_VISIBLE | WS_CHILD,
		rect.right / 2 + rect.right / 3 + 40, rect.bottom / 2 - rect.bottom / 3 + 10 * heightTenth - 10, 100, 20,
		lineWnd->getHandle(), nullptr, lineWnd->hCurrentInst, nullptr));

	for (int i = 0; i < 20; ++i) {
		for (int j = 0; j < 40; ++j) {
			SetPixel(hdc, rect.right / 2 - rect.right / 3 + j, rect.bottom / 2 - rect.bottom / 3 - 50 + i, RGB(255, 153, 0));

			SetPixel(hdc, rect.right / 2 + rect.right / 3 + j - 40, rect.bottom / 2 - rect.bottom / 3 - 50 + i, RGB(204, 0, 153));
		}
	}


	EndPaint(lineWnd->getHandle(), ps);
	DeleteDC(hdc);
}
