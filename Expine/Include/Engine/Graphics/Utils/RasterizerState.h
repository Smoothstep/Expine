#pragma once

#include "D3D.h"

namespace D3D
{
	namespace RasterizerState
	{
		extern const D3D12_RASTERIZER_DESC Default;
		extern const D3D12_RASTERIZER_DESC Wireframe;
		extern const D3D12_RASTERIZER_DESC CullNone;
		extern const D3D12_RASTERIZER_DESC CullFront;
	}
}