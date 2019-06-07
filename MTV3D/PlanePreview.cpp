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
#include "PlanePreview.h"


PlanePreview::PlanePreview(char axis, float axisValue): axis(axis), axisValue(axisValue) {
}


void PlanePreview::run(DWORD callingThreadId, HINSTANCE hCurrentInst, HACCEL hAccelTable, LPWSTR fileAbsolutePath, int planePointsDataSize, Graphics::Point2D* planePointsData) {
	this->hCurrentInst = hCurrentInst;
	this->hAccelTable = hAccelTable;
	this->fileAbsolutePath = fileAbsolutePath;
	this->windowTitle = new WCHAR[WCHAR_ARR_MAX];
	swprintf_s(this->windowTitle, WCHAR_ARR_MAX - 1, L"MTV3D - %s - %c = %1.3f", this->fileAbsolutePath, this->axis, this->axisValue);

	this->hVisMerWnd = std::make_unique<PlaneMergedWnd>(this->hCurrentInst, this->windowTitle);
	ShowWindow(this->hVisMerWnd->getHandle(), SW_SHOWMAXIMIZED);

	SetCursor(LoadCursor(nullptr, IDC_WAIT));


	std::set<float> axisOneValues;
	std::set<float> axisTwoValues;

	std::vector<std::vector<Graphics::Point2D>> visPlaneModel;
	std::vector<Graphics::Point2D> visLineModel;

	long double resultMinValue = std::numeric_limits<long double>::infinity();
	long double resultMaxValue = 0.L;
	long double relerrMinValue = 1.L;
	long double relerrMaxValue = 0.L;


	std::sort(planePointsData, planePointsData + planePointsDataSize,
		[](const Graphics::Point2D & a, const Graphics::Point2D & b) -> bool {
			if (a.axisOne == b.axisOne) {
				return a.axisTwo < b.axisTwo;
			}
			else {
				return a.axisOne < b.axisOne;
			}
		}
	);


	for (int i = 0; i < planePointsDataSize; ++i) {
		axisOneValues.insert(planePointsData[i].axisOne);
		axisTwoValues.insert(planePointsData[i].axisTwo);

		Graphics::Point2D visPoint = {
			planePointsData[i].axisOne, planePointsData[i].axisTwo,
			planePointsData[i].value, planePointsData[i].relError
		};

		if (i > 0) {
			if (planePointsData[i].axisOne != planePointsData[i - 1].axisOne) {
				visPlaneModel.push_back(visLineModel);

				visLineModel.clear();
			}

			if (visPoint.value > 0.L) {
				if (visPoint.value < resultMinValue) resultMinValue = visPoint.value;
				if (visPoint.value > resultMaxValue) resultMaxValue = visPoint.value;
				if (visPoint.relError < relerrMinValue) relerrMinValue = visPoint.relError;
				if (visPoint.relError > relerrMaxValue) relerrMaxValue = visPoint.relError;
			}
		}

		visLineModel.push_back(visPoint);
	}
	visPlaneModel.push_back(visLineModel);
	visLineModel.clear();
	delete[] planePointsData;


	long double resultMinValueLog10 = std::log10(resultMinValue);
	long double resultMaxValueLog10 = std::log10(resultMaxValue);
	long double resultLogFifth = (resultMaxValueLog10 - resultMinValueLog10) / 5.L;
	if (resultLogFifth < 0.L) {
		resultLogFifth = -resultLogFifth;
	}

	this->resultLegend.push_back({ 1.f, 0.f, 1.f, resultMinValue });
	this->resultLegend.push_back({ 0.f, 0.f, 1.f, std::pow(10, resultMinValueLog10 + resultLogFifth) });
	this->resultLegend.push_back({ 0.f, 1.f, 1.f, std::pow(10, resultMinValueLog10 + 2.L * resultLogFifth) });
	this->resultLegend.push_back({ 0.f, 1.f, 0.f, std::pow(10, resultMinValueLog10 + 3.L * resultLogFifth) });
	this->resultLegend.push_back({ 1.f, 1.f, 0.f, std::pow(10, resultMinValueLog10 + 4.L * resultLogFifth) });
	this->resultLegend.push_back({ 1.f, 0.f, 0.f, resultMaxValue });


	long double relerrMinValueLog10 = (relerrMinValue > 0.L) ? std::log10(relerrMinValue) : std::numeric_limits<long double>::min_exponent10;
	long double relerrMaxValueLog10 = std::log10(relerrMaxValue);
	long double relerrLogQuarter = (relerrMaxValueLog10 - relerrMinValueLog10) / 4.L;

	this->relerrLegend.push_back({ 0.f, 0.f, 1.f, relerrMaxValue });
	this->relerrLegend.push_back({ 0.5f, 0.f, 1.f, std::pow(10, relerrMaxValueLog10 - relerrLogQuarter) });
	this->relerrLegend.push_back({ 1.f, 0.f, 1.f, std::pow(10, relerrMaxValueLog10 - 2.L * relerrLogQuarter) });
	this->relerrLegend.push_back({ 1.f, 0.f, 0.5f, std::pow(10, relerrMaxValueLog10 - 3.L * relerrLogQuarter) });
	this->relerrLegend.push_back({ 1.f, 0.f, 0.f, relerrMinValue });


	int axisOneSize = axisOneValues.size();
	int axisTwoSize = axisTwoValues.size();
	int axisReliefSize = resultMaxValue - resultMinValue;

	int q = axisOneSize - 1;
	int w = axisTwoSize - 1;

	float modelAbscissaLength = std::abs(visPlaneModel[q][w].axisOne - visPlaneModel[0][0].axisOne);
	float modelOrdinateLength = std::abs(visPlaneModel[axisOneSize - 1][axisTwoSize - 1].axisTwo - visPlaneModel[0][0].axisTwo);

	float modelAbscissaCenter = std::vector<float>(axisOneValues.begin(), axisOneValues.end())[axisOneSize / 2];
	float modelOrdinateCenter = std::vector<float>(axisTwoValues.begin(), axisTwoValues.end())[axisTwoSize / 2];
	float modelReliefCenter = float(resultMaxValue / 2.f);

	float diagonal3DLength = std::sqrt(modelAbscissaLength * modelAbscissaLength + modelOrdinateLength * modelOrdinateLength + modelReliefCenter * modelReliefCenter);



	CreateWindow(L"STATIC", L"Result", WS_VISIBLE | WS_CHILD,
		this->hVisMerWnd->getWndRect().right / 2 - this->hVisMerWnd->getDisplayDim() / 25 - 110, 30, 120, 20,
		this->hVisMerWnd->getHandle(), nullptr, this->hCurrentInst, nullptr);

	CreateWindow(L"STATIC", L"Relative\nError", WS_VISIBLE | WS_CHILD,
		this->hVisMerWnd->getWndRect().right / 2 + this->hVisMerWnd->getDisplayDim() / 25 + 30, 30, 120, 40,
		this->hVisMerWnd->getHandle(), nullptr, this->hCurrentInst, nullptr);

	WCHAR legendValue[101];

	for (int i = 0; i <= 5; ++i) {
		swprintf_s(legendValue, L"%1.5e", this->resultLegend[5 - i].value);

		CreateWindow(L"STATIC", legendValue, WS_VISIBLE | WS_CHILD,
			this->hVisMerWnd->getWndRect().right / 2 - this->hVisMerWnd->getDisplayDim() / 25 - 110, 90 + int((this->hVisMerWnd->getDisplayDim() / 2.L) * i / 5.L), 120, 20,
			this->hVisMerWnd->getHandle(), nullptr, this->hCurrentInst, nullptr);
	}

	swprintf_s(legendValue, L"%1.5e", 0.L);

	CreateWindow(L"STATIC", legendValue, WS_VISIBLE | WS_CHILD,
		this->hVisMerWnd->getWndRect().right / 2 - this->hVisMerWnd->getDisplayDim() / 25 - 110, 90 + int(this->hVisMerWnd->getDisplayDim() / 2.L) + 60, 120, 20,
		this->hVisMerWnd->getHandle(), nullptr, this->hCurrentInst, nullptr);


	for (int i = 0; i <= 4; ++i) {
		swprintf_s(legendValue, L"%1.5e", this->relerrLegend[4 - i].value);

		CreateWindow(L"STATIC", legendValue, WS_VISIBLE | WS_CHILD,
			this->hVisMerWnd->getWndRect().right / 2 + this->hVisMerWnd->getDisplayDim() / 25 + 30, 90 + int((this->hVisMerWnd->getDisplayDim() / 2.L) * i / 4.L), 120, 20,
			this->hVisMerWnd->getHandle(), nullptr, this->hCurrentInst, nullptr);
	}


	SetCursor(LoadCursor(nullptr, IDC_ARROW));


	MSG msg;
	bool quitFlag = false;

	while (true) {
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				quitFlag = true;
				break;
			}

			if (!TranslateAccelerator(msg.hwnd, this->hAccelTable, &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		if (quitFlag) {
			break;
		}
	}

	delete[] this->windowTitle;

	float* customLParam = new float[2];
	customLParam[0] = float((int)this->axis);
	customLParam[1] = this->axisValue;

	PostThreadMessage(callingThreadId, WM_THREAD_DONE, 0, (LPARAM)customLParam);
}


Graphics::CustomColor PlanePreview::getResultColor(long double resultValue) {
	if (resultValue == 0.L) {
		return { 1.f, 1.f, 1.f };
	}

	long double valLog = std::log10(resultValue);
	long double levelColor;

	if (valLog <= std::log10(this->resultLegend[1].value)) {
		levelColor = std::abs(valLog - std::log10(this->resultLegend[0].value)) / std::abs(std::log10(this->resultLegend[1].value) - std::log10(this->resultLegend[0].value));
		return { 1.f - (float)levelColor, 0.f, 1.f };
	}
	else if (valLog <= std::log10(this->resultLegend[2].value)) {
		levelColor = std::abs(valLog - std::log10(this->resultLegend[1].value)) / std::abs(std::log10(this->resultLegend[2].value) - std::log10(this->resultLegend[1].value));
		return { 0.f, (float)levelColor, 1.f };
	}
	else if (valLog <= std::log10(this->resultLegend[3].value)) {
		levelColor = std::abs(valLog - std::log10(this->resultLegend[2].value)) / std::abs(std::log10(this->resultLegend[3].value) - std::log10(this->resultLegend[2].value));
		return { 0.f, 1.f, 1.f - (float)levelColor };
	}
	else if (valLog <= std::log10(this->resultLegend[4].value)) {
		levelColor = std::abs(valLog - std::log10(this->resultLegend[3].value)) / std::abs(std::log10(this->resultLegend[4].value) - std::log10(this->resultLegend[3].value));
		return { (float)levelColor, 1.f, 0.f };
	}
	else {
		levelColor = std::abs(valLog - std::log10(this->resultLegend[4].value)) / std::abs(std::log10(this->resultLegend[5].value) - std::log10(this->resultLegend[4].value));
		return { 1.f, 1.f - (float)levelColor, 0.f };
	}
}


Graphics::CustomColor PlanePreview::getRelErrColor(long double relerrValue) {
	long double valLog = std::log10(relerrValue);
	long double levelColor;

	if (valLog >= this->relerrLegend[1].value) {
		levelColor = std::abs(std::log10(this->relerrLegend[0].value - valLog)) / std::abs(std::log10(this->relerrLegend[0].value) - std::log10(this->relerrLegend[1].value));
		return { (float)levelColor * 0.5f, 0.f, 1.f };
	}
	else if (valLog >= this->relerrLegend[2].value) {
		levelColor = std::abs(std::log10(this->relerrLegend[1].value - valLog)) / std::abs(std::log10(this->relerrLegend[1].value) - std::log10(this->relerrLegend[2].value));
		return { (float)levelColor * 0.5f + 0.5f, 0.f, 1.f };
	}
	else if (valLog >= this->relerrLegend[3].value) {
		levelColor = std::abs(std::log10(this->relerrLegend[2].value - valLog)) / std::abs(std::log10(this->relerrLegend[2].value) - std::log10(this->relerrLegend[3].value));
		return { 1.f, 0.f, 1.f - (float)levelColor * 0.5f };
	}
	else {
		levelColor = std::abs(std::log10(this->relerrLegend[3].value - valLog)) / std::abs(std::log10(this->relerrLegend[3].value) - (this->relerrLegend[4].value > 0.L) ? std::log10(this->relerrLegend[4].value) : std::numeric_limits<long double>::min_exponent10);
		return { 1.f, 0.f, 0.5f - (float)levelColor * 0.5f };
	}
}
