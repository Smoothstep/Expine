#pragma once

#include "D3D.h"

namespace D3D
{
	namespace SamplerStates
	{
		static D3D12_STATIC_SAMPLER_DESC AnistropicSampler
		(
			const UINT							ShaderRegister,
			const D3D12_TEXTURE_ADDRESS_MODE	AddressU,
			const D3D12_TEXTURE_ADDRESS_MODE	AddressV,
			const D3D12_TEXTURE_ADDRESS_MODE	AddressW,
			const UINT							MaxAnistropy = 16
		)
		{
			return CD3DX12_STATIC_SAMPLER_DESC
			(
				ShaderRegister,
				D3D12_FILTER_ANISOTROPIC,
				AddressU,
				AddressV,
				AddressW,
				0.0f,
				MaxAnistropy,
				D3D12_COMPARISON_FUNC_NEVER
			);
		}

		static D3D12_STATIC_SAMPLER_DESC PointSampler
		(
			const UINT							ShaderRegister,
			const D3D12_TEXTURE_ADDRESS_MODE	AddressU,
			const D3D12_TEXTURE_ADDRESS_MODE	AddressV,
			const D3D12_TEXTURE_ADDRESS_MODE	AddressW
		)
		{
			return CD3DX12_STATIC_SAMPLER_DESC
			(
				ShaderRegister,
				D3D12_FILTER_MIN_MAG_MIP_POINT,
				AddressU,
				AddressV,
				AddressW,
				0.0f,
				1,
				D3D12_COMPARISON_FUNC_NEVER
			);
		}

		static D3D12_STATIC_SAMPLER_DESC LinearSampler
		(
			const UINT							ShaderRegister,
			const D3D12_TEXTURE_ADDRESS_MODE	AddressU,
			const D3D12_TEXTURE_ADDRESS_MODE	AddressV,
			const D3D12_TEXTURE_ADDRESS_MODE	AddressW
		)
		{
			return CD3DX12_STATIC_SAMPLER_DESC
			(
				ShaderRegister,
				D3D12_FILTER_MIN_MAG_MIP_LINEAR,
				AddressU,
				AddressV,
				AddressW,
				0.0f,
				0,
				D3D12_COMPARISON_FUNC_NEVER,
				D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK,
				FLT_MIN,
				FLT_MAX
			);
		}

		static D3D12_STATIC_SAMPLER_DESC BilinearSampler
		(
			const UINT							ShaderRegister,
			const D3D12_TEXTURE_ADDRESS_MODE	AddressU,
			const D3D12_TEXTURE_ADDRESS_MODE	AddressV,
			const D3D12_TEXTURE_ADDRESS_MODE	AddressW
		)
		{
			return CD3DX12_STATIC_SAMPLER_DESC
			(
				ShaderRegister,
				D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT,
				AddressU,
				AddressV,
				AddressW,
				0.0f,
				1,
				D3D12_COMPARISON_FUNC_ALWAYS
			);
		}
	}
}