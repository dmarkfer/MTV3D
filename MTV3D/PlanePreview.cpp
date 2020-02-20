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


thread_local float PlanePreview::scaleBase = 1.f;
thread_local bool PlanePreview::gridActive = true;
thread_local bool PlanePreview::axesValsActive = true;
thread_local bool PlanePreview::flagLinePrevCreation = false;


PlanePreview::PlanePreview(char axis, float axisValue): axis(axis), axisValue(axisValue) {
}


void PlanePreview::run(DWORD callingThreadId, HINSTANCE hCurrentInst, HACCEL hAccelTable, LPWSTR fileAbsolutePath, int planePointsDataSize, Graphics::Point2D* planePointsData) {
	this->hCurrentInst = hCurrentInst;
	this->hAccelTable = hAccelTable;
	this->fileAbsolutePath = fileAbsolutePath;
	this->windowTitle = new WCHAR[WCHAR_ARR_MAX];
	WCHAR axisEqValueStr[101];
	swprintf_s(axisEqValueStr, L"%c = %1.3f", this->axis, this->axisValue);
	swprintf_s(this->windowTitle, WCHAR_ARR_MAX - 1, L"MTV3D - %s ( %s )", this->fileAbsolutePath, axisEqValueStr);

	this->hVisMerWnd = std::make_unique<PlaneMergedWnd>(this->hCurrentInst, this->windowTitle);
	if (this->axis == 'X') {
		SendMessage(this->hVisMerWnd->getRadioButtonY(), BM_SETCHECK, BST_CHECKED, 0);
		EnableWindow(this->hVisMerWnd->getRadioButtonX(), false);
		ShowWindow(this->hVisMerWnd->getRadioButtonX(), SW_HIDE);
		CreateWindow(L"STATIC", axisEqValueStr, WS_VISIBLE | WS_CHILD,
			this->hVisMerWnd->getWndRect().right / 4 + 50, this->hVisMerWnd->getDisplayDim() + this->hVisMerWnd->getDialogHeight() / 3 + 30, 100, 20,
			this->hVisMerWnd->getHandle(), nullptr, this->hCurrentInst, nullptr);
	}
	else if (this->axis == 'Y') {
		SendMessage(this->hVisMerWnd->getRadioButtonX(), BM_SETCHECK, BST_CHECKED, 0);
		EnableWindow(this->hVisMerWnd->getRadioButtonY(), false);
		ShowWindow(this->hVisMerWnd->getRadioButtonY(), SW_HIDE);
		CreateWindow(L"STATIC", axisEqValueStr, WS_VISIBLE | WS_CHILD,
			this->hVisMerWnd->getWndRect().right / 4 + 120, this->hVisMerWnd->getDisplayDim() + this->hVisMerWnd->getDialogHeight() / 3 + 30, 100, 20,
			this->hVisMerWnd->getHandle(), nullptr, this->hCurrentInst, nullptr);
	}
	else {
		SendMessage(this->hVisMerWnd->getRadioButtonX(), BM_SETCHECK, BST_CHECKED, 0);
		EnableWindow(this->hVisMerWnd->getRadioButtonZ(), false);
		ShowWindow(this->hVisMerWnd->getRadioButtonZ(), SW_HIDE);
		CreateWindow(L"STATIC", axisEqValueStr, WS_VISIBLE | WS_CHILD,
			this->hVisMerWnd->getWndRect().right / 4 + 200, this->hVisMerWnd->getDisplayDim() + this->hVisMerWnd->getDialogHeight() / 3 + 30, 100, 20,
			this->hVisMerWnd->getHandle(), nullptr, this->hCurrentInst, nullptr);
	}
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
	long double relerrLogFifth = (relerrMaxValueLog10 - relerrMinValueLog10) / 5.L;

	this->relerrLegend.push_back({ 1.f, 0.f, 0.f, relerrMinValue });
	this->relerrLegend.push_back({ 1.f, 1.f, 0.f, std::pow(10, relerrMaxValueLog10 - 4.L * relerrLogFifth) });
	this->relerrLegend.push_back({ 0.f, 1.f, 0.f, std::pow(10, relerrMaxValueLog10 - 3.L * relerrLogFifth) });
	this->relerrLegend.push_back({ 0.f, 1.f, 1.f, std::pow(10, relerrMaxValueLog10 - 2.L * relerrLogFifth) });
	this->relerrLegend.push_back({ 0.f, 0.f, 1.f, std::pow(10, relerrMaxValueLog10 - relerrLogFifth) });
	this->relerrLegend.push_back({ 1.f, 0.f, 1.f, relerrMaxValue });


	int axisOneSize = axisOneValues.size();
	int axisTwoSize = axisTwoValues.size();

	float modelAbscissaLength = std::abs(visPlaneModel[axisOneSize - 1][axisTwoSize - 1].axisOne - visPlaneModel[0][0].axisOne);
	float modelOrdinateLength = std::abs(visPlaneModel[axisOneSize - 1][axisTwoSize - 1].axisTwo - visPlaneModel[0][0].axisTwo);
	long double modelReliefLength = max(modelAbscissaLength, modelOrdinateLength);

	float modelAbscissaCenter = std::vector<float>(axisOneValues.begin(), axisOneValues.end())[axisOneSize / 2];
	float modelOrdinateCenter = std::vector<float>(axisTwoValues.begin(), axisTwoValues.end())[axisTwoSize / 2];

	float diagonal3DLength = std::sqrt(modelAbscissaLength * modelAbscissaLength + modelOrdinateLength * modelOrdinateLength + float(modelReliefLength * modelReliefLength));



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


	for (int i = 0; i <= 5; ++i) {
		swprintf_s(legendValue, L"%1.5e", this->relerrLegend[5 - i].value);

		CreateWindow(L"STATIC", legendValue, WS_VISIBLE | WS_CHILD,
			this->hVisMerWnd->getWndRect().right / 2 + this->hVisMerWnd->getDisplayDim() / 25 + 30, 90 + int((this->hVisMerWnd->getDisplayDim() / 2.L) * i / 5.L), 120, 20,
			this->hVisMerWnd->getHandle(), nullptr, this->hCurrentInst, nullptr);
	}

	swprintf_s(legendValue, L"irrelevant");

	CreateWindow(L"STATIC", legendValue, WS_VISIBLE | WS_CHILD,
		this->hVisMerWnd->getWndRect().right / 2 + this->hVisMerWnd->getDisplayDim() / 25 + 30, 90 + int(this->hVisMerWnd->getDisplayDim() / 2.L) + 60, 120, 20,
		this->hVisMerWnd->getHandle(), nullptr, this->hCurrentInst, nullptr);


	SetCursor(LoadCursor(nullptr, IDC_ARROW));


	this->initDirect3D();


	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;

	D3D11_VIEWPORT viewport;
	viewport.Width = (float)this->hVisMerWnd->getDisplayDim();
	viewport.Height = (float)this->hVisMerWnd->getDisplayDim();
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;
	viewport.TopLeftX = 0.f;
	viewport.TopLeftY = 0.f;

	Microsoft::WRL::ComPtr<ID3D11Resource> backBufferResultDisplay;
	this->swapChainResultDisplay->GetBuffer(0, __uuidof(ID3D11Resource), &backBufferResultDisplay);
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetResultDisplay;
	this->d3dDevice->CreateRenderTargetView(backBufferResultDisplay.Get(), nullptr, &renderTargetResultDisplay);

	Microsoft::WRL::ComPtr<ID3D11Resource> backBufferRelErrDisplay;
	this->swapChainRelErrDisplay->GetBuffer(0, __uuidof(ID3D11Resource), &backBufferRelErrDisplay);
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetRelErrDisplay;
	this->d3dDevice->CreateRenderTargetView(backBufferRelErrDisplay.Get(), nullptr, &renderTargetRelErrDisplay);


	Microsoft::WRL::ComPtr<IDWriteFactory2> writeFactory;
	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &writeFactory);

	Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
	this->d3dDevice.Get()->QueryInterface(__uuidof(IDXGIDevice), &dxgiDevice);

	Microsoft::WRL::ComPtr<ID2D1Device> d2dDevice;
	D2D1_CREATION_PROPERTIES d2dDeviceCreationProps;
	ZeroMemory(&d2dDeviceCreationProps, sizeof(D2D1_CREATION_PROPERTIES));
	d2dDeviceCreationProps.threadingMode = D2D1_THREADING_MODE_SINGLE_THREADED;
	d2dDeviceCreationProps.debugLevel = D2D1_DEBUG_LEVEL_NONE;
	d2dDeviceCreationProps.options = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
	D2D1CreateDevice(dxgiDevice.Get(), &d2dDeviceCreationProps, &d2dDevice);

	Microsoft::WRL::ComPtr<ID2D1DeviceContext> d2dDeviceContext;
	d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2dDeviceContext);

	D2D1_BITMAP_PROPERTIES1 d2dBitmapProperties;
	ZeroMemory(&d2dBitmapProperties, sizeof(D2D1_BITMAP_PROPERTIES));
	d2dBitmapProperties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	d2dBitmapProperties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
	d2dBitmapProperties.dpiX = 96.f;
	d2dBitmapProperties.dpiY = 96.f;
	d2dBitmapProperties.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
	d2dBitmapProperties.colorContext = nullptr;

	Microsoft::WRL::ComPtr<IDXGISurface> dxgiBufferRes;
	this->swapChainResultDisplay->GetBuffer(0, __uuidof(IDXGISurface), &dxgiBufferRes);
	Microsoft::WRL::ComPtr<IDXGISurface> dxgiBufferRelErr;
	this->swapChainRelErrDisplay->GetBuffer(0, __uuidof(IDXGISurface), &dxgiBufferRelErr);

	Microsoft::WRL::ComPtr<ID2D1Bitmap1> d2dTargetBitmapRes;
	d2dDeviceContext->CreateBitmapFromDxgiSurface(dxgiBufferRes.Get(), &d2dBitmapProperties, &d2dTargetBitmapRes);
	Microsoft::WRL::ComPtr<ID2D1Bitmap1> d2dTargetBitmapRelErr;
	d2dDeviceContext->CreateBitmapFromDxgiSurface(dxgiBufferRelErr.Get(), &d2dBitmapProperties, &d2dTargetBitmapRelErr);

	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> blackBrush;
	d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &blackBrush);

	Microsoft::WRL::ComPtr<IDWriteTextFormat> textFormat;
	writeFactory.Get()->CreateTextFormat(L"Arial", nullptr, DWRITE_FONT_WEIGHT_LIGHT, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 10.f, L"en-us", &textFormat);
	textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

	Microsoft::WRL::ComPtr<IDWriteTextLayout> textLayout;


	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	this->d3dDevice->CreateDepthStencilState(&depthStencilDesc, &depthStencilState);

	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencil;
	D3D11_TEXTURE2D_DESC depthDesc;
	ZeroMemory(&depthDesc, sizeof(D3D11_TEXTURE2D_DESC));
	depthDesc.Width = this->hVisMerWnd->getDisplayDim();
	depthDesc.Height = this->hVisMerWnd->getDisplayDim();
	depthDesc.MipLevels = 1u;
	depthDesc.ArraySize = 1u;
	depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthDesc.SampleDesc.Count = 1u;
	depthDesc.SampleDesc.Quality = 0u;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	this->d3dDevice->CreateTexture2D(&depthDesc, nullptr, &depthStencil);

	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0u;

	this->d3dDevice->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, &depthStencilView);


	std::vector<std::pair<DirectX::XMVECTOR, std::pair<std::pair<long double, long double>, std::wstring>>> gridAxesValuesVertices;
	std::vector<Graphics::Vertex> gridLinesVertices;
	constexpr float gridLineExtensionPerc = 0.1f;
	float relfToAxis = (1 + gridLineExtensionPerc) * float(modelReliefLength / 2.L);

	for (int i = 0; i <= 10; ++i) {
		Graphics::Point2D visp = visPlaneModel[0][i * (axisTwoSize - 1) / 10];
		visp.axisOne -= modelAbscissaCenter;
		visp.axisTwo -= modelOrdinateCenter;
		visp.axisOne -= gridLineExtensionPerc * modelAbscissaLength;
		gridLinesVertices.push_back({ visp.axisOne, relfToAxis * (1 + 2 * gridLineExtensionPerc), visp.axisTwo, { 0.f, 0.f, 0.f } });

		gridLinesVertices.push_back({ visp.axisOne, -relfToAxis * (1 + 2 * gridLineExtensionPerc), visp.axisTwo, { 0.f, 0.f, 0.f } });

		gridLinesVertices.push_back({ visp.axisOne, -relfToAxis * (1 + 2 * gridLineExtensionPerc), visp.axisTwo, { 0.f, 0.f, 0.f } });

		visp = visPlaneModel[axisOneSize - 1][i * (axisTwoSize - 1) / 10];
		float twoRealValue = visp.axisTwo;
		visp.axisOne -= modelAbscissaCenter;
		visp.axisTwo -= modelOrdinateCenter;
		visp.axisOne += gridLineExtensionPerc * modelAbscissaLength;
		gridLinesVertices.push_back({ visp.axisOne, -relfToAxis * (1 + 2 * gridLineExtensionPerc), visp.axisTwo, { 0.f, 0.f, 0.f } });

		gridAxesValuesVertices.push_back(std::make_pair(
			DirectX::XMVectorSet(visp.axisOne, -relfToAxis * (1 + 2 * gridLineExtensionPerc), visp.axisTwo, 1.f),
			std::make_pair(
				std::make_pair(twoRealValue, 0.L),
				i == 5 ? L"2MID" : L"2"
			)
		));

		visp = visPlaneModel[i * (axisOneSize - 1) / 10][0];
		visp.axisOne -= modelAbscissaCenter;
		visp.axisTwo -= modelOrdinateCenter;
		visp.axisTwo -= gridLineExtensionPerc * modelOrdinateLength;
		gridLinesVertices.push_back({ visp.axisOne, relfToAxis * (1 + 2 * gridLineExtensionPerc), visp.axisTwo, { 0.f, 0.f, 0.f } });
		gridLinesVertices.push_back({ visp.axisOne, -relfToAxis * (1 + 2 * gridLineExtensionPerc), visp.axisTwo, { 0.f, 0.f, 0.f } });

		gridLinesVertices.push_back({ visp.axisOne, -relfToAxis * (1 + 2 * gridLineExtensionPerc), visp.axisTwo, { 0.f, 0.f, 0.f } });

		visp = visPlaneModel[i * (axisOneSize - 1) / 10][axisTwoSize - 1];
		float oneRealValue = visp.axisOne;
		visp.axisOne -= modelAbscissaCenter;
		visp.axisTwo -= modelOrdinateCenter;
		visp.axisTwo += gridLineExtensionPerc * modelOrdinateLength;
		gridLinesVertices.push_back({ visp.axisOne, -relfToAxis * (1 + 2 * gridLineExtensionPerc), visp.axisTwo, { 0.f, 0.f, 0.f } });

		gridAxesValuesVertices.push_back(std::make_pair(
			DirectX::XMVectorSet(visp.axisOne, -relfToAxis * (1 + 2 * gridLineExtensionPerc), visp.axisTwo, 1.f),
			std::make_pair(
				std::make_pair(oneRealValue, 0.L),
				i == 5 ? L"1MID" : L"1"
			)
		));

		visp = visPlaneModel[axisOneSize - 1][0];
		visp.axisOne -= modelAbscissaCenter;
		visp.axisTwo -= modelOrdinateCenter;
		visp.axisOne += gridLineExtensionPerc * modelAbscissaLength;
		visp.axisTwo -= gridLineExtensionPerc * modelOrdinateLength;
		gridLinesVertices.push_back({ visp.axisOne, i * float((1 + gridLineExtensionPerc) * modelReliefLength / 10.L) - relfToAxis, visp.axisTwo, { 0.f, 0.f, 0.f } });

		gridAxesValuesVertices.push_back(std::make_pair(
			DirectX::XMVectorSet(visp.axisOne, i * float((1 + gridLineExtensionPerc) * modelReliefLength / 10.L) - relfToAxis, visp.axisTwo, 1.f),
			std::make_pair(
				std::make_pair(resultMinValue + i * (resultMaxValue - resultMinValue) / 10.L, relerrMaxValue - i * (relerrMaxValue - relerrMinValue) / 10.L),
				i == 5 ? L"3MID" : L"3"
			)
		));

		visp = visPlaneModel[0][0];
		visp.axisOne -= modelAbscissaCenter;
		visp.axisTwo -= modelOrdinateCenter;
		visp.axisOne -= gridLineExtensionPerc * modelAbscissaLength;
		visp.axisTwo -= gridLineExtensionPerc * modelOrdinateLength;
		gridLinesVertices.push_back({ visp.axisOne, i * float((1 + gridLineExtensionPerc) * modelReliefLength / 10.L) - relfToAxis, visp.axisTwo, { 0.f, 0.f, 0.f } });

		gridLinesVertices.push_back({ visp.axisOne, i * float((1 + gridLineExtensionPerc) * modelReliefLength / 10.L) - relfToAxis, visp.axisTwo, { 0.f, 0.f, 0.f } });

		visp = visPlaneModel[0][axisTwoSize - 1];
		visp.axisOne -= modelAbscissaCenter;
		visp.axisTwo -= modelOrdinateCenter;
		visp.axisOne -= gridLineExtensionPerc * modelAbscissaLength;
		visp.axisTwo += gridLineExtensionPerc * modelOrdinateLength;
		gridLinesVertices.push_back({ visp.axisOne, i * float((1 + gridLineExtensionPerc) * modelReliefLength / 10.L) - relfToAxis, visp.axisTwo, { 0.f, 0.f, 0.f } });
	}

	Microsoft::WRL::ComPtr<ID3D11Buffer> gridVertexBuffer;
	D3D11_BUFFER_DESC gridVertexBufferDesc;
	ZeroMemory(&gridVertexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	gridVertexBufferDesc.ByteWidth = sizeof(Graphics::Vertex) * gridLinesVertices.size();
	gridVertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	gridVertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	gridVertexBufferDesc.CPUAccessFlags = 0;
	gridVertexBufferDesc.MiscFlags = 0;
	gridVertexBufferDesc.StructureByteStride = sizeof(Graphics::Vertex);

	D3D11_SUBRESOURCE_DATA gridVertexSubresourceData;
	ZeroMemory(&gridVertexSubresourceData, sizeof(D3D11_SUBRESOURCE_DATA));
	gridVertexSubresourceData.pSysMem = &gridLinesVertices[0];
	gridVertexSubresourceData.SysMemPitch = 0;
	gridVertexSubresourceData.SysMemSlicePitch = 0;

	this->d3dDevice->CreateBuffer(&gridVertexBufferDesc, &gridVertexSubresourceData, &gridVertexBuffer);


	long double resValQ = (1 + gridLineExtensionPerc) * modelReliefLength / (resultMaxValue - resultMinValue);
	long double relErrValQ = (1 + gridLineExtensionPerc) * modelReliefLength / (relerrMaxValue - relerrMinValue);

	for (int i = 0; i < axisOneSize; ++i) {
		for (int j = 0; j < axisTwoSize; ++j) {
			Graphics::Point2D visp = visPlaneModel[i][j];
			visp.axisOne -= modelAbscissaCenter;
			visp.axisTwo -= modelOrdinateCenter;
			verticesResult.push_back({ visp.axisOne, float((visp.value - resultMinValue) * resValQ - relfToAxis), visp.axisTwo, getResultColor(visp.value) });
			verticesRelErr.push_back({ visp.axisOne, float((visp.value == 0.L ? 0.L : visp.relError - relerrMinValue) * relErrValQ - relfToAxis), visp.axisTwo, getRelErrColor(visp.value, visp.relError) });

			if (i > 0 && j > 0) {
				indices.push_back((i - 1) * axisTwoSize + j - 1);
				indices.push_back((i - 1) * axisTwoSize + j);
				indices.push_back(i * axisTwoSize + j - 1);

				indices.push_back((i - 1) * axisTwoSize + j - 1);
				indices.push_back(i * axisTwoSize + j - 1);
				indices.push_back((i - 1) * axisTwoSize + j);

				indices.push_back((i - 1) * axisTwoSize + j);
				indices.push_back(i * axisTwoSize + j);
				indices.push_back(i * axisTwoSize + j - 1);

				indices.push_back((i - 1) * axisTwoSize + j);
				indices.push_back(i * axisTwoSize + j - 1);
				indices.push_back(i * axisTwoSize + j);
			}
		}
	}



	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexResultBuffer;
	D3D11_BUFFER_DESC vertexResultBufferDesc;
	ZeroMemory(&vertexResultBufferDesc, sizeof(D3D11_BUFFER_DESC));
	vertexResultBufferDesc.ByteWidth = sizeof(Graphics::Vertex) * verticesResult.size();
	vertexResultBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexResultBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexResultBufferDesc.CPUAccessFlags = 0;
	vertexResultBufferDesc.MiscFlags = 0;
	vertexResultBufferDesc.StructureByteStride = sizeof(Graphics::Vertex);

	D3D11_SUBRESOURCE_DATA vertexResultSubresourceData;
	ZeroMemory(&vertexResultSubresourceData, sizeof(D3D11_SUBRESOURCE_DATA));
	vertexResultSubresourceData.pSysMem = &verticesResult[0];
	vertexResultSubresourceData.SysMemPitch = 0;
	vertexResultSubresourceData.SysMemSlicePitch = 0;

	this->d3dDevice->CreateBuffer(&vertexResultBufferDesc, &vertexResultSubresourceData, &vertexResultBuffer);


	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexRelErrBuffer;
	D3D11_BUFFER_DESC vertexRelErrBufferDesc;
	ZeroMemory(&vertexRelErrBufferDesc, sizeof(D3D11_BUFFER_DESC));
	vertexRelErrBufferDesc.ByteWidth = sizeof(Graphics::Vertex) * verticesRelErr.size();
	vertexRelErrBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexRelErrBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexRelErrBufferDesc.CPUAccessFlags = 0;
	vertexRelErrBufferDesc.MiscFlags = 0;
	vertexRelErrBufferDesc.StructureByteStride = sizeof(Graphics::Vertex);

	D3D11_SUBRESOURCE_DATA vertexRelErrSubresourceData;
	ZeroMemory(&vertexRelErrSubresourceData, sizeof(D3D11_SUBRESOURCE_DATA));
	vertexRelErrSubresourceData.pSysMem = &verticesRelErr[0];
	vertexRelErrSubresourceData.SysMemPitch = 0;
	vertexRelErrSubresourceData.SysMemSlicePitch = 0;

	this->d3dDevice->CreateBuffer(&vertexRelErrBufferDesc, &vertexRelErrSubresourceData, &vertexRelErrBuffer);


	const UINT stride = sizeof(Graphics::Vertex);
	const UINT offset = 0;


	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	indexBufferDesc.ByteWidth = sizeof(unsigned) * indices.size();
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = sizeof(unsigned);

	D3D11_SUBRESOURCE_DATA indexSubresourceData;
	ZeroMemory(&indexSubresourceData, sizeof(D3D11_SUBRESOURCE_DATA));
	indexSubresourceData.pSysMem = &indices[0];
	indexSubresourceData.SysMemPitch = 0;
	indexSubresourceData.SysMemSlicePitch = 0;

	this->d3dDevice->CreateBuffer(&indexBufferDesc, &indexSubresourceData, &indexBuffer);


	float color[] = { 1.f, 1.f, 1.f, 1.f };

	Microsoft::WRL::ComPtr<ID3D11Buffer> constBuffer;

	std::vector<DirectX::XMVECTOR> gridAxesValScreenVertices;

	DirectX::XMVECTOR eyePosition = DirectX::XMVectorSet(diagonal3DLength, diagonal3DLength, diagonal3DLength, 0.f);
	DirectX::XMVECTOR focusPosition = DirectX::XMVectorSet(0.f, 0.f, 0.f, 0.f);
	DirectX::XMVECTOR upDirection = DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f);
	DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationY(0);
	DirectX::XMMATRIX roundRotationMatrix = rotationMatrix;
	DirectX::XMMATRIX transformationMatrix;

	DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(eyePosition, focusPosition, upDirection);
	DirectX::XMMATRIX projectionMatrix;

	DirectX::XMVECTOR crossProd = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(eyePosition, upDirection));
	DirectX::XMVECTOR rotQuat90Deg = DirectX::XMQuaternionRotationAxis(eyePosition, -DirectX::XMConvertToRadians(90));
	DirectX::XMVECTOR horizontalPlaneNormal = DirectX::XMPlaneNormalize(DirectX::XMVector3Rotate(crossProd, rotQuat90Deg));
	DirectX::XMVECTOR verticalPlaneNormal = DirectX::XMPlaneNormalize(crossProd);



	WCHAR axVal[20];
	int axValSize;


	MSG msg;
	bool quitFlag = false;

	while (true) {
		if (PlanePreview::flagLinePrevCreation) {
			PlanePreview::flagLinePrevCreation = false;

			WCHAR selectedLineAxisStr[101];
			GetWindowText(this->hVisMerWnd->getAxisValueBox(), selectedLineAxisStr, 100);
			auto axisValCont = Graphics::validFloat(std::wstring(selectedLineAxisStr));

			if (axisValCont) {
				char selAxis = 'Z';

				if (Button_GetCheck(this->hVisMerWnd->getRadioButtonX()) == BST_CHECKED) {
					selAxis = 'X';
				}
				else if (Button_GetCheck(this->hVisMerWnd->getRadioButtonY()) == BST_CHECKED) {
					selAxis = 'Y';
				}


				float closestAxisVal = *axisValCont;
				float lastDistance = std::numeric_limits<float>::infinity();
				int axisIndex;

				if (selAxis == 'X') {
					int cntr = 0;
					for (auto axEl : axisOneValues) {
						float dist = std::abs(axEl - *axisValCont);

						if (dist <= lastDistance) {
							lastDistance = dist;
							closestAxisVal = axEl;
							axisIndex = cntr;
						}

						++cntr;
					}
				}
				else if (selAxis == 'Y'  &&  this->axis == 'X') {
					int cntr = 0;
					for (auto axEl : axisOneValues) {
						float dist = std::abs(axEl - *axisValCont);

						if (dist <= lastDistance) {
							lastDistance = dist;
							closestAxisVal = axEl;
							axisIndex = cntr;
						}

						++cntr;
					}
				}
				else if (selAxis == 'Y'  &&  this->axis == 'Z') {
					int cntr = 0;
					for (auto axEl : axisTwoValues) {
						float dist = std::abs(axEl - *axisValCont);

						if (dist <= lastDistance) {
							lastDistance = dist;
							closestAxisVal = axEl;
							axisIndex = cntr;
						}

						++cntr;
					}
				}
				else {
					int cntr = 0;
					for (auto axEl : axisTwoValues) {
						float dist = std::abs(axEl - *axisValCont);

						if (dist <= lastDistance) {
							lastDistance = dist;
							closestAxisVal = axEl;
							axisIndex = cntr;
						}

						++cntr;
					}
				}


				auto pr = std::make_pair(
					std::make_pair(this->axis, this->axisValue),
					std::make_pair(selAxis, closestAxisVal)
				);

				if (this->openLinePreviews.find(pr) == this->openLinePreviews.end()) {
					std::vector<Graphics::Point1D> linePoints;

					if (selAxis == 'X') {
						for (int j = 0; j < axisTwoSize; ++j) {
							Graphics::Point2D p = visPlaneModel[axisIndex][j];
							linePoints.push_back({ p.axisTwo, p.value, p.relError });
						}
					}
					else if (selAxis == 'Y'  &&  this->axis == 'X') {
						for (int j = 0; j < axisTwoSize; ++j) {
							Graphics::Point2D p = visPlaneModel[axisIndex][j];
							linePoints.push_back({ p.axisTwo, p.value, p.relError });
						}
					}
					else if (selAxis == 'Y'  &&  this->axis == 'Z') {
						for (int i = 0; i < axisOneSize; ++i) {
							Graphics::Point2D p = visPlaneModel[i][axisIndex];
							linePoints.push_back({ p.axisOne, p.value, p.relError });
						}
					}
					else {
						for (int i = 0; i < axisOneSize; ++i) {
							Graphics::Point2D p = visPlaneModel[i][axisIndex];
							linePoints.push_back({ p.axisOne, p.value, p.relError });
						}
					}


					int linePointsDataSize = linePoints.size();
					Graphics::Point1D* linePointsData = new Graphics::Point1D[linePointsDataSize];
					memcpy(linePointsData, linePoints.data(), linePointsDataSize * sizeof(Graphics::Point1D));


					this->openLinePreviews[pr] = std::make_unique<LinePreviewWnd>(this->hCurrentInst, fileAbsolutePath, pr, linePointsDataSize, linePointsData);
				}
				else {
					if (IsWindow(this->openLinePreviews[pr]->getHandle())) {
						MessageBox(this->hVisMerWnd->getHandle(), (std::wstring(L"You have already opened line: ") + std::wstring(1, this->axis) + L" = " + std::to_wstring(this->axisValue) + L" , " + std::wstring(1, selAxis) + L" = " + std::to_wstring(*axisValCont)).c_str(), L"Already open!", MB_ICONINFORMATION);
					}
					else {
						this->openLinePreviews[pr]->reCreate();
					}
				}
			}
		}




		RECT fillRect;
		GetClientRect(this->hVisMerWnd->getResultLegend(), &fillRect);

		HDC hdc = GetDC(this->hVisMerWnd->getResultLegend());
		Gdiplus::Graphics * gdiGraphics = Gdiplus::Graphics::FromHDC(hdc);

		int legendSize = this->resultLegend.size() - 1;
		int levelSize = int(fillRect.bottom / (float)legendSize);

		for (int i = 0; i < legendSize; ++i) {
			Graphics::CustomColor colHigh = this->resultLegend[legendSize - i].color;
			Graphics::CustomColor colLow = this->resultLegend[legendSize - i - 1].color;;

			Gdiplus::LinearGradientBrush linGradBrush(
				Gdiplus::Point(0, i * levelSize - 1),
				Gdiplus::Point(0, (i + 1) * levelSize),
				Gdiplus::Color(255, int(colHigh.r * 255), int(colHigh.g * 255), int(colHigh.b * 255)),
				Gdiplus::Color(255, int(colLow.r * 255), int(colLow.g * 255), int(colLow.b * 255))
			);

			Gdiplus::Pen brushPen(&linGradBrush, (float)fillRect.right);

			gdiGraphics->DrawLine(&brushPen, fillRect.right / 2, i * levelSize, fillRect.right / 2, (i + 1) * levelSize + ((i == legendSize - 1) ? 1 : 0));
		}

		delete gdiGraphics;
		DeleteDC(hdc);



		GetClientRect(this->hVisMerWnd->getRelErrLegend(), &fillRect);

		hdc = GetDC(this->hVisMerWnd->getRelErrLegend());
		gdiGraphics = Gdiplus::Graphics::FromHDC(hdc);

		legendSize = this->relerrLegend.size() - 1;
		levelSize = int(fillRect.bottom / (float)legendSize);

		for (int i = 0; i < legendSize; ++i) {
			Graphics::CustomColor colHigh = this->relerrLegend[legendSize - i].color;
			Graphics::CustomColor colLow = this->relerrLegend[legendSize - i - 1].color;;

			Gdiplus::LinearGradientBrush linGradBrush(
				Gdiplus::Point(0, i * levelSize - 1),
				Gdiplus::Point(0, (i + 1) * levelSize),
				Gdiplus::Color(255, int(colHigh.r * 255), int(colHigh.g * 255), int(colHigh.b * 255)),
				Gdiplus::Color(255, int(colLow.r * 255), int(colLow.g * 255), int(colLow.b * 255))
			);

			Gdiplus::Pen brushPen(&linGradBrush, (float)fillRect.right);

			gdiGraphics->DrawLine(&brushPen, fillRect.right / 2, i * levelSize, fillRect.right / 2, (i + 1) * levelSize + ((i == legendSize - 1) ? 1 : 0));
		}

		delete gdiGraphics;
		DeleteDC(hdc);



		hdc = GetDC(this->hVisMerWnd->getHandle());
		gdiGraphics = Gdiplus::Graphics::FromHDC(hdc);

		Gdiplus::Pen whitePen(Gdiplus::Color(255, 255, 255, 255), (float)fillRect.right);

		gdiGraphics->DrawLine(&whitePen, this->hVisMerWnd->getWndRect().right / 2 - this->hVisMerWnd->getDisplayDim() / 25 - 10 + fillRect.right / 2, 100 + this->hVisMerWnd->getDisplayDim() / 2 + 48,
			this->hVisMerWnd->getWndRect().right / 2 - this->hVisMerWnd->getDisplayDim() / 25 - 10 + fillRect.right / 2, 100 + this->hVisMerWnd->getDisplayDim() / 2 + 48 + 20);

		gdiGraphics->DrawLine(&whitePen, this->hVisMerWnd->getWndRect().right / 2 + 10 + fillRect.right / 2, 100 + this->hVisMerWnd->getDisplayDim() / 2 + 48,
			this->hVisMerWnd->getWndRect().right / 2 + 10 + fillRect.right / 2, 100 + this->hVisMerWnd->getDisplayDim() / 2 + 48 + 20);

		delete gdiGraphics;
		DeleteDC(hdc);
		



		this->d3dDeviceContext->ClearRenderTargetView(renderTargetResultDisplay.Get(), color);
		this->d3dDeviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.f, 0u);

		this->d3dDeviceContext->IASetVertexBuffers(0, 1, vertexResultBuffer.GetAddressOf(), &stride, &offset);
		this->d3dDeviceContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);


		if (CursorData::cursorGrabInteractionProject == this->hVisMerWnd->getResultDisplay() || CursorData::cursorGrabInteractionProject == this->hVisMerWnd->getRelErrDisplay()) {
			POINT cursorPosition;
			GetCursorPos(&cursorPosition);
			ScreenToClient(CursorData::cursorGrabInteractionProject, &cursorPosition);

			Graphics::ScreenVector clickScreenVector = {
				CursorData::clickPosX - this->hVisMerWnd->getDisplayDim() / 2.f,
				-(CursorData::clickPosY - this->hVisMerWnd->getDisplayDim() / 2.f)
			};
			Graphics::ScreenVector cursorScreenVector = {
				cursorPosition.x - this->hVisMerWnd->getDisplayDim() / 2.f,
				-(cursorPosition.y - this->hVisMerWnd->getDisplayDim() / 2.f)
			};

			float clickScreenVectorLength = std::sqrt(clickScreenVector.x * clickScreenVector.x + clickScreenVector.y * clickScreenVector.y);
			float cursorScreenVectorLength = std::sqrt(cursorScreenVector.x * cursorScreenVector.x + cursorScreenVector.y * cursorScreenVector.y);
			float rotationAngle = DirectX::XMConvertToRadians(360) * (cursorScreenVectorLength - clickScreenVectorLength) / (this->hVisMerWnd->getDisplayDim() / 2.f);

			DirectX::XMVECTOR clickVector = DirectX::XMVectorSet(clickScreenVector.x, clickScreenVector.y, 0.f, 0.f);
			DirectX::XMVECTOR cursorVector = DirectX::XMVectorSet(cursorScreenVector.x, cursorScreenVector.y, 0.f, 0.f);
			int angleSign = 1;
			float clickScreenVectorDirectionQuotient = clickScreenVector.y / (float)clickScreenVector.x;
			float cursorScreenVectorDirectionQuotient = cursorScreenVector.y / (float)cursorScreenVector.x;

			if (clickScreenVector.x >= 0.f && clickScreenVector.y >= 0.f) {
				if (cursorScreenVector.x >= 0 && cursorScreenVector.y >= 0) {
					if (cursorScreenVectorDirectionQuotient > clickScreenVectorDirectionQuotient) {
						angleSign = -angleSign;
					}
				}
				else if (cursorScreenVector.x >= 0.f && cursorScreenVector.y < 0.f) {
					angleSign = -angleSign;
				}
				else if (cursorScreenVector.x < 0.f && cursorScreenVector.y < 0.f) {
					if (cursorScreenVectorDirectionQuotient < clickScreenVectorDirectionQuotient) {
						angleSign = -angleSign;
					}
				}
			}
			else if (clickScreenVector.x < 0.f && clickScreenVector.y >= 0.f) {
				if (cursorScreenVector.x < 0.f && cursorScreenVector.y >= 0.f) {
					if (cursorScreenVectorDirectionQuotient > clickScreenVectorDirectionQuotient) {
						angleSign = -angleSign;
					}
				}
				else if (cursorScreenVector.x < 0.f && cursorScreenVector.y < 0.f) {
					angleSign = -angleSign;
				}
				else if (cursorScreenVector.x >= 0.f && cursorScreenVector.y < 0.f) {
					if (cursorScreenVectorDirectionQuotient < clickScreenVectorDirectionQuotient) {
						angleSign = -angleSign;
					}
				}
			}
			else if (clickScreenVector.x < 0.f && clickScreenVector.y < 0.f) {
				if (cursorScreenVector.x < 0.f && cursorScreenVector.y < 0.f) {
					if (cursorScreenVectorDirectionQuotient > clickScreenVectorDirectionQuotient) {
						angleSign = -angleSign;
					}
				}
				else if (cursorScreenVector.x >= 0.f && cursorScreenVector.y < 0.f) {
					angleSign = -angleSign;
				}
				else if (cursorScreenVector.x >= 0.f && cursorScreenVector.y >= 0.f) {
					if (cursorScreenVectorDirectionQuotient < clickScreenVectorDirectionQuotient) {
						angleSign = -angleSign;
					}
				}
			}
			else if (clickScreenVector.x >= 0.f && clickScreenVector.y < 0.f) {
				if (cursorScreenVector.x >= 0.f && cursorScreenVector.y < 0.f) {
					if (cursorScreenVectorDirectionQuotient > clickScreenVectorDirectionQuotient) {
						angleSign = -angleSign;
					}
				}
				else if (cursorScreenVector.x >= 0.f && cursorScreenVector.y >= 0.f) {
					angleSign = -angleSign;
				}
				else if (cursorScreenVector.x < 0.f && cursorScreenVector.y >= 0.f) {
					if (cursorScreenVectorDirectionQuotient < clickScreenVectorDirectionQuotient) {
						angleSign = -angleSign;
					}
				}
			}

			float angleClickToCurrentCursor = angleSign * DirectX::XMVector2AngleBetweenVectors(clickVector, cursorVector).m128_f32[0];

			DirectX::XMVECTOR screenUpDirection = DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f);
			float screenAzimut = DirectX::XMVector2AngleBetweenVectors(screenUpDirection, cursorVector).m128_f32[0];
			if (cursorScreenVector.x >= 0) {
				screenAzimut = DirectX::XMConvertToRadians(360) - screenAzimut;
			}
			DirectX::XMVECTOR rotQuat = DirectX::XMQuaternionRotationAxis(eyePosition, -screenAzimut);
			DirectX::XMVECTOR rotationAxis = DirectX::XMVector3Rotate(crossProd, rotQuat);

			roundRotationMatrix *= DirectX::XMMatrixRotationAxis(eyePosition, angleClickToCurrentCursor);
			rotationMatrix *= DirectX::XMMatrixRotationAxis(eyePosition, angleClickToCurrentCursor) * DirectX::XMMatrixRotationAxis(rotationAxis, rotationAngle);

			CursorData::clickPosX = cursorPosition.x;
			CursorData::clickPosY = cursorPosition.y;
		}


		transformationMatrix = rotationMatrix * DirectX::XMMatrixScaling(scaleBase, scaleBase, scaleBase);
		projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(70), 1.f, 1.f, scaleBase * 100.f * diagonal3DLength);


		gridAxesValScreenVertices.clear();

		for (auto axisPoint : gridAxesValuesVertices) {
			axisPoint.first = DirectX::XMVector3Transform(axisPoint.first, transformationMatrix);

			gridAxesValScreenVertices.push_back(DirectX::XMVector3Project(
				axisPoint.first,
				viewport.TopLeftX, viewport.TopLeftY, viewport.Width, viewport.Height, viewport.MinDepth, viewport.MaxDepth,
				projectionMatrix,
				viewMatrix,
				roundRotationMatrix * DirectX::XMMatrixScaling(scaleBase, scaleBase, scaleBase)
			));
		}


		const Graphics::ConstBufferStruct cb = {
			{
				DirectX::XMMatrixTranspose(
					transformationMatrix *
					viewMatrix *
					projectionMatrix
				)
			}
		};

		D3D11_BUFFER_DESC constBufferDesc;
		ZeroMemory(&constBufferDesc, sizeof(D3D11_BUFFER_DESC));
		constBufferDesc.ByteWidth = sizeof(cb);
		constBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		constBufferDesc.MiscFlags = 0;
		constBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA constBufferSubresourceData;
		ZeroMemory(&constBufferSubresourceData, sizeof(D3D11_SUBRESOURCE_DATA));
		constBufferSubresourceData.pSysMem = &cb;
		constBufferSubresourceData.SysMemPitch = 0;
		constBufferSubresourceData.SysMemSlicePitch = 0;

		this->d3dDevice->CreateBuffer(&constBufferDesc, &constBufferSubresourceData, &constBuffer);
		this->d3dDeviceContext->VSSetConstantBuffers(0, 1, constBuffer.GetAddressOf());

		this->d3dDevice->CreateVertexShader(Graphics::vertexShaderBlob, Graphics::vertexShaderFileSize, nullptr, &vertexShader);
		this->d3dDeviceContext->VSSetShader(vertexShader.Get(), nullptr, 0);
		this->d3dDevice->CreateInputLayout(Graphics::inputElementDesc, 2, Graphics::vertexShaderBlob, Graphics::vertexShaderFileSize, &inputLayout);
		this->d3dDeviceContext->IASetInputLayout(inputLayout.Get());
		this->d3dDevice->CreatePixelShader(Graphics::pixelShaderBlob, Graphics::pixelShaderFileSize, nullptr, &pixelShader);
		this->d3dDeviceContext->PSSetShader(pixelShader.Get(), nullptr, 0);

		this->d3dDeviceContext->OMSetDepthStencilState(depthStencilState.Get(), 1u);
		this->d3dDeviceContext->OMSetRenderTargets(1u, renderTargetResultDisplay.GetAddressOf(), depthStencilView.Get());
		this->d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		this->d3dDeviceContext->RSSetViewports(1u, &viewport);

		this->d3dDeviceContext->DrawIndexed(indices.size(), 0u, 0u);


		if (PlanePreview::gridActive) {
			this->d3dDeviceContext->IASetVertexBuffers(0, 1, gridVertexBuffer.GetAddressOf(), &stride, &offset);
			this->d3dDeviceContext->VSSetConstantBuffers(0, 1, constBuffer.GetAddressOf());

			this->d3dDeviceContext->VSSetShader(vertexShader.Get(), nullptr, 0);
			this->d3dDeviceContext->IASetInputLayout(inputLayout.Get());
			this->d3dDeviceContext->PSSetShader(pixelShader.Get(), nullptr, 0);

			this->d3dDeviceContext->OMSetDepthStencilState(depthStencilState.Get(), 1u);
			this->d3dDeviceContext->OMSetRenderTargets(1u, renderTargetResultDisplay.GetAddressOf(), depthStencilView.Get());

			this->d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
			this->d3dDeviceContext->RSSetViewports(1u, &viewport);

			this->d3dDeviceContext->Draw(gridLinesVertices.size(), 0u);
		}


		if (PlanePreview::axesValsActive) {
			d2dDeviceContext->SetTarget(d2dTargetBitmapRes.Get());

			d2dDeviceContext->BeginDraw();

			auto axValIter = gridAxesValuesVertices.begin();

			for (auto scrVec : gridAxesValScreenVertices) {
				axValSize = swprintf_s(axVal, L"%1.3Lf", axValIter->second.first.first);

				float offsetX = 0.f, offsetY = 0.f;

				if (axValIter->second.second[0] == '1') {
					offsetX = 20.f;
					offsetY = 0.f;
				}
				else if (axValIter->second.second[0] == '2') {
					offsetX = -50.f;
					offsetY = 10.f;
				}
				else {
					offsetX = -70.f;
					offsetY = -10.f;

					axValSize = swprintf_s(axVal, L"%1.5e", axValIter->second.first.first);
				}

				writeFactory->CreateTextLayout(axVal, axValSize, textFormat.Get(), axValSize * 10.f, 20.f, &textLayout);

				d2dDeviceContext->DrawTextLayout(
					D2D1::Point2F(scrVec.m128_f32[0] + offsetX, scrVec.m128_f32[1] + offsetY),
					textLayout.Get(),
					blackBrush.Get()
				);

				if (axValIter->second.second.size() > 1) {
					if (axValIter->second.second[0] == '1') {
						offsetX += 40.f;
						offsetY += 40.f;

						if (this->axis == 'X') {
							writeFactory->CreateTextLayout(L"Y", 1, textFormat.Get(), 10.f, 20.f, &textLayout);
						}
						else {
							writeFactory->CreateTextLayout(L"X", 1, textFormat.Get(), 10.f, 20.f, &textLayout);
						}
					}
					else if (axValIter->second.second[0] == '2') {
						offsetX += -30.f;
						offsetY += 20.f;

						if (this->axis == 'Z') {
							writeFactory->CreateTextLayout(L"Y", 1, textFormat.Get(), 10.f, 20.f, &textLayout);
						}
						else {
							writeFactory->CreateTextLayout(L"Z", 1, textFormat.Get(), 10.f, 20.f, &textLayout);
						}
					}
					else {
						offsetX += -50.f;
						offsetY += 20.f;

						writeFactory->CreateTextLayout(L"Result", 6, textFormat.Get(), 60.f, 20.f, &textLayout);
					}

					d2dDeviceContext->DrawTextLayout(
						D2D1::Point2F(scrVec.m128_f32[0] + offsetX, scrVec.m128_f32[1] + offsetY),
						textLayout.Get(),
						blackBrush.Get()
					);
				}

				++axValIter;
			}

			d2dDeviceContext->EndDraw();
		}


		this->swapChainResultDisplay->Present(1, 0);



		this->d3dDeviceContext->ClearRenderTargetView(renderTargetRelErrDisplay.Get(), color);
		this->d3dDeviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.f, 0u);
		this->d3dDeviceContext->IASetVertexBuffers(0, 1, vertexRelErrBuffer.GetAddressOf(), &stride, &offset);
		this->d3dDeviceContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		this->d3dDeviceContext->VSSetConstantBuffers(0, 1, constBuffer.GetAddressOf());

		this->d3dDeviceContext->VSSetShader(vertexShader.Get(), nullptr, 0);
		this->d3dDeviceContext->IASetInputLayout(inputLayout.Get());
		this->d3dDeviceContext->PSSetShader(pixelShader.Get(), nullptr, 0);

		this->d3dDeviceContext->OMSetDepthStencilState(depthStencilState.Get(), 1u);
		this->d3dDeviceContext->OMSetRenderTargets(1u, renderTargetRelErrDisplay.GetAddressOf(), depthStencilView.Get());
		this->d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		this->d3dDeviceContext->RSSetViewports(1u, &viewport);

		this->d3dDeviceContext->DrawIndexed(indices.size(), 0u, 0u);


		if (PlanePreview::gridActive) {
			this->d3dDeviceContext->IASetVertexBuffers(0, 1, gridVertexBuffer.GetAddressOf(), &stride, &offset);
			this->d3dDeviceContext->VSSetConstantBuffers(0, 1, constBuffer.GetAddressOf());

			this->d3dDeviceContext->VSSetShader(vertexShader.Get(), nullptr, 0);
			this->d3dDeviceContext->IASetInputLayout(inputLayout.Get());
			this->d3dDeviceContext->PSSetShader(pixelShader.Get(), nullptr, 0);

			this->d3dDeviceContext->OMSetDepthStencilState(depthStencilState.Get(), 1u);
			this->d3dDeviceContext->OMSetRenderTargets(1u, renderTargetRelErrDisplay.GetAddressOf(), depthStencilView.Get());

			this->d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
			this->d3dDeviceContext->RSSetViewports(1u, &viewport);

			this->d3dDeviceContext->Draw(gridLinesVertices.size(), 0u);
		}


		if (PlanePreview::axesValsActive) {
			d2dDeviceContext->SetTarget(d2dTargetBitmapRelErr.Get());

			d2dDeviceContext->BeginDraw();

			auto axValIter = gridAxesValuesVertices.begin();

			for (auto scrVec : gridAxesValScreenVertices) {
				axValSize = swprintf_s(axVal, L"%1.3Lf", axValIter->second.first.first);

				float offsetX = 0.f, offsetY = 0.f;

				if (axValIter->second.second[0] == '1') {
					offsetX = 20.f;
					offsetY = 0.f;
				}
				else if (axValIter->second.second[0] == '2') {
					offsetX = -50.f;
					offsetY = 10.f;
				}
				else {
					offsetX = -70.f;
					offsetY = -10.f;

					axValSize = swprintf_s(axVal, L"%1.5e", axValIter->second.first.second);
				}

				writeFactory->CreateTextLayout(axVal, axValSize, textFormat.Get(), axValSize * 10.f, 20.f, &textLayout);

				d2dDeviceContext->DrawTextLayout(
					D2D1::Point2F(scrVec.m128_f32[0] + offsetX, scrVec.m128_f32[1] + offsetY),
					textLayout.Get(),
					blackBrush.Get()
				);

				if (axValIter->second.second.size() > 1) {
					if (axValIter->second.second[0] == '1') {
						offsetX += 40.f;
						offsetY += 40.f;

						if (this->axis == 'X') {
							writeFactory->CreateTextLayout(L"Y", 1, textFormat.Get(), 10.f, 20.f, &textLayout);
						}
						else {
							writeFactory->CreateTextLayout(L"X", 1, textFormat.Get(), 10.f, 20.f, &textLayout);
						}
					}
					else if (axValIter->second.second[0] == '2') {
						offsetX += -30.f;
						offsetY += 20.f;

						if (this->axis == 'Z') {
							writeFactory->CreateTextLayout(L"Y", 1, textFormat.Get(), 10.f, 20.f, &textLayout);
						}
						else {
							writeFactory->CreateTextLayout(L"Z", 1, textFormat.Get(), 10.f, 20.f, &textLayout);
						}
					}
					else {
						offsetX += -60.f;
						offsetY += 20.f;

						writeFactory->CreateTextLayout(L"Rel Error", 9, textFormat.Get(), 90.f, 20.f, &textLayout);
					}

					d2dDeviceContext->DrawTextLayout(
						D2D1::Point2F(scrVec.m128_f32[0] + offsetX, scrVec.m128_f32[1] + offsetY),
						textLayout.Get(),
						blackBrush.Get()
					);
				}

				++axValIter;
			}

			d2dDeviceContext->EndDraw();
		}


		this->swapChainRelErrDisplay->Present(1, 0);



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


Graphics::CustomColor PlanePreview::getRelErrColor(long double resultValue, long double relerrValue) {
	if (resultValue == 0.L) {
		return { 1.f, 1.f, 1.f };
	}

	long double valLog = std::log10(relerrValue);
	long double levelColor;

	if (relerrValue >= this->relerrLegend[4].value) {
		levelColor = std::abs(std::log10(this->relerrLegend[5].value - valLog)) / std::abs(std::log10(this->relerrLegend[5].value) - std::log10(this->relerrLegend[4].value));
		return { 1.f - (float)levelColor, 0.f, 1.f };
	}
	else if (relerrValue >= this->relerrLegend[3].value) {
		levelColor = std::abs(std::log10(this->relerrLegend[4].value - valLog)) / std::abs(std::log10(this->relerrLegend[4].value) - std::log10(this->relerrLegend[3].value));
		return { 0.f, (float)levelColor, 1.f };
	}
	else if (relerrValue >= this->relerrLegend[2].value) {
		levelColor = std::abs(std::log10(this->relerrLegend[3].value - valLog)) / std::abs(std::log10(this->relerrLegend[3].value) - std::log10(this->relerrLegend[2].value));
		return { 0.f, 1.f, 1.f - (float)levelColor };
	}
	else if (relerrValue >= this->relerrLegend[1].value) {
		levelColor = std::abs(std::log10(this->relerrLegend[2].value - valLog)) / std::abs(std::log10(this->relerrLegend[2].value) - std::log10(this->relerrLegend[1].value));
		return { (float)levelColor, 1.f, 0.f };
	}
	else {
		levelColor = std::abs(std::log10(this->relerrLegend[1].value - valLog)) / std::abs(std::log10(this->relerrLegend[1].value) - (this->relerrLegend[0].value > 0.L) ? std::log10(this->relerrLegend[0].value) : std::numeric_limits<long double>::min_exponent10);
		return { 1.f, 1.f - (float)levelColor, 0.f };
	}
}


void PlanePreview::initDirect3D() {
	D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		D3D11_CREATE_DEVICE_BGRA_SUPPORT,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&this->d3dDevice,
		nullptr,
		&this->d3dDeviceContext
	);

	Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
	Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
	Microsoft::WRL::ComPtr<IDXGIFactory> dxgiFactory;

	this->d3dDevice.As(&dxgiDevice);
	dxgiDevice->GetAdapter(&dxgiAdapter);
	dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));


	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	swapChainDesc.BufferDesc.Width = 0;
	swapChainDesc.BufferDesc.Height = 0;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;

	swapChainDesc.OutputWindow = this->hVisMerWnd->getResultDisplay();
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	dxgiFactory->CreateSwapChain(dxgiDevice.Get(), &swapChainDesc, &this->swapChainResultDisplay);


	swapChainDesc.OutputWindow = this->hVisMerWnd->getRelErrDisplay();
	dxgiFactory->CreateSwapChain(dxgiDevice.Get(), &swapChainDesc, &this->swapChainRelErrDisplay);
}
