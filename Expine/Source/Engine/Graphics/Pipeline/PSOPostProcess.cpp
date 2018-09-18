#include "Precompiled.h"

#include "Pipeline/PSOPostProcess.h"

#include "Utils/State/StateSampler.h"
#include "Utils/State/StateDepthStencil.h"
#include "Utils/State/StateBlend.h"
#include "Utils/State/StateRasterizer.h"

namespace D3D
{
	namespace Pipelines
	{
		namespace PostProcess
		{
			namespace Bloom
			{
				ErrorCode PipelineSetup::CreateRootSignature()
				{
					ErrorCode Error;

					RootSignature = new RRootSignature();
					{
						CD3DX12_DESCRIPTOR_RANGE DescriptorRangeSRV
						(
							D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 
							3,
							0
						);

						CD3DX12_DESCRIPTOR_RANGE DescriptorRangeCBV
						(
							D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
							1,
							0
						);

						CD3DX12_ROOT_PARAMETER RootParameters[NumBloomSetupParameter];
						{
							RootParameters[BloomPostProcess].InitAsDescriptorTable(1, &DescriptorRangeCBV, D3D12_SHADER_VISIBILITY_PIXEL);
							RootParameters[BloomThreshold].InitAsConstants(4, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL);
							RootParameters[BloomInput].InitAsDescriptorTable(1, &DescriptorRangeSRV, D3D12_SHADER_VISIBILITY_PIXEL);
						}

						D3D12_STATIC_SAMPLER_DESC Samplers[1];
						{
							Samplers[0] = SamplerStates::BilinearSampler
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
							_countof(Samplers),
							Samplers))))
						{
							return Error;
						}
					}

					return S_OK;
				}

				ErrorCode PipelineSetup::CreatePipelineState()
				{
					ErrorCode Error;

					CGrpShader * DefaultShader = new CGrpShader(this);
					{
						if ((Error = DefaultShader->CompileShaderType(CGrpShader::Pixel, L"PostProcess\\Bloom.hlsl", "MainPS", "ps_5_0")))
						{
							return Error;
						}

						if ((Error = DefaultShader->CompileShaderType(CGrpShader::Vertex, L"PostProcess\\Bloom.hlsl", "MainVS", "vs_5_0")))
						{
							return Error;
						}
					}

					PipelineState = new RPipelineState(RootSignature);
					{
						D3D12_INPUT_LAYOUT_DESC InputLayoutDesc = { 0, NULL };
						D3D12_RASTERIZER_DESC RasterizerDesc = RasterizerState::CullNone;
						D3D12_BLEND_DESC BlendDesc = BlendState::NoBlend;

						RPipelineState::InitializeOptions * Options = new RPipelineState::InitializeOptionsGraphics
						(
							DefaultShader,
							InputLayoutDesc,
							BlendDesc,
							RasterizerDesc,
							D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
							DXGI_FORMAT_R8G8B8A8_UNORM
						);

						if ((Error = PipelineState->Create(Options)))
						{
							return Error;
						}
					}

					return S_OK;
				}

				ErrorCode PipelineOverlay::CreateRootSignature()
				{
					ErrorCode Error;

					RootSignature = new RRootSignature();
					{
						CD3DX12_DESCRIPTOR_RANGE DescriptorRangeSRV
						(
							D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
							3,
							0
						);

						CD3DX12_DESCRIPTOR_RANGE DescriptorRangeCBV
						(
							D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
							1,
							0
						);

						CD3DX12_ROOT_PARAMETER RootParameters[NumBloomSetupParameter];
						{
							RootParameters[BloomPostProcess].InitAsDescriptorTable(1, &DescriptorRangeCBV, D3D12_SHADER_VISIBILITY_PIXEL);
							RootParameters[BloomColorScale].InitAsConstants(4, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL);
							RootParameters[BloomInput].InitAsDescriptorTable(1, &DescriptorRangeSRV, D3D12_SHADER_VISIBILITY_PIXEL);
						}

						D3D12_STATIC_SAMPLER_DESC Samplers[3];
						{
							Samplers[0] = SamplerStates::BilinearSampler
							(
								0,
								D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
								D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
								D3D12_TEXTURE_ADDRESS_MODE_CLAMP
							);

							Samplers[1] = SamplerStates::BilinearSampler
							(
								1,
								D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
								D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
								D3D12_TEXTURE_ADDRESS_MODE_CLAMP
							);

							Samplers[2] = SamplerStates::BilinearSampler
							(
								2,
								D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
								D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
								D3D12_TEXTURE_ADDRESS_MODE_CLAMP
							);
						}

						if ((Error = RootSignature->Create(RRootSignature::InitializeOptions(
							_countof(RootParameters),
							RootParameters,
							_countof(Samplers),
							Samplers))))
						{
							return Error;
						}
					}

					return S_OK;
				}

				ErrorCode PipelineOverlay::CreatePipelineState()
				{
					ErrorCode Error;

					CGrpShader * DefaultShader = new CGrpShader(this);
					{
						if ((Error = DefaultShader->CompileShaderType(CGrpShader::Pixel, L"PostProcess\\Bloom.hlsl", "VisualizeBloomOverlayPS", "ps_5_0")))
						{
							return Error;
						}

						if ((Error = DefaultShader->CompileShaderType(CGrpShader::Vertex, L"PostProcess\\Bloom.hlsl", "MainVS", "vs_5_0")))
						{
							return Error;
						}
					}

					PipelineState = new RPipelineState(RootSignature);
					{
						D3D12_INPUT_LAYOUT_DESC InputLayoutDesc = { 0, NULL };
						D3D12_RASTERIZER_DESC RasterizerDesc = RasterizerState::CullNone;
						D3D12_BLEND_DESC BlendDesc = BlendState::NoBlend;

						RPipelineState::InitializeOptions * Options = new RPipelineState::InitializeOptionsGraphics
						(
							DefaultShader,
							InputLayoutDesc,
							BlendDesc,
							RasterizerDesc,
							D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
							DXGI_FORMAT_R8G8B8A8_UNORM
						);

						if ((Error = PipelineState->Create(Options)))
						{
							return Error;
						}
					}

					return S_OK;
				}
			}

			ErrorCode InitializePipelines()
			{
				ErrorCode Error;

				if (Bloom::PipelineSetup::New())
				{
					if ((Error = Bloom::PipelineSetup::Instance().Initialize()))
					{
						return Error;
					}
				}

				if (Bloom::PipelineOverlay::New())
				{
					if ((Error = Bloom::PipelineOverlay::Instance().Initialize()))
					{
						return Error;
					}
				}

				return S_OK;
			}
		}
	}
}
