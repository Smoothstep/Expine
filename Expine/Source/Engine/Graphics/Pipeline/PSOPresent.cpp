#include "Precompiled.h"

#include "Pipeline/Pipelines.h"
#include "Scene/Scene.h"
#include "Utils/State/StateRasterizer.h"
#include "Utils/State/StateBlend.h"
#include "Utils/State/StateSampler.h"

namespace D3D
{
	namespace Pipelines
	{
		ErrorCode PipelinePresent::CreateRootSignature()
		{
			ErrorCode Error;

			RootSignature = new RRootSignature();
			{
				CD3DX12_ROOT_PARAMETER RootParameters[1];
				{
					RootParameters[0].InitAsDescriptorTable
					(
						1,
						&CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, SRV_COMPOSITE),
						D3D12_SHADER_VISIBILITY_PIXEL
					);
				}

				D3D12_STATIC_SAMPLER_DESC SamplerDescs[1];
				{
					SamplerDescs[0] = SamplerStates::BilinearSampler
					(
						0,
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP
					);
				}

				if ((Error = RootSignature->Create(RRootSignature::InitializeOptions(
					_countof(RootParameters),
					RootParameters,
					_countof(SamplerDescs),
					SamplerDescs))))
				{
					return Error;
				}
			}

			return S_OK;
		}

		ErrorCode PipelinePresent::CreatePipelineState()
		{
			ErrorCode Error;

			SharedPointer<CGrpShader> DefaultShader = new CGrpShader(this);
			{
				if ((Error = DefaultShader->CompileShaderType(CGrpShader::Pixel, L"Present.hlsl", "PSMain", "ps_5_0")))
				{
					return Error;
				}

				if ((Error = DefaultShader->CompileShaderType(CGrpShader::Vertex, L"Quad.hlsl", "VSMain2", "vs_5_0")))
				{
					return Error;
				}
			}

			PipelineState = new RPipelineState(RootSignature);
			{
				D3D12_RASTERIZER_DESC RasterizerDesc = RasterizerState::Default;
				D3D12_BLEND_DESC BlendDesc = BlendState::NoBlend;
				D3D12_INPUT_LAYOUT_DESC InputLayout = {};
				{
					InputLayout.NumElements = 0;
					InputLayout.pInputElementDescs = NULL;
				}

				RPipelineState::InitializeOptions * Options = new RPipelineState::InitializeOptionsGraphics
				(
					DefaultShader,
					InputLayout,
					BlendDesc,
					RasterizerDesc,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
					CSceneComponents::CompositeSceneFormat
				);

				if ((Error = PipelineState->Create(Options)))
				{
					return Error;
				}
			}

			return S_OK;
		}
	}
}
