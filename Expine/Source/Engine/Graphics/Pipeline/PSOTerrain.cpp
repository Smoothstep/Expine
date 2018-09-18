#include "Precompiled.h"

#include "Pipeline/Pipelines.h"
#include "Pipeline/PSOTerrain.h"
#include "Utils/State/StateRasterizer.h"
#include "Utils/State/StateBlend.h"
#include "Utils/State/StateDepthStencil.h"
#include "Utils/State/StateSampler.h"

#include "Scene/Scene.h"

namespace D3D
{
	namespace Pipelines
	{
		namespace Terrain
		{
			ErrorCode PipelineGeometry::CreateRootSignature()
			{
				ErrorCode Error;

				RootSignature = new RRootSignature();
				{
					CD3DX12_ROOT_PARAMETER RootParameters[3];
					{
						CD3DX12_DESCRIPTOR_RANGE DescriptorRangesDS[1];
						{
							DescriptorRangesDS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
						}

						RootParameters[0].InitAsDescriptorTable(_countof(DescriptorRangesDS), DescriptorRangesDS, D3D12_SHADER_VISIBILITY_ALL);

						CD3DX12_DESCRIPTOR_RANGE DescriptorRangesPS[7];
						{
							DescriptorRangesPS[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, SRV_Height);
							DescriptorRangesPS[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 0, SRV_TextureAtlas);
							DescriptorRangesPS[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 0, SRV_TextureAtlasNormals);
							DescriptorRangesPS[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5, 0, SRV_TextureAtlasParallax);
							DescriptorRangesPS[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6, 0, SRV_Noise);
							DescriptorRangesPS[5].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 7, 0, SRV_ColorMap);
							DescriptorRangesPS[6].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 8, 0, SRV_NormalMap);
						}

						RootParameters[1].InitAsDescriptorTable(_countof(DescriptorRangesPS), DescriptorRangesPS, D3D12_SHADER_VISIBILITY_ALL);
						RootParameters[2].InitAsConstants(1, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL);
					}

					D3D12_STATIC_SAMPLER_DESC SamplerDescs[1];
					{
						SamplerDescs[0] = SamplerStates::LinearSampler
						(
							0,
							D3D12_TEXTURE_ADDRESS_MODE_WRAP,
							D3D12_TEXTURE_ADDRESS_MODE_WRAP,
							D3D12_TEXTURE_ADDRESS_MODE_WRAP
						);

						SamplerDescs[0].MipLODBias = 0.0f;
					}

					if ((Error = RootSignature->Create(RRootSignature::InitializeOptions(_countof(RootParameters), RootParameters, _countof(SamplerDescs), SamplerDescs))))
					{
						return Error;
					}
				}

				D3D12_INDIRECT_ARGUMENT_DESC ArgumentDescs[1] = {};
				{
					ArgumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
				}

				D3D12_COMMAND_SIGNATURE_DESC CommandSignatureDesc = {};
				{
					CommandSignatureDesc.ByteStride = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);
					CommandSignatureDesc.NumArgumentDescs = _countof(ArgumentDescs);
					CommandSignatureDesc.pArgumentDescs = ArgumentDescs;
				}

				CommandSignature = new RCommandSignature();
				{
					if ((Error = CommandSignature->Create(CommandSignatureDesc)))
					{
						return Error;
					}
				}

				return S_OK;
			}

			ErrorCode PipelineGeometry::CreatePipelineState()
			{
				ErrorCode Error;

				static SharedPointer<CGrpShader> DefaultShader;

				if (DefaultShader == NULL)
				{
					DefaultShader = new CGrpShader(this);
				}

				if ((Error = DefaultShader->CompileShaderType(CGrpShader::Pixel, L"TerrainPixelShader.hlsl", "PSMain", "ps_5_0")))
				{
					return Error;
				}

				if ((Error = DefaultShader->CompileShaderType(CGrpShader::Vertex, L"TerrainVertexShader.hlsl", "VSMain", "vs_5_0")))
				{
					return Error;
				}

				if ((Error = DefaultShader->CompileShaderType(CGrpShader::Hull, L"TerrainHullShader.hlsl", "HS", "hs_5_0")))
				{
					return Error;
				}

				if ((Error = DefaultShader->CompileShaderType(CGrpShader::Domain, L"TerrainDomainShader.hlsl", "DS", "ds_5_0")))
				{
					return Error;
				}

				PipelineState = new RPipelineState(RootSignature);
				{
					D3D12_INPUT_ELEMENT_DESC InputElementDesc[] =
					{
						"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0,  0,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0,
						"TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0,	12,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0,
						"TEXCOORD",	1, DXGI_FORMAT_R32G32B32_FLOAT,		0,	20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0,
						"TEXCOORD",	2, DXGI_FORMAT_R32G32B32_FLOAT,		0,	32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0,
						"TEXCOORD",	3, DXGI_FORMAT_R32G32B32_FLOAT,		0,	44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0,
						"TEXCOORD", 4, DXGI_FORMAT_R8G8B8A8_UINT,		0,	56, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0,
						"TEXCOORD", 5, DXGI_FORMAT_R8G8B8A8_UINT,		0,	60, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0,
						"TEXCOORD", 6, DXGI_FORMAT_R8G8B8A8_UINT,		0,	64, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
					};

					D3D12_INPUT_LAYOUT_DESC InputLayoutDesc = INPUT_LAYOUT(InputElementDesc);
					D3D12_DEPTH_STENCIL_DESC DepthStencilDesc = DepthStencilState::Default;
					D3D12_RASTERIZER_DESC RasterizerDesc = RasterizerState::Default;
					D3D12_BLEND_DESC BlendDesc = BlendState::NoBlend;

					DXGI_FORMAT RTVFormats[2] =
					{
						CScene::ColorSceneFormat,
						CScene::NormalSceneFormat
					};

					RPipelineState::InitializeOptions * Options = new RPipelineState::InitializeOptionsGraphics
					(
						DefaultShader,
						InputLayoutDesc,
						BlendDesc,
						RasterizerDesc,
						DepthStencilDesc,
						D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH,
						_countof(RTVFormats),
						RTVFormats,
						CSceneComponents::DepthFormat
					);

					if ((Error = PipelineState->Create(Options)))
					{
						return Error;
					}
				}

				return S_OK;
			}

			ErrorCode PipelineShadows::CreateRootSignature()
			{
				ErrorCode Error;

				D3D12_STATIC_SAMPLER_DESC SamplerDescs[1];
				{
					SamplerDescs[0] = SamplerStates::LinearSampler
					(
						0,
						D3D12_TEXTURE_ADDRESS_MODE_WRAP,
						D3D12_TEXTURE_ADDRESS_MODE_WRAP,
						D3D12_TEXTURE_ADDRESS_MODE_WRAP
					);

					SamplerDescs[0].MipLODBias = 0.0f;
				}

				RootSignature = new RRootSignature();
				{
					CD3DX12_ROOT_PARAMETER RootParameters[2];
					{
						CD3DX12_DESCRIPTOR_RANGE DescriptorRangesCBV[1];
						{
							DescriptorRangesCBV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, CBV_LIGHT);
						}

						CD3DX12_DESCRIPTOR_RANGE DescriptorRangesSRV[1];
						{
							DescriptorRangesSRV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, SRV_Height);
						}

						RootParameters[0].InitAsDescriptorTable(_countof(DescriptorRangesCBV), DescriptorRangesCBV, D3D12_SHADER_VISIBILITY_DOMAIN);
						RootParameters[1].InitAsDescriptorTable(_countof(DescriptorRangesSRV), DescriptorRangesSRV, D3D12_SHADER_VISIBILITY_DOMAIN);
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

				D3D12_INDIRECT_ARGUMENT_DESC ArgumentDescs[1] = {};
				{
					ArgumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
				}

				D3D12_COMMAND_SIGNATURE_DESC CommandSignatureDesc = {};
				{
					CommandSignatureDesc.ByteStride = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);
					CommandSignatureDesc.NumArgumentDescs = _countof(ArgumentDescs);
					CommandSignatureDesc.pArgumentDescs = ArgumentDescs;
				}

				CommandSignature = new RCommandSignature();
				{
					if ((Error = CommandSignature->Create(CommandSignatureDesc)))
					{
						return Error;
					}
				}

				return S_OK;
			}

			ErrorCode PipelineShadows::CreatePipelineState()
			{
				ErrorCode Error;

				static SharedPointer<CGrpShader> DefaultShader;

				if (DefaultShader == NULL)
				{
					DefaultShader = new CGrpShader(this);
				}

				if ((Error = DefaultShader->CompileShaderType(CGrpShader::Vertex, L"TerrainVertexShader.hlsl", "VSShadows", "vs_5_0")))
				{
					return Error;
				}

				if ((Error = DefaultShader->CompileShaderType(CGrpShader::Hull, L"TerrainHullShaderShadows.hlsl", "HSShadows", "hs_5_0")))
				{
					return Error;
				}

				if ((Error = DefaultShader->CompileShaderType(CGrpShader::Domain, L"TerrainDomainShaderShadows.hlsl", "DSShadows", "ds_5_0")))
				{
					return Error;
				}

				PipelineState = new RPipelineState(RootSignature);
				{
					D3D12_INPUT_ELEMENT_DESC InputElementDesc[5] =
					{
						"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0,  0,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0,
						"TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0,	12,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0,
						"TEXCOORD",	1, DXGI_FORMAT_R32G32B32_FLOAT,		0,	20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0,
						"TEXCOORD",	2, DXGI_FORMAT_R32G32B32_FLOAT,		0,	32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0,
						"TEXCOORD",	3, DXGI_FORMAT_R32G32B32_FLOAT,		0,	44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
					};

					D3D12_INPUT_LAYOUT_DESC InputLayoutDesc = INPUT_LAYOUT(InputElementDesc);
					D3D12_DEPTH_STENCIL_DESC DepthStencilDesc = DepthStencilState::Default;
					D3D12_RASTERIZER_DESC RasterizerDesc = RasterizerState::Default;
					D3D12_BLEND_DESC BlendDesc = BlendState::NoBlend;
					{
						BlendDesc.RenderTarget[0].RenderTargetWriteMask = 0x00000000;
					}

					RPipelineState::InitializeOptions * Options = new RPipelineState::InitializeOptionsGraphics
					(
						DefaultShader,
						InputLayoutDesc,
						BlendDesc,
						RasterizerDesc,
						DepthStencilDesc,
						D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH,
						DXGI_FORMAT_UNKNOWN,
						CSceneComponents::ShadowMapFormat
					);

					if ((Error = PipelineState->Create(Options)))
					{
						return Error;
					}
				}

				return S_OK;
			}

			ErrorCode PipelineComputeOcclusion::CreateRootSignature()
			{
				ErrorCode Error;

				D3D12_STATIC_SAMPLER_DESC SamplerDescs[1];
				{
					SamplerDescs[0] = SamplerStates::LinearSampler
					(
						0,
						D3D12_TEXTURE_ADDRESS_MODE_WRAP,
						D3D12_TEXTURE_ADDRESS_MODE_WRAP,
						D3D12_TEXTURE_ADDRESS_MODE_WRAP
					);
				}

				RootSignature = new RRootSignature();
				{
					CD3DX12_ROOT_PARAMETER RootParameters[NumRootParameters];
					{
						CD3DX12_DESCRIPTOR_RANGE DescriptorRangesSRV[2];
						{
							DescriptorRangesSRV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, SRV_Height);
							DescriptorRangesSRV[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, SRV_Normal);
						}

						CD3DX12_DESCRIPTOR_RANGE DescriptorRangesUAV[1];
						{
							DescriptorRangesUAV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2, 0, UAV_OCCLUSION);
						}

						RootParameters[LightCBV].InitAsConstantBufferView(0);
						RootParameters[SizeConstant].InitAsConstants(2, 1);
						RootParameters[TerrainPatch].InitAsConstants(4, 2);
						RootParameters[TerrainSRV].InitAsDescriptorTable(_countof(DescriptorRangesSRV), DescriptorRangesSRV);
						RootParameters[OcclusionMapUAV].InitAsDescriptorTable(_countof(DescriptorRangesUAV), DescriptorRangesUAV);
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

			ErrorCode PipelineComputeOcclusion::CreatePipelineState()
			{
				ErrorCode Error;

				static SharedPointer<CGrpShader> DefaultShader;

				if (DefaultShader == NULL)
				{
					DefaultShader = new CGrpShader(this);
				}

				if ((Error = DefaultShader->CompileShaderType(CGrpShader::Compute, L"TerrainOcclusionCompute.hlsl", "ComputeOcclusion", "cs_5_0")))
				{
					return Error;
				}

				PipelineState = new RPipelineState(RootSignature);
				{
					RPipelineState::InitializeOptionsCompute * Options = new RPipelineState::InitializeOptionsCompute(DefaultShader);

					if ((Error = PipelineState->Create(Options)))
					{
						return Error;
					}
				}

				D3D12_INDIRECT_ARGUMENT_DESC ArgumentDescs[2] = {};
				{
					ArgumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
					ArgumentDescs[0].Constant.DestOffsetIn32BitValues = 0;
					ArgumentDescs[0].Constant.Num32BitValuesToSet = 4;
					ArgumentDescs[0].Constant.RootParameterIndex = TerrainPatch;
					ArgumentDescs[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
				}
				
				D3D12_COMMAND_SIGNATURE_DESC CommandSignatureDesc = {};
				{
					CommandSignatureDesc.ByteStride = sizeof(IndirectCommand);
					CommandSignatureDesc.NumArgumentDescs = _countof(ArgumentDescs);
					CommandSignatureDesc.pArgumentDescs = ArgumentDescs;
					CommandSignatureDesc.NodeMask = 1;
				}

				CommandSignature = new RCommandSignature();
				{
					if ((Error = CommandSignature->Create(RootSignature, CommandSignatureDesc)))
					{
						return Error;
					}
				}

				return S_OK;
			}
		}
	}
}
