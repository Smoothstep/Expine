#include "Precompiled.h"

#include "Utils/State/StateBlend.h"

namespace D3D
{
	namespace BlendState
	{
		const D3D12_RENDER_TARGET_BLEND_DESC NoRenderTargetBlend =
		{
			FALSE,
			FALSE,
			D3D12_BLEND_ONE,
			D3D12_BLEND_ONE,
			D3D12_BLEND_OP_ADD,
			D3D12_BLEND_ONE,
			D3D12_BLEND_ONE,
			D3D12_BLEND_OP_ADD,
			D3D12_LOGIC_OP_NOOP,
			D3D12_COLOR_WRITE_ENABLE_ALL
		};

		const D3D12_RENDER_TARGET_BLEND_DESC AlphaRenderTargetBlend =
		{
			TRUE,
			FALSE,
			D3D12_BLEND_SRC_ALPHA,
			D3D12_BLEND_INV_SRC_ALPHA,
			D3D12_BLEND_OP_ADD,
			D3D12_BLEND_SRC_ALPHA,
			D3D12_BLEND_INV_SRC_ALPHA,
			D3D12_BLEND_OP_ADD,
			D3D12_LOGIC_OP_NOOP,
			D3D12_COLOR_WRITE_ENABLE_ALL
		};

		const D3D12_RENDER_TARGET_BLEND_DESC AdditiveModulateRenderTargetBlend =
		{
			TRUE,
			FALSE,
			D3D12_BLEND_BLEND_FACTOR,
			D3D12_BLEND_ONE,
			D3D12_BLEND_OP_ADD,
			D3D12_BLEND_BLEND_FACTOR,
			D3D12_BLEND_ONE,
			D3D12_BLEND_OP_ADD,
			D3D12_LOGIC_OP_NOOP,
			D3D12_COLOR_WRITE_ENABLE_ALL
		};

		const D3D12_RENDER_TARGET_BLEND_DESC AdditiveRenderTargetBlend =
		{
			TRUE,
			FALSE,
			D3D12_BLEND_BLEND_FACTOR,
			D3D12_BLEND_ONE,
			D3D12_BLEND_OP_ADD,
			D3D12_BLEND_BLEND_FACTOR,
			D3D12_BLEND_ONE,
			D3D12_BLEND_OP_ADD,
			D3D12_LOGIC_OP_NOOP,
			D3D12_COLOR_WRITE_ENABLE_ALL
		};

		const D3D12_BLEND_DESC NoBlend =
		{
			FALSE,
			FALSE,
			{
				NoRenderTargetBlend,
				NoRenderTargetBlend,
				NoRenderTargetBlend,
				NoRenderTargetBlend,
				NoRenderTargetBlend,
				NoRenderTargetBlend,
				NoRenderTargetBlend,
				NoRenderTargetBlend
			}
		};

		const D3D12_BLEND_DESC AlphaBlendDesc =
		{
			FALSE,
			FALSE,
			{
				AlphaRenderTargetBlend,
				AlphaRenderTargetBlend,
				AlphaRenderTargetBlend,
				AlphaRenderTargetBlend,
				AlphaRenderTargetBlend,
				AlphaRenderTargetBlend,
				AlphaRenderTargetBlend,
				AlphaRenderTargetBlend
			}
		};

		const D3D12_BLEND_DESC AdditiveModulate =
		{
			FALSE,
			FALSE,
			{
				AdditiveModulateRenderTargetBlend,
				AdditiveModulateRenderTargetBlend,
				AdditiveModulateRenderTargetBlend,
				AdditiveModulateRenderTargetBlend,
				AdditiveModulateRenderTargetBlend,
				AdditiveModulateRenderTargetBlend,
				AdditiveModulateRenderTargetBlend,
				AdditiveModulateRenderTargetBlend
			}
		};

		const D3D12_BLEND_DESC Additive =
		{
			FALSE,
			FALSE,
			{
				AdditiveRenderTargetBlend,
				AdditiveRenderTargetBlend,
				AdditiveRenderTargetBlend,
				AdditiveRenderTargetBlend,
				AdditiveRenderTargetBlend,
				AdditiveRenderTargetBlend,
				AdditiveRenderTargetBlend,
				AdditiveRenderTargetBlend
			}
		};
	}
}
