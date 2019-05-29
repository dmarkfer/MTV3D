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
#include "VisComponent.h"


DWORD VisComponent::mainThreadId = 0;
HCURSOR VisComponent::cursorHandNoGrab = nullptr;
HCURSOR VisComponent::cursorHandGrab = nullptr;
HWND VisComponent::cursorGrabInteractionProject = nullptr;
int VisComponent::clickPosX = 0;
int VisComponent::clickPosY = 0;
float VisComponent::scaleBase = 1.f;

unsigned VisComponent::vertexShaderFileSize = 0;
char* VisComponent::vertexShaderBlob = nullptr;
const D3D11_INPUT_ELEMENT_DESC VisComponent::inputElementDesc[] = {
	{ "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "Color", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};
unsigned VisComponent::pixelShaderFileSize = 0;
char* VisComponent::pixelShaderBlob = nullptr;


void VisComponent::run(HINSTANCE hCurrentInst, HACCEL hAccelTable, int projectId, LPWSTR fileAbsolutePath, int visPointsDataSize, Point* visPointsData) {
	VisComponent::scaleBase = 1.f;

	this->hCurrentInst = hCurrentInst;
	this->hAccelTable = hAccelTable;
	this->projectId = projectId;
	this->fileAbsolutePath = fileAbsolutePath;
	this->windowTitle = new WCHAR[WCHAR_ARR_MAX];
	swprintf_s(this->windowTitle, WCHAR_ARR_MAX - 1, L"MTV3D - %s", this->fileAbsolutePath);

	this->hVisMerWnd = std::make_unique<VisMergedWindow>(this->hCurrentInst, this->windowTitle);
	ShowWindow(this->hVisMerWnd->getHandle(), SW_SHOWMAXIMIZED);

	SetCursor(LoadCursor(nullptr, IDC_WAIT));

	std::set<float> axisXValues;
	std::set<float> axisYValues;
	std::set<float> axisZValues;

	std::vector<std::vector<VisComponent::Point>> visPlaneModel;
	std::vector<VisComponent::Point> visLineModel;

	long double resultMinValue = 0.L, resultMaxValue = 0.L, relerrMinValue = 0.L, relerrMaxValue = 0.L;

	for (int i = 0; i < visPointsDataSize; ++i) {
		axisXValues.insert(visPointsData[i].x);
		axisYValues.insert(visPointsData[i].y);
		axisZValues.insert(visPointsData[i].z);

		VisComponent::Point visPoint = {
			visPointsData[i].x, visPointsData[i].y, visPointsData[i].z,
			visPointsData[i].value, visPointsData[i].relError
		};

		if (i > 0) {
			if (visPointsData[i].y != visPointsData[i - 1].y) {
				visPlaneModel.push_back(visLineModel);

				visLineModel.clear();
			}
			if (visPointsData[i].x != visPointsData[i - 1].x) {
				vis3DDataModel.push_back(visPlaneModel);

				visPlaneModel.clear();
			}

			if (visPoint.value > 0.) {
				if (visPoint.value < resultMinValue) resultMinValue = visPoint.value;
				if (visPoint.value > resultMaxValue) resultMaxValue = visPoint.value;
				if (visPoint.relError < relerrMinValue) relerrMinValue = visPoint.relError;
				if (visPoint.relError > relerrMaxValue) relerrMaxValue = visPoint.relError;
			}
		}
		else {
			resultMinValue = visPoint.value;
			resultMaxValue = visPoint.value;
			relerrMinValue = visPoint.relError;
			relerrMaxValue = visPoint.relError;
		}

		visLineModel.push_back(visPoint);
	}
	visPlaneModel.push_back(visLineModel);
	vis3DDataModel.push_back(visPlaneModel);
	visPlaneModel.clear();
	visLineModel.clear();
	delete[] visPointsData;


	long double resultMinValueLog10 = std::log10(resultMinValue);
	long double resultMaxValueLog10 = std::log10(resultMaxValue);
	long double resultLogFifth = (resultMaxValueLog10 - resultMinValueLog10) / 5.L;
	if (resultLogFifth < 0.L) {
		resultLogFifth = -resultLogFifth;
	}

	this->resultLegend.push_back({ 1.f, 0.f, 1.f, resultMinValue });
	this->resultLegend.push_back({ 0.f, 0.f, 1.f, std::pow(10, resultMinValueLog10 + resultLogFifth) });
	this->resultLegend.push_back({ 0.f, 1.f, 1.f, std::pow(10, resultMinValueLog10 + 2. * resultLogFifth) });
	this->resultLegend.push_back({ 0.f, 1.f, 0.f, std::pow(10, resultMinValueLog10 + 3. * resultLogFifth) });
	this->resultLegend.push_back({ 1.f, 1.f, 0.f, std::pow(10, resultMinValueLog10 + 4. * resultLogFifth) });
	this->resultLegend.push_back({ 1.f, 0.f, 0.f, resultMaxValue });


	long double relerrMinValueLog10 = std::log10(relerrMinValue);
	long double relerrMaxValueLog10 = std::log10(relerrMaxValue);
	long double relerrLogHalf = (relerrMaxValueLog10 - relerrMinValueLog10) / 2.L;

	this->relerrLegend.push_back({ 0.f, 0.f, 1.f, relerrMinValue });
	this->relerrLegend.push_back({ 1.f, 0.f, 1.f, std::pow(10, relerrMinValueLog10 + relerrLogHalf) });
	this->relerrLegend.push_back({ 1.f, 0.f, 0.f, relerrMaxValue });


	int axisXSize = axisXValues.size();
	int axisYSize = axisYValues.size();
	int axisZSize = axisZValues.size();

	float modelAbscissaLength = std::abs(vis3DDataModel[axisXSize - 1][axisYSize - 1][axisZSize - 1].x - vis3DDataModel[0][0][0].x);
	float modelOrdinateLength = std::abs(vis3DDataModel[axisXSize - 1][axisYSize - 1][axisZSize - 1].y - vis3DDataModel[0][0][0].y);
	float modelApplicateLength = std::abs(vis3DDataModel[axisXSize - 1][axisYSize - 1][axisZSize - 1].z - vis3DDataModel[0][0][0].z);

	float modelAbscissaCenter = std::vector<float>(axisXValues.begin(), axisXValues.end())[axisXSize / 2];
	float modelOrdinateCenter = std::vector<float>(axisYValues.begin(), axisYValues.end())[axisYSize / 2];
	float modelApplicateCenter = std::vector<float>(axisZValues.begin(), axisZValues.end())[axisZSize / 2];

	float diagonal3DLength = std::sqrt(modelAbscissaLength * modelAbscissaLength + modelOrdinateLength * modelOrdinateLength + modelApplicateLength * modelApplicateLength);

	SetCursor(LoadCursor(nullptr, IDC_ARROW));

	this->initDirect3D();

	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;

	D3D11_VIEWPORT vp;
	vp.Width = (float)this->hVisMerWnd->getDisplayDim();
	vp.Height = (float)this->hVisMerWnd->getDisplayDim();
	vp.MinDepth = 0.f;
	vp.MaxDepth = 1.f;
	vp.TopLeftX = 0.f;
	vp.TopLeftY = 0.f;

	
	Microsoft::WRL::ComPtr<ID3D11Resource> backBufferResultDisplay;
	this->swapChainResultDisplay->GetBuffer(0, __uuidof(ID3D11Resource), &backBufferResultDisplay);
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetResultDisplay;
	this->d3dDevice->CreateRenderTargetView(backBufferResultDisplay.Get(), nullptr, &renderTargetResultDisplay);

	Microsoft::WRL::ComPtr<ID3D11Resource> backBufferRelErrDisplay;
	this->swapChainRelErrDisplay->GetBuffer(0, __uuidof(ID3D11Resource), &backBufferRelErrDisplay);
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetRelErrDisplay;
	this->d3dDevice->CreateRenderTargetView(backBufferRelErrDisplay.Get(), nullptr, &renderTargetRelErrDisplay);


	for (int i = 0; i < axisXSize; ++i) {
		for (int j = 0; j < axisYSize; ++j) {
			Point visp = vis3DDataModel[i][j][0];
			visp.x -= modelAbscissaCenter;
			visp.y -= modelOrdinateCenter;
			visp.z -= modelApplicateCenter;
			vertices.push_back({ visp.x, visp.y, visp.z, getResultColor(visp.value) });

			if (i > 0 && j > 0) {
				indices.push_back((i - 1) * axisXSize + j - 1);
				indices.push_back((i - 1) * axisXSize + j);
				indices.push_back(i * axisXSize + j - 1);

				indices.push_back((i - 1) * axisXSize + j - 1);
				indices.push_back(i * axisXSize + j - 1);
				indices.push_back((i - 1) * axisXSize + j);

				indices.push_back((i - 1) * axisXSize + j);
				indices.push_back(i * axisXSize + j);
				indices.push_back(i * axisXSize + j - 1);

				indices.push_back((i - 1) * axisXSize + j);
				indices.push_back(i * axisXSize + j - 1);
				indices.push_back(i * axisXSize + j);
			}
		}
	}

	/*int startIndex = vertices.size();

	for (int i = 0; i < axisXSize; ++i) {
		for (int j = 0; j < axisYSize; ++j) {
			Point visp = vis3DDataModel[i][j][axisZSize - 1];
			visp.x -= modelAbscissaCenter;
			visp.y -= modelOrdinateCenter;
			visp.z -= modelApplicateCenter;
			vertices.push_back({ visp.x, visp.y, visp.z, getResultColor(visp.value) });

			if (i > 0 && j > 0) {
				indices.push_back(startIndex + (i - 1) * axisXSize + j - 1);
				indices.push_back(startIndex + (i - 1) * axisXSize + j);
				indices.push_back(startIndex + i * axisXSize + j - 1);

				indices.push_back(startIndex + (i - 1) * axisXSize + j - 1);
				indices.push_back(startIndex + i * axisXSize + j - 1);
				indices.push_back(startIndex + (i - 1) * axisXSize + j);

				indices.push_back(startIndex + (i - 1) * axisXSize + j);
				indices.push_back(startIndex + i * axisXSize + j);
				indices.push_back(startIndex + i * axisXSize + j - 1);

				indices.push_back(startIndex + (i - 1) * axisXSize + j);
				indices.push_back(startIndex + i * axisXSize + j - 1);
				indices.push_back(startIndex + i * axisXSize + j);
			}
		}
	}

	startIndex = vertices.size();

	for (int k = 0; k < axisZSize; ++k) {
		for (int i = 0; i < axisXSize; ++i) {
			Point visp = vis3DDataModel[i][0][k];
			visp.x -= modelAbscissaCenter;
			visp.y -= modelOrdinateCenter;
			visp.z -= modelApplicateCenter;
			vertices.push_back({ visp.x, visp.y, visp.z, getResultColor(visp.value) });

			if (k > 0 && i > 0) {
				indices.push_back(startIndex + (k - 1) * axisXSize + i - 1);
				indices.push_back(startIndex + (k - 1) * axisXSize + i);
				indices.push_back(startIndex + k * axisXSize + i - 1);

				indices.push_back(startIndex + (k - 1) * axisXSize + i - 1);
				indices.push_back(startIndex + k * axisXSize + i - 1);
				indices.push_back(startIndex + (k - 1) * axisXSize + i);

				indices.push_back(startIndex + (k - 1) * axisXSize + i);
				indices.push_back(startIndex + k * axisXSize + i);
				indices.push_back(startIndex + k * axisXSize + i - 1);

				indices.push_back(startIndex + (k - 1) * axisXSize + i);
				indices.push_back(startIndex + k * axisXSize + i - 1);
				indices.push_back(startIndex + k * axisXSize + i);
			}
		}
	}

	startIndex = vertices.size();

	for (int k = 0; k < axisZSize; ++k) {
		for (int i = 0; i < axisXSize; ++i) {
			Point visp = vis3DDataModel[i][axisYSize - 1][k];
			visp.x -= modelAbscissaCenter;
			visp.y -= modelOrdinateCenter;
			visp.z -= modelApplicateCenter;
			vertices.push_back({ visp.x, visp.y, visp.z, getResultColor(visp.value) });

			if (k > 0 && i > 0) {
				indices.push_back(startIndex + (k - 1) * axisXSize + i - 1);
				indices.push_back(startIndex + (k - 1) * axisXSize + i);
				indices.push_back(startIndex + k * axisXSize + i - 1);

				indices.push_back(startIndex + (k - 1) * axisXSize + i - 1);
				indices.push_back(startIndex + k * axisXSize + i - 1);
				indices.push_back(startIndex + (k - 1) * axisXSize + i);

				indices.push_back(startIndex + (k - 1) * axisXSize + i);
				indices.push_back(startIndex + k * axisXSize + i);
				indices.push_back(startIndex + k * axisXSize + i - 1);

				indices.push_back(startIndex + (k - 1) * axisXSize + i);
				indices.push_back(startIndex + k * axisXSize + i - 1);
				indices.push_back(startIndex + k * axisXSize + i);
			}
		}
	}

	startIndex = vertices.size();

	for (int j = 0; j < axisYSize; ++j) {
		for (int k = 0; k < axisZSize; ++k) {
			Point visp = vis3DDataModel[0][j][k];
			visp.x -= modelAbscissaCenter;
			visp.y -= modelOrdinateCenter;
			visp.z -= modelApplicateCenter;
			vertices.push_back({ visp.x, visp.y, visp.z, getResultColor(visp.value) });

			if (j > 0 && k > 0) {
				indices.push_back(startIndex + (j - 1) * axisXSize + k - 1);
				indices.push_back(startIndex + (j - 1) * axisXSize + k);
				indices.push_back(startIndex + j * axisXSize + k - 1);

				indices.push_back(startIndex + (j - 1) * axisXSize + k - 1);
				indices.push_back(startIndex + j * axisXSize + k - 1);
				indices.push_back(startIndex + (j - 1) * axisXSize + k);

				indices.push_back(startIndex + (j - 1) * axisXSize + k);
				indices.push_back(startIndex + j * axisXSize + k);
				indices.push_back(startIndex + j * axisXSize + k - 1);

				indices.push_back(startIndex + (j - 1) * axisXSize + k);
				indices.push_back(startIndex + j * axisXSize + k - 1);
				indices.push_back(startIndex + j * axisXSize + k);
			}
		}
	}

	startIndex = vertices.size();

	for (int j = 0; j < axisYSize; ++j) {
		for (int k = 0; k < axisZSize; ++k) {
			Point visp = vis3DDataModel[axisXSize - 1][j][k];
			visp.x -= modelAbscissaCenter;
			visp.y -= modelOrdinateCenter;
			visp.z -= modelApplicateCenter;
			vertices.push_back({ visp.x, visp.y, visp.z, getResultColor(visp.value) });

			if (j > 0 && k > 0) {
				indices.push_back(startIndex + (j - 1) * axisXSize + k - 1);
				indices.push_back(startIndex + (j - 1) * axisXSize + k);
				indices.push_back(startIndex + j * axisXSize + k - 1);

				indices.push_back(startIndex + (j - 1) * axisXSize + k - 1);
				indices.push_back(startIndex + j * axisXSize + k - 1);
				indices.push_back(startIndex + (j - 1) * axisXSize + k);

				indices.push_back(startIndex + (j - 1) * axisXSize + k);
				indices.push_back(startIndex + j * axisXSize + k);
				indices.push_back(startIndex + j * axisXSize + k - 1);

				indices.push_back(startIndex + (j - 1) * axisXSize + k);
				indices.push_back(startIndex + j * axisXSize + k - 1);
				indices.push_back(startIndex + j * axisXSize + k);
			}
		}
	}*/


	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * vertices.size();
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = sizeof(Vertex);

	D3D11_SUBRESOURCE_DATA vertexSubresourceData;
	ZeroMemory(&vertexSubresourceData, sizeof(D3D11_SUBRESOURCE_DATA));
	vertexSubresourceData.pSysMem = &vertices[0];
	vertexSubresourceData.SysMemPitch = 0;
	vertexSubresourceData.SysMemSlicePitch = 0;

	this->d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, &vertexBuffer);

	const UINT stride = sizeof(Vertex);
	const UINT offset = 0;


	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	indexBufferDesc.ByteWidth = sizeof(unsigned short) * indices.size();
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = sizeof(unsigned short);

	D3D11_SUBRESOURCE_DATA indexSubresourceData;
	ZeroMemory(&indexSubresourceData, sizeof(D3D11_SUBRESOURCE_DATA));
	indexSubresourceData.pSysMem = &indices[0];
	indexSubresourceData.SysMemPitch = 0;
	indexSubresourceData.SysMemSlicePitch = 0;

	this->d3dDevice->CreateBuffer(&indexBufferDesc, &indexSubresourceData, &indexBuffer);


	DirectX::XMVECTOR eyePosition = DirectX::XMVectorSet(diagonal3DLength, diagonal3DLength, diagonal3DLength, 0.f);
	DirectX::XMVECTOR focusPosition = DirectX::XMVectorSet(0.f, 0.f, 0.f, 0.f);
	DirectX::XMVECTOR upDirection = DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f);
	DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationX(0.f);


	MSG msg;
	bool quitFlag = false;

	while (true) {
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				quitFlag = true;
				break;
			}

			if (! TranslateAccelerator(msg.hwnd, this->hAccelTable, &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		if (quitFlag) {
			break;
		}


		
		HDC hdc = GetDC(this->hVisMerWnd->getResultLegend());

		RECT fillRect;
		GetClientRect(this->hVisMerWnd->getResultLegend(), &fillRect);

		for (int i = 0; i < fillRect.bottom; ++i) {
			CustomColor colHigh;
			CustomColor colLow;
			CustomColor fillCol;

			if (i < fillRect.bottom / 5) {
				colHigh = this->resultLegend[5].color;
				colLow = this->resultLegend[4].color;

				fillCol.r = colLow.r + (colHigh.r - colLow.r) * (fillRect.bottom / 5 - i) / (fillRect.bottom / 5);
				fillCol.g = colLow.g + (colHigh.g - colLow.g) * (fillRect.bottom / 5 - i) / (fillRect.bottom / 5);
				fillCol.b = colLow.b + (colHigh.b - colLow.b) * (fillRect.bottom / 5 - i) / (fillRect.bottom / 5);
			}
			else if (i < fillRect.bottom * 2 / 5) {
				colHigh = this->resultLegend[4].color;
				colLow = this->resultLegend[3].color;

				fillCol.r = colLow.r + (colHigh.r - colLow.r) * (fillRect.bottom * 2 / 5 - i) / (fillRect.bottom / 5);
				fillCol.g = colLow.g + (colHigh.g - colLow.g) * (fillRect.bottom * 2 / 5 - i) / (fillRect.bottom / 5);
				fillCol.b = colLow.b + (colHigh.b - colLow.b) * (fillRect.bottom * 2 / 5 - i) / (fillRect.bottom / 5);
			}
			else if (i < fillRect.bottom * 3 / 5) {
				colHigh = this->resultLegend[3].color;
				colLow = this->resultLegend[2].color;

				fillCol.r = colLow.r + (colHigh.r - colLow.r) * (fillRect.bottom * 3 / 5 - i) / (fillRect.bottom / 5);
				fillCol.g = colLow.g + (colHigh.g - colLow.g) * (fillRect.bottom * 3 / 5 - i) / (fillRect.bottom / 5);
				fillCol.b = colLow.b + (colHigh.b - colLow.b) * (fillRect.bottom * 3 / 5 - i) / (fillRect.bottom / 5);
			}
			else if (i < fillRect.bottom * 4 / 5) {
				colHigh = this->resultLegend[2].color;
				colLow = this->resultLegend[1].color;

				fillCol.r = colLow.r + (colHigh.r - colLow.r) * (fillRect.bottom * 4 / 5 - i) / (fillRect.bottom / 5);
				fillCol.g = colLow.g + (colHigh.g - colLow.g) * (fillRect.bottom * 4 / 5 - i) / (fillRect.bottom / 5);
				fillCol.b = colLow.b + (colHigh.b - colLow.b) * (fillRect.bottom * 4 / 5 - i) / (fillRect.bottom / 5);
			}
			else {
				colHigh = this->resultLegend[1].color;
				colLow = this->resultLegend[0].color;

				fillCol.r = colLow.r + (colHigh.r - colLow.r) * (fillRect.bottom - i) / (fillRect.bottom / 5 + 1);
				fillCol.g = colLow.g + (colHigh.g - colLow.g) * (fillRect.bottom - i) / (fillRect.bottom / 5 + 1);
				fillCol.b = colLow.b + (colHigh.b - colLow.b) * (fillRect.bottom - i) / (fillRect.bottom / 5 + 1);
			}

			for (int j = 0; j < fillRect.right; ++j) {
				SetPixel(hdc, j, i, RGB(fillCol.r * 255, fillCol.g * 255, fillCol.b * 255));
			}
		}

		DeleteDC(hdc);


		hdc = GetDC(this->hVisMerWnd->getRelErrLegend());

		GetClientRect(this->hVisMerWnd->getRelErrLegend(), &fillRect);

		for (int i = 0; i < fillRect.bottom; ++i) {
			CustomColor colHigh;
			CustomColor colLow;
			CustomColor fillCol;

			if (i < fillRect.bottom / 2) {
				colHigh = this->relerrLegend[2].color;
				colLow = this->relerrLegend[1].color;

				fillCol.r = colLow.r + (colHigh.r - colLow.r) * (fillRect.bottom / 2 - i) / (fillRect.bottom / 2);
				fillCol.g = colLow.g + (colHigh.g - colLow.g) * (fillRect.bottom / 2 - i) / (fillRect.bottom / 2);
				fillCol.b = colLow.b + (colHigh.b - colLow.b) * (fillRect.bottom / 2 - i) / (fillRect.bottom / 2);
			}
			else {
				colHigh = this->relerrLegend[1].color;
				colLow = this->relerrLegend[0].color;

				fillCol.r = colLow.r + (colHigh.r - colLow.r) * (fillRect.bottom - i) / (fillRect.bottom / 2 + 1);
				fillCol.g = colLow.g + (colHigh.g - colLow.g) * (fillRect.bottom - i) / (fillRect.bottom / 2 + 1);
				fillCol.b = colLow.b + (colHigh.b - colLow.b) * (fillRect.bottom - i) / (fillRect.bottom / 2 + 1);
			}

			for (int j = 0; j < fillRect.right; ++j) {
				SetPixel(hdc, j, i, RGB(fillCol.r * 255, fillCol.g * 255, fillCol.b * 255));
			}
		}

		DeleteDC(hdc);



		float color[] = { 0.f, 0.f, 0.f, 1.f };

		this->d3dDeviceContext->ClearRenderTargetView(renderTargetResultDisplay.Get(), color);
		this->d3dDeviceContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
		this->d3dDeviceContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);


		if (cursorGrabInteractionProject == this->hVisMerWnd->getResultDisplay() || cursorGrabInteractionProject == this->hVisMerWnd->getRelErrDisplay()) {
			POINT cursorPosition;
			GetCursorPos(&cursorPosition);
			ScreenToClient(this->hVisMerWnd->getResultDisplay(), &cursorPosition);
			RECT wndClientArea;
			GetClientRect(this->hVisMerWnd->getResultDisplay(), &wndClientArea);

			ScreenVector clickScreenVector = {
				VisComponent::clickPosX - wndClientArea.right / 2.f,
				- (VisComponent::clickPosY - wndClientArea.bottom / 2.f)
			};
			ScreenVector cursorScreenVector = {
				cursorPosition.x - wndClientArea.right / 2.f,
				- (cursorPosition.y - wndClientArea.bottom / 2.f)
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

			DirectX::XMVECTOR crossProd = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(eyePosition, upDirection));
			DirectX::XMVECTOR screenUpDirection = DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f);
			float screenAzimut = DirectX::XMVector2AngleBetweenVectors(screenUpDirection, cursorVector).m128_f32[0];
			if (cursorScreenVector.x >= 0) {
				screenAzimut = DirectX::XMConvertToRadians(360) - screenAzimut;
			}
			DirectX::XMVECTOR rotQuat = DirectX::XMQuaternionRotationAxis(eyePosition, - screenAzimut);
			DirectX::XMVECTOR rotationAxis = DirectX::XMVector3Rotate(crossProd, rotQuat);

			rotationMatrix *= DirectX::XMMatrixRotationAxis(eyePosition, angleClickToCurrentCursor) * DirectX::XMMatrixRotationAxis(rotationAxis, rotationAngle);

			VisComponent::clickPosX = cursorPosition.x;
			VisComponent::clickPosY = cursorPosition.y;
		}

		const ConstBufferStruct cb = {
			{
				DirectX::XMMatrixTranspose(
					rotationMatrix *
					DirectX::XMMatrixScaling(VisComponent::scaleBase, VisComponent::scaleBase, VisComponent::scaleBase) *
					DirectX::XMMatrixLookAtLH(eyePosition, focusPosition, upDirection) *
					DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(70), 1.f, 1.f, 3.f * diagonal3DLength)
				)
			}
		};

		Microsoft::WRL::ComPtr<ID3D11Buffer> constBuffer;

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

		this->d3dDevice->CreateVertexShader(VisComponent::vertexShaderBlob, VisComponent::vertexShaderFileSize, nullptr, &vertexShader);
		this->d3dDeviceContext->VSSetShader(vertexShader.Get(), nullptr, 0);
		this->d3dDevice->CreateInputLayout(VisComponent::inputElementDesc, (UINT)std::size(VisComponent::inputElementDesc), VisComponent::vertexShaderBlob, VisComponent::vertexShaderFileSize, &inputLayout);
		this->d3dDeviceContext->IASetInputLayout(inputLayout.Get());
		this->d3dDevice->CreatePixelShader(VisComponent::pixelShaderBlob, VisComponent::pixelShaderFileSize, nullptr, &pixelShader);
		this->d3dDeviceContext->PSSetShader(pixelShader.Get(), nullptr, 0);

		this->d3dDeviceContext->OMSetRenderTargets(1u, renderTargetResultDisplay.GetAddressOf(), nullptr);
		this->d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		this->d3dDeviceContext->RSSetViewports(1u, &vp);

		this->d3dDeviceContext->DrawIndexed(indices.size(), 0u, 0u);
		
		this->swapChainResultDisplay->Present(1, 0);



		this->d3dDeviceContext->ClearRenderTargetView(renderTargetRelErrDisplay.Get(), color);
		this->d3dDeviceContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
		this->d3dDeviceContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
		this->d3dDeviceContext->VSSetConstantBuffers(0, 1, constBuffer.GetAddressOf());

		this->d3dDeviceContext->VSSetShader(vertexShader.Get(), nullptr, 0);
		this->d3dDeviceContext->IASetInputLayout(inputLayout.Get());
		this->d3dDeviceContext->PSSetShader(pixelShader.Get(), nullptr, 0);

		this->d3dDeviceContext->OMSetRenderTargets(1u, renderTargetRelErrDisplay.GetAddressOf(), nullptr);
		this->d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		this->d3dDeviceContext->RSSetViewports(1u, &vp);

		this->d3dDeviceContext->DrawIndexed(indices.size(), 0u, 0u);

		this->swapChainRelErrDisplay->Present(1, 0);
	}

	delete[] this->windowTitle;
	
	int* customLParam = new int;
	*customLParam = this->projectId;
	PostThreadMessage(VisComponent::mainThreadId, WM_THREAD_DONE, 0, (LPARAM)customLParam);
}

LRESULT CALLBACK VisComponent::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	WndClass::Type wcType = WndClass::typeByWndHandle(hWnd);

	switch (message) {
	case WM_MOUSEWHEEL: {
		WORD fwKeys = GET_KEYSTATE_WPARAM(wParam);
		WORD zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		WORD xPos = GET_X_LPARAM(lParam);
		WORD yPos = GET_Y_LPARAM(lParam);

		if (zDelta == 120) {
			scaleBase *= 1.05f;
		}
		else {
			scaleBase *= 0.95f;
		}

		return 0;
		break;
	}
	case WM_SETCURSOR: {
		if (wcType == WndClass::Type::VIS_DISPLAY) {
			SetCursor(VisComponent::cursorGrabInteractionProject == hWnd ? VisComponent::cursorHandGrab : VisComponent::cursorHandNoGrab);
		}
		else {
			VisComponent::cursorGrabInteractionProject = nullptr;
			SetCursor(LoadCursor(nullptr, IDC_ARROW));
		}
		return true;
		break;
	}
	case WM_LBUTTONDOWN: {
		if (wcType == WndClass::Type::VIS_DISPLAY) {
			VisComponent::cursorGrabInteractionProject = hWnd;
			SetCursor(VisComponent::cursorHandGrab);
			VisComponent::clickPosX = GET_X_LPARAM(lParam);
			VisComponent::clickPosY = GET_Y_LPARAM(lParam);
		}
		break;
	}
	case WM_LBUTTONUP: {
		if (wcType == WndClass::Type::VIS_DISPLAY) {
			VisComponent::cursorGrabInteractionProject = nullptr;
			SetCursor(VisComponent::cursorHandNoGrab);
		}
		break;
	}
	case WM_PAINT: {
		switch (wcType) {
		case WndClass::Type::VIS_LEGEND: {
			break;
		}
		}
		break;
	}
	case WM_DESTROY: {
		switch (wcType) {
		case WndClass::Type::VIS_RESULT: {
			break;
		}
		case WndClass::Type::VIS_RELERR: {
			break;
		}
		case WndClass::Type::VIS_MERGED: {
			PostQuitMessage(0);
		}
		}
	}
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}


void VisComponent::initDirect3D() {
	D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
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


VisComponent::CustomColor VisComponent::getResultColor(long double resultValue) {
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


VisComponent::CustomColor VisComponent::getRelErrColor(long double relerrValue) {
	long double valLog = std::log10(relerrValue);
	long double levelColor;

	if (valLog <= std::log10(this->relerrLegend[1].value)) {
		levelColor = std::abs(valLog - std::log10(this->relerrLegend[0].value)) / std::abs(std::log10(this->relerrLegend[1].value) - std::log10(this->relerrLegend[0].value));
		return { (float)levelColor, 0.f, 1.f };
	}
	else {
		levelColor = std::abs(valLog - std::log10(this->relerrLegend[1].value)) / std::abs(std::log10(this->relerrLegend[2].value) - std::log10(this->relerrLegend[1].value));
		return { 1.f, 0.f, (float)levelColor };
	}
}
