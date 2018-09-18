#include "Precompiled.h"

#include "Utils/State/StateDepthStencil.h"

namespace D3D
{
	namespace DepthStencilState
	{
		const D3D12_DEPTH_STENCIL_DESC None =
		{
			FALSE, // DepthEnable
			D3D12_DEPTH_WRITE_MASK_ZERO,
			D3D12_COMPARISON_FUNC_GREATER_EQUAL, // DepthFunc
			FALSE, // StencilEnable
			D3D12_DEFAULT_STENCIL_READ_MASK,
			D3D12_DEFAULT_STENCIL_WRITE_MASK,
			{
				D3D12_STENCIL_OP_KEEP, // StencilFailOp
				D3D12_STENCIL_OP_KEEP, // StencilDepthFailOp
				D3D12_STENCIL_OP_KEEP, // StencilPassOp
				D3D12_COMPARISON_FUNC_ALWAYS // StencilFunc
			}, // FrontFace,
			{
				D3D12_STENCIL_OP_KEEP, // StencilFailOp
				D3D12_STENCIL_OP_KEEP, // StencilDepthFailOp
				D3D12_STENCIL_OP_KEEP, // StencilPassOp
				D3D12_COMPARISON_FUNC_ALWAYS // StencilFunc
			} // BackFace
		};

		const D3D12_DEPTH_STENCIL_DESC Default =
		{
			TRUE, // DepthEnable
			D3D12_DEPTH_WRITE_MASK_ALL,
			D3D12_COMPARISON_FUNC_GREATER_EQUAL, // DepthFunc
			FALSE, // StencilEnable
			D3D12_DEFAULT_STENCIL_READ_MASK,
			D3D12_DEFAULT_STENCIL_WRITE_MASK,
			{
				D3D12_STENCIL_OP_KEEP, // StencilFailOp
				D3D12_STENCIL_OP_KEEP, // StencilDepthFailOp
				D3D12_STENCIL_OP_KEEP, // StencilPassOp
				D3D12_COMPARISON_FUNC_ALWAYS // StencilFunc
			}, // FrontFace,
			{
				D3D12_STENCIL_OP_KEEP, // StencilFailOp
				D3D12_STENCIL_OP_KEEP, // StencilDepthFailOp
				D3D12_STENCIL_OP_KEEP, // StencilPassOp
				D3D12_COMPARISON_FUNC_ALWAYS // StencilFunc
			} // BackFace
		};

		const D3D12_DEPTH_STENCIL_DESC Read =
		{
			TRUE, // DepthEnable
			D3D12_DEPTH_WRITE_MASK_ZERO,
			D3D12_COMPARISON_FUNC_GREATER_EQUAL, // DepthFunc
			FALSE, // StencilEnable
			D3D12_DEFAULT_STENCIL_READ_MASK,
			D3D12_DEFAULT_STENCIL_WRITE_MASK,
			{
				D3D12_STENCIL_OP_KEEP, // StencilFailOp
				D3D12_STENCIL_OP_KEEP, // StencilDepthFailOp
				D3D12_STENCIL_OP_KEEP, // StencilPassOp
				D3D12_COMPARISON_FUNC_ALWAYS // StencilFunc
			}, // FrontFace,
			{
				D3D12_STENCIL_OP_KEEP, // StencilFailOp
				D3D12_STENCIL_OP_KEEP, // StencilDepthFailOp
				D3D12_STENCIL_OP_KEEP, // StencilPassOp
				D3D12_COMPARISON_FUNC_ALWAYS // StencilFunc
			} // BackFace
		};
	}
}
