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
LPWSTR VisComponent::appRootDir = nullptr;


VisComponent::VisComponent(
	Microsoft::WRL::ComPtr<ID3D11Device>& d3dDevice,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>& d3dDeviceContext
): d3dDevice(d3dDevice), d3dDeviceContext(d3dDeviceContext) {
	this->d3dDevice.As(&this->dxgiDevice);
}


void VisComponent::run(HINSTANCE hCurrentInst, HACCEL hAccelTable, int projectId, LPWSTR fileAbsolutePath, int n, Point* visPoints) {
	this->hCurrentInst = hCurrentInst;
	this->hAccelTable = hAccelTable;
	this->projectId = projectId;
	this->fileAbsolutePath = fileAbsolutePath;
	this->windowTitle = new WCHAR[WCHAR_ARR_MAX];
	swprintf_s(this->windowTitle, WCHAR_ARR_MAX - 1, L"MTV3D - %s", this->fileAbsolutePath);

	for (int i = 0; i < n; ++i) {
		this->visPoints.push_back(visPoints[i]);
	}
	delete[] visPoints;


	this->hVisMerWnd = std::make_unique<VisMergedWindow>(this->hCurrentInst, this->windowTitle);
	ShowWindow(this->hVisMerWnd->getHandle(), SW_SHOWMAXIMIZED);

	this->initDirect3D();
	
	Microsoft::WRL::ComPtr<ID3D11Resource> backBuffer;
	this->swapChain->GetBuffer(0, __uuidof(ID3D11Resource), &backBuffer);
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTarget;
	this->d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTarget);
	float color[] = { 1.f, 0.f, 0.f, 1.f };


	MSG msg;

	while (GetMessage(&msg, nullptr, 0, 0)) {
		if (!TranslateAccelerator(msg.hwnd, this->hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		this->d3dDeviceContext->ClearRenderTargetView(renderTarget.Get(), color);


		struct Vertex {
			float x, y;
			float r, g, b;
		};
		const Vertex vertices[] = {
			{ 0.0f, std::sqrt(3.f)/2.f-0.5f, 1.0f, 0.0f, 0.0f },
			{ 0.5f, -0.5f, 0.0f, 1.0f, 0.0f },
			{ -0.5f, -0.5f, 0.0f, 0.0f, 1.0f }
		};

		Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
		D3D11_BUFFER_DESC bufferDesc;
		ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
		bufferDesc.ByteWidth = sizeof(vertices);
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = sizeof(Vertex);

		D3D11_SUBRESOURCE_DATA subresourceData;
		ZeroMemory(&subresourceData, sizeof(D3D11_SUBRESOURCE_DATA));
		subresourceData.pSysMem = vertices;
		subresourceData.SysMemPitch = 0;
		subresourceData.SysMemSlicePitch = 0;

		this->d3dDevice->CreateBuffer(&bufferDesc, &subresourceData, &vertexBuffer);
		const UINT stride = sizeof(Vertex);
		const UINT offset = 0;
		this->d3dDeviceContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);


		std::ifstream ifStreamShader((std::wstring(VisComponent::appRootDir) + L"\\VertexShader.cso"), std::ifstream::in | std::ifstream::binary);
		ifStreamShader.seekg(0, std::ios::end);
		unsigned shaderFileSize = (unsigned)ifStreamShader.tellg();
		std::unique_ptr<char[]> shaderBlob = std::make_unique<char[]>(shaderFileSize);
		ifStreamShader.seekg(0, std::ios::beg);
		ifStreamShader.read(shaderBlob.get(), shaderFileSize);
		ifStreamShader.close();

		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
		this->d3dDevice->CreateVertexShader(shaderBlob.get(), shaderFileSize, nullptr, &vertexShader);
		this->d3dDeviceContext->VSSetShader(vertexShader.Get(), nullptr, 0);

		Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
		const D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
			{ "Position", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "Color", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		this->d3dDevice->CreateInputLayout(inputElementDesc, (UINT)std::size(inputElementDesc), shaderBlob.get(), shaderFileSize, &inputLayout);
		this->d3dDeviceContext->IASetInputLayout(inputLayout.Get());

		ifStreamShader.open((std::wstring(VisComponent::appRootDir) + L"\\PixelShader.cso"), std::ifstream::in | std::ifstream::binary);
		ifStreamShader.seekg(0, std::ios::end);
		shaderFileSize = (unsigned)ifStreamShader.tellg();
		shaderBlob = std::make_unique<char[]>(shaderFileSize);
		ifStreamShader.seekg(0, std::ios::beg);
		ifStreamShader.read(shaderBlob.get(), shaderFileSize);
		ifStreamShader.close();

		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
		this->d3dDevice->CreatePixelShader(shaderBlob.get(), shaderFileSize, nullptr, &pixelShader);
		this->d3dDeviceContext->PSSetShader(pixelShader.Get(), nullptr, 0);



		this->d3dDeviceContext->OMSetRenderTargets(1, renderTarget.GetAddressOf(), nullptr);

		this->d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


		D3D11_VIEWPORT viewport;
		ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = 300;
		viewport.Height = 300;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1;
		this->d3dDeviceContext->RSSetViewports(1, &viewport);


		this->d3dDeviceContext->Draw((UINT)std::size(vertices), 0);
		this->swapChain->Present(1, 0);
	}

	delete[] this->windowTitle;
	
	int* customLParam = new int;
	*customLParam = this->projectId;
	PostThreadMessage(VisComponent::mainThreadId, WM_THREAD_DONE, 0, (LPARAM)customLParam);
}

LRESULT CALLBACK VisComponent::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	WndClass::Type wcType = WndClass::typeByWndHandle(hWnd);

	switch (message) {
	case WM_SETCURSOR: {
		if (wcType == WndClass::Type::VIS_DISPLAY) {
			SetCursor(LoadCursor(nullptr, IDC_HAND));
		}
		else {
			SetCursor(LoadCursor(nullptr, IDC_ARROW));
		}
		return true;
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
	ZeroMemory(&this->swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	this->swapChainDesc.BufferDesc.Width = 0;
	this->swapChainDesc.BufferDesc.Height = 0;
	this->swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	this->swapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
	this->swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	this->swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	this->swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	this->swapChainDesc.SampleDesc.Count = 1;
	this->swapChainDesc.SampleDesc.Quality = 0;

	this->swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	this->swapChainDesc.BufferCount = 1;

	this->swapChainDesc.OutputWindow = this->hVisMerWnd->getResultDisplay();
	this->swapChainDesc.Windowed = TRUE;
	this->swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	this->swapChainDesc.Flags = 0;


	this->dxgiDevice->GetAdapter(&this->dxgiAdapter);
	this->dxgiAdapter->GetParent(IID_PPV_ARGS(&this->dxgiFactory));

	dxgiFactory->CreateSwapChain(dxgiDevice.Get(), &this->swapChainDesc, &this->swapChain);
}
