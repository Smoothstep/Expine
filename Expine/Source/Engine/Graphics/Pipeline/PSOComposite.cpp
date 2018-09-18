#include "Precompiled.h"
#include "Pipeline/PSOComposite.h"
#include "Scene/Scene.h"
#include "Utils/State/StateRasterizer.h"
#include "Utils/State/StateBlend.h"
#include "Utils/State/StateSampler.h"

namespace D3D
{
	namespace Pipelines
	{
		ErrorCode PipelineComposite::CreateRootSignature()
		{
			ErrorCode Error;

			RootSignature = new RRootSignature();
			{
				CD3DX12_ROOT_PARAMETER RootParameters[2];
				{
					D3D12_DESCRIPTOR_RANGE DescriptorRangesScene[] =
					{
						CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, NumCBVDescriptors, 0),
						CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 6, 0, 0, NumCBVDescriptors),
						CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1, 0, NumCBVDescriptors + NumSRVDescriptors)
					};

					D3D12_DESCRIPTOR_RANGE DescriptorRangesTexture[] =
					{
						CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6, 0, 0),
						CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 7, 0, 1),
						CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 8, 0, 2)
					};

					RootParameters[SceneConstants].InitAsDescriptorTable
					(
						_countof(	DescriptorRangesScene),
									DescriptorRangesScene,
						D3D12_SHADER_VISIBILITY_ALL
					);

					RootParameters[SceneResources].InitAsDescriptorTable
					(
						_countof(	DescriptorRangesTexture),
									DescriptorRangesTexture,
						D3D12_SHADER_VISIBILITY_ALL
					);
				}

				D3D12_STATIC_SAMPLER_DESC SamplerDescs[4];
				{
					SamplerDescs[0] = SamplerStates::BilinearSampler
					(
						0,
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP
					);

					SamplerDescs[1] = CD3DX12_STATIC_SAMPLER_DESC
					(
						1, 
						D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, 
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
						0.0,
						0,
						D3D12_COMPARISON_FUNC_LESS,
						D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
						FLT_MIN,
						FLT_MAX,
						D3D12_SHADER_VISIBILITY_PIXEL
					);
					
					SamplerDescs[2] = SamplerStates::LinearSampler
					(
						2,
						D3D12_TEXTURE_ADDRESS_MODE_WRAP,
						D3D12_TEXTURE_ADDRESS_MODE_WRAP,
						D3D12_TEXTURE_ADDRESS_MODE_WRAP
					);

					SamplerDescs[3] = SamplerStates::LinearSampler
					(
						3,
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

		ErrorCode PipelineComposite::CreatePipelineState()
		{
			ErrorCode Error;

			static SharedPointer<CGrpShader> DefaultShader;

			if (DefaultShader == NULL)
			{
				DefaultShader = new CGrpShader(this);
			}

			if ((Error = DefaultShader->CompileShaderType(CGrpShader::Pixel, L"Composite.hlsl", "PSMain", "ps_5_0")))
			{
				return Error;
			}

			if ((Error = DefaultShader->CompileShaderType(CGrpShader::Vertex, L"CompositeVertexShader.hlsl", "VSMain", "vs_5_0")))
			{
				return Error;
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
