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
#include "Graphics.h"


unsigned Graphics::vertexShaderFileSize = 0;
char* Graphics::vertexShaderBlob = nullptr;

const D3D11_INPUT_ELEMENT_DESC Graphics::inputElementDesc[] = {
	{ "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "Color", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

unsigned Graphics::pixelShaderFileSize = 0;
char* Graphics::pixelShaderBlob = nullptr;


std::optional<float> Graphics::validFloat(std::wstring numberWStr) {
	if (numberWStr.empty()) {
		return {};
	}

	try {
		return { stof(numberWStr, nullptr) };
	}
	catch (...) {
		MessageBox(nullptr, L"Invalid input!", nullptr, MB_ICONERROR);

		return {};
	}
}
