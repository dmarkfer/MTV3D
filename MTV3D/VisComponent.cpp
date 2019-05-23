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

		std::chrono::steady_clock::time_point last;
		float angle = std::chrono::duration<float>(std::chrono::steady_clock::now() - last).count();
		float c = std::sin(angle) / 2.f + 0.5f;
		float color[] = { c, c, 1.f, 1.f };

		this->d3dDeviceContext->ClearRenderTargetView(renderTarget.Get(), color);

		struct Vertex {
			float x, y, z;
			unsigned char r, g, b, a;
		};

		const Vertex vertices[] = {
			{ -1.f, -1.f, -1.f,   255, 255,   0, 1. },
			{ 1.f, -1.f, -1.f,      0, 255,   0, 1. },
			{ -1.f, 1.f, -1.f,      0,   0, 255, 1. },
			{ 1.f, 1.f, -1.f,     255, 255,   0, 1. },
			{ -1.f, -1.f, 1.f,    255,   0, 255, 1. },
			{ 1.f, -1.f, 1.f,       0, 255, 255, 1. },
			{ -1.f, 1.f, 1.f,       0,   0,   0, 1. },
			{ 1.f, 1.f, 1.f,      255, 255, 255, 1. }
		};

		Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));
		vertexBufferDesc.ByteWidth = sizeof(vertices);
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = sizeof(Vertex);

		D3D11_SUBRESOURCE_DATA vertexSubresourceData;
		ZeroMemory(&vertexSubresourceData, sizeof(D3D11_SUBRESOURCE_DATA));
		vertexSubresourceData.pSysMem = vertices;
		vertexSubresourceData.SysMemPitch = 0;
		vertexSubresourceData.SysMemSlicePitch = 0;

		this->d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, &vertexBuffer);
		const UINT stride = sizeof(Vertex);
		const UINT offset = 0;
		this->d3dDeviceContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);


		const unsigned short indices[] = {
			0,2,1, 2,3,1,
			1,3,5, 3,7,5,
			2,6,3, 3,6,7,
			4,5,7, 4,7,6,
			0,4,2, 2,4,6,
			0,1,4, 1,5,4
		};

		Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc, sizeof(D3D11_BUFFER_DESC));
		indexBufferDesc.ByteWidth = sizeof(indices);
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = sizeof(unsigned short);

		D3D11_SUBRESOURCE_DATA indexSubresourceData;
		ZeroMemory(&indexSubresourceData, sizeof(D3D11_SUBRESOURCE_DATA));
		indexSubresourceData.pSysMem = indices;
		indexSubresourceData.SysMemPitch = 0;
		indexSubresourceData.SysMemSlicePitch = 0;

		this->d3dDevice->CreateBuffer(&indexBufferDesc, &indexSubresourceData, &indexBuffer);
		this->d3dDeviceContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);


		struct ConstBufferStruct {
			DirectX::XMMATRIX transform;
		};
		const ConstBufferStruct cb = {
			{
				DirectX::XMMatrixTranspose(
					DirectX::XMMatrixRotationZ(angle) *
					DirectX::XMMatrixRotationX(angle) *
					DirectX::XMMatrixTranslation(0.f, 0.f, 4.f) *
					DirectX::XMMatrixPerspectiveLH(1.f, 3.f / 4.f, 0.5f, 10.f)
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
			{ "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "Color", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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


		this->d3dDeviceContext->OMSetRenderTargets(1u, renderTarget.GetAddressOf(), nullptr);

		this->d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


		D3D11_VIEWPORT vp;
		vp.Width = 800;
		vp.Height = 600;
		vp.MinDepth = 0;
		vp.MaxDepth = 1;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		this->d3dDeviceContext->RSSetViewports(1u, &vp);


		this->d3dDeviceContext->DrawIndexed((UINT)std::size(indices), 0u, 0u);
		
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
