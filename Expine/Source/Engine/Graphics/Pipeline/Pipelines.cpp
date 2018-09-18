#include "Precompiled.h"
#include "Pipeline/Pipelines.h"
#include "Pipeline/PSOAtmosphere.h"
#include "Pipeline/PSOPostProcess.h"
#include "Pipeline/PSOTerrain.h"
#include "Pipeline/PSOTriangle.h"
#include "Pipeline/PSOLine.h"
#include "Pipeline/PSOComposite.h"
#include "Utils/StringOp.h"
#include "Utils/State/StateRasterizer.h"
#include "Utils/State/StateBlend.h"
#include "Utils/State/StateDepthStencil.h"
#include "Utils/State/StateSampler.h"
#include "Scene/Scene.h"

namespace D3D
{
	namespace Pipelines
	{
		ErrorCode PipelineColorTriangle::Initialize()
		{
			ErrorCode Error;

			if ((Error = CreateRootSignature()))
			{
				return Error;
			}

			if ((Error = CreatePipelineState()))
			{
				return Error;
			}

			return S_OK;
		}

		ErrorCode PipelineColorTriangle::CreateRootSignature()
		{
			ErrorCode Error;

			RootSignature = new RRootSignature();
			{
				CD3DX12_ROOT_PARAMETER RootParameters[1];
				{
					CD3DX12_DESCRIPTOR_RANGE DescriptorRangesCBV[1];
					{
						DescriptorRangesCBV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
					}

					RootParameters[0].InitAsDescriptorTable(_countof(DescriptorRangesCBV), DescriptorRangesCBV, D3D12_SHADER_VISIBILITY_ALL);
				}

				if ((Error = RootSignature->Create(RRootSignature::InitializeOptions(_countof(RootParameters), RootParameters, 0, NULL))))
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

		ErrorCode PipelineColorTriangle::CreatePipelineState()
		{
			ErrorCode Error;

			CGrpShader * DefaultShader = new CGrpShader(this);
			{
				if ((Error = DefaultShader->CompileShaderType(CGrpShader::Pixel, L"LinePixelShader.hlsl", "PSMain", "ps_5_0")))
				{
					return Error;
				}

				if ((Error = DefaultShader->CompileShaderType(CGrpShader::Vertex, L"LineVertexShader.hlsl", "VSMain", "vs_5_0")))
				{
					return Error;
				}
			}

			D3D12_INPUT_ELEMENT_DESC InputElementDesc[2] =
			{
				"POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0,  0,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0,
				"COLOR",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	16,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
			};

			D3D12_INPUT_LAYOUT_DESC InputLayoutDesc = INPUT_LAYOUT(InputElementDesc);
			D3D12_DEPTH_STENCIL_DESC DepthStencilDesc = DepthStencilState::Default;
			D3D12_RASTERIZER_DESC RasterizerDesc = RasterizerState::Default;
			D3D12_BLEND_DESC BlendDesc = BlendState::NoBlend;

			RPipelineState::InitializeOptions * Options = new RPipelineState::InitializeOptionsGraphics
			(
				DefaultShader,
				InputLayoutDesc,
				BlendDesc,
				RasterizerDesc,
				DepthStencilDesc,
				D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
				DXGI_FORMAT_R8G8B8A8_UNORM,
				CSceneComponents::DepthFormat
			);

			PipelineState = new RPipelineState(RootSignature);

			if ((Error = PipelineState->Create(Options)))
			{
				return Error;
			}

			return S_OK;
		}

		ErrorCode PipelineWireframeTriangle::CreateRootSignature()
		{
			ErrorCode Error;

			RootSignature = new RRootSignature();
			{
				CD3DX12_ROOT_PARAMETER RootParameters[1];
				{
					RootParameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
				}

				if ((Error = RootSignature->Create(RRootSignature::InitializeOptions(_countof(RootParameters), RootParameters, 0, NULL))))
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

		ErrorCode PipelineWireframeTriangle::CreatePipelineState()
		{
			ErrorCode Error;

			CGrpShader * DefaultShader = new CGrpShader(this);
			{
				if ((Error = DefaultShader->CompileShaderType(CGrpShader::Pixel, L"LinePixelShader.hlsl", "PSMain", "ps_5_0")))
				{
					return Error;
				}

				if ((Error = DefaultShader->CompileShaderType(CGrpShader::Vertex, L"LineVertexShader.hlsl", "VSMain", "vs_5_0")))
				{
					return Error;
				}
			}

			D3D12_INPUT_ELEMENT_DESC InputElementDesc[2] =
			{
				"POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0,  0,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0,
				"COLOR",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	12,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
			};

			D3D12_INPUT_LAYOUT_DESC InputLayoutDesc = INPUT_LAYOUT(InputElementDesc);
			D3D12_RASTERIZER_DESC RasterizerDesc = RasterizerState::Default;
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

			PipelineState = new RPipelineState(RootSignature);

			if ((Error = PipelineState->Create(Options)))
			{
				return Error;
			}

			return S_OK;
		}

		ErrorCode PipelineWireframeTriangle::Initialize()
		{
			ErrorCode Error;

			if ((Error = CreateRootSignature()))
			{
				return Error;
			}

			if ((Error = CreatePipelineState()))
			{
				return Error;
			}

			return S_OK;
		}

		ErrorCode PipelineColorLine::CreateRootSignature()
		{
			ErrorCode Error;

			RootSignature = new RRootSignature();
			{
				CD3DX12_ROOT_PARAMETER RootParameters[1];
				{
					CD3DX12_DESCRIPTOR_RANGE DescriptorRangesCBV[1];
					{
						DescriptorRangesCBV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
					}

					RootParameters[0].InitAsDescriptorTable(_countof(DescriptorRangesCBV), DescriptorRangesCBV, D3D12_SHADER_VISIBILITY_ALL);
				}

				if ((Error = RootSignature->Create(RRootSignature::InitializeOptions(_countof(RootParameters), RootParameters, 0, NULL))))
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

		ErrorCode PipelineColorLine::CreatePipelineState()
		{
			ErrorCode Error;

			CGrpShader * DefaultShader = new CGrpShader(this);
			{
				if ((Error = DefaultShader->CompileShaderType(CGrpShader::Pixel, L"LinePixelShader.hlsl", "PSMain", "ps_5_0")))
				{
					return Error;
				}

				if ((Error = DefaultShader->CompileShaderType(CGrpShader::Vertex, L"LineVertexShader.hlsl", "VSMain", "vs_5_0")))
				{
					return Error;
				}
			}

			PipelineState = new RPipelineState(RootSignature);
			{
				D3D12_INPUT_ELEMENT_DESC InputElementDesc[2] =
				{
					"POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0,  0,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0,
					"COLOR",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	16,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
				};

				D3D12_INPUT_LAYOUT_DESC InputLayoutDesc = INPUT_LAYOUT(InputElementDesc);
				D3D12_RASTERIZER_DESC RasterizerDesc = RasterizerState::CullNone;
				D3D12_BLEND_DESC BlendDesc = BlendState::AlphaBlendDesc;

				RPipelineState::InitializeOptions * Options = new RPipelineState::InitializeOptionsGraphics
				(
					DefaultShader,
					InputLayoutDesc,
					BlendDesc,
					RasterizerDesc,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,
					DXGI_FORMAT_R8G8B8A8_UNORM
				);

				if ((Error = PipelineState->Create(Options)))
				{
					return Error;
				}
			}

			return S_OK;
		}

		ErrorCode PipelineColorLine::Initialize()
		{
			ErrorCode Error;

			if ((Error = CreateRootSignature()))
			{
				return Error;
			}

			if ((Error = CreatePipelineState()))
			{
				return Error;
			}

			return S_OK;
		}

		ErrorCode InitializePipelines()
		{
			ErrorCode Error;

			if (PipelineColorLine::New())
			{
				if ((Error = PipelineColorLine::Instance().Initialize()))
				{
					return Error;
				}
			}

			if (PipelineWireframeTriangle::New())
			{
				if ((Error = PipelineWireframeTriangle::Instance().Initialize()))
				{
					return Error;
				}
			}

			if (PipelineColorTriangle::New())
			{
				if ((Error = PipelineColorTriangle::Instance().Initialize()))
				{
					return Error;
				}
			}

			if (PipelineComposite::New())
			{
				if ((Error = PipelineComposite::Instance().Initialize()))
				{
					return Error;
				}
			}

			if (PipelinePresent::New())
			{
				if ((Error = PipelinePresent::Instance().Initialize()))
				{
					return Error;
				}
			}

			if (Terrain::PipelineGeometry::New())
			{
				if ((Error = Terrain::PipelineGeometry::Instance().Initialize()))
				{
					return Error;
				}
			}

			if (Terrain::PipelineShadows::New())
			{
				if ((Error = Terrain::PipelineShadows::Instance().Initialize()))
				{
					return Error;
				}
			}

			if (Terrain::PipelineComputeOcclusion::New())
			{
				if ((Error = Terrain::PipelineComputeOcclusion::Instance().Initialize()))
				{
					return Error;
				}
			}

			if (VolumetricLighting::DownsamplePipeline::New())
			{
				if ((Error = VolumetricLighting::DownsamplePipeline::Instance().Initialize()))
				{
					return Error;
				}
			}

			if (VolumetricLighting::ResolvePSPipeline::New())
			{
				if ((Error = VolumetricLighting::ResolvePSPipeline::Instance().Initialize()))
				{
					return Error;
				}
			}

			if (VolumetricLighting::ResolvePSPipelineMSAA::New())
			{
				if ((Error = VolumetricLighting::ResolvePSPipelineMSAA::Instance().Initialize()))
				{
					return Error;
				}
			}

			if (VolumetricLighting::TemporalFilter::New())
			{
				if ((Error = VolumetricLighting::TemporalFilter::Instance().Initialize()))
				{
					return Error;
				}
			}

			if (VolumetricLighting::ApplyLighting::New())
			{
				if ((Error = VolumetricLighting::ApplyLighting::Instance().Initialize()))
				{
					return Error;
				}
			}

			if (VolumetricLighting::ApplyLightingMSAA::New())
			{
				if ((Error = VolumetricLighting::ApplyLightingMSAA::Instance().Initialize()))
				{
					return Error;
				}
			}

			if (VolumetricLighting::ComputePhaseLookup::New())
			{
				if ((Error = VolumetricLighting::ComputePhaseLookup::Instance().Initialize()))
				{
					return Error;
				}
			}

			if (VolumetricLighting::Directional::PipelineFinal::New())
			{
				if ((Error = VolumetricLighting::Directional::PipelineFinal::Instance().Initialize()))
				{
					return Error;
				}
			}

			if (VolumetricLighting::Directional::PipelineGeometryFrustumGrid::New())
			{
				if ((Error = VolumetricLighting::Directional::PipelineGeometryFrustumGrid::Instance().Initialize()))
				{
					return Error;
				}
			}

			if (VolumetricLighting::Directional::PipelineGeometryFrustumBase::New())
			{
				if ((Error = VolumetricLighting::Directional::PipelineGeometryFrustumBase::Instance().Initialize()))
				{
					return Error;
				}
			}

			if (VolumetricLighting::Directional::PipelineSky::New())
			{
				if ((Error = VolumetricLighting::Directional::PipelineSky::Instance().Initialize()))
				{
					return Error;
				}
			}

			if ((Error = AtmosphericScattering::InitializePipelines()))
			{
				return Error;
			}
			
			if ((Error = PostProcess::InitializePipelines()))
			{
				return Error;
			}
			
			return S_OK;
		}
}

	ErrorCode D3D::Pipelines::VolumetricLighting::DownsamplePipeline::CreateRootSignature()
	{
		ErrorCode Error;

		CD3DX12_ROOT_PARAMETER RootParameters[2];
		{
			CD3DX12_DESCRIPTOR_RANGE DescriptorRangesCBV[1];
			{
				DescriptorRangesCBV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, NumConstantBuffers, 0);
			}

			CD3DX12_DESCRIPTOR_RANGE DescriptorRangesSRV[1];
			{
				DescriptorRangesSRV[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, SRV_DEPTH);
			}

			RootParameters[0].InitAsDescriptorTable(_countof(DescriptorRangesCBV), DescriptorRangesCBV, D3D12_SHADER_VISIBILITY_ALL);
			RootParameters[1].InitAsDescriptorTable(_countof(DescriptorRangesSRV), DescriptorRangesSRV, D3D12_SHADER_VISIBILITY_PIXEL);
		}

		D3D12_STATIC_SAMPLER_DESC Samplers[2];
		{
			Samplers[0] = SamplerStates::PointSampler
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
		}

		RootSignature = new RRootSignature();
		{
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

	ErrorCode D3D::Pipelines::VolumetricLighting::DownsamplePipeline::CreatePipelineState()
	{
		ErrorCode Error;

		SharedPointer<CGrpShader> Shaders = new CGrpShader(this);
		{
			if ((Error = Shaders->CompileShaderType(CGrpShader::Vertex, L"VolumetricLighting\\QuadVS.hlsl", "VSMain2", "vs_5_0")))
			{
				return Error;
			}

			if ((Error = Shaders->CompileShaderType(CGrpShader::Pixel, L"VolumetricLighting\\DownsampleDepthPS.hlsl", "main", "ps_5_0")))
			{
				return Error;
			}
		}

		D3D12_RASTERIZER_DESC Rasterizer = RasterizerState::CullNone;
		D3D12_INPUT_LAYOUT_DESC InputLayout = {};
		{
			InputLayout.NumElements = 0;
			InputLayout.pInputElementDescs = NULL;
		}
		D3D12_DEPTH_STENCIL_DESC DepthStencil = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		{
			DepthStencil.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		}
		CD3DX12_BLEND_DESC Blend(D3D12_DEFAULT);
		{
			Blend.RenderTarget[0].RenderTargetWriteMask = 0x00000000;
		}

		PipelineState = new RPipelineState(RootSignature);
		{
			if ((Error = PipelineState->Create(new RPipelineState::InitializeOptionsGraphics(
				Shaders,
				InputLayout,
				Blend,
				Rasterizer,
				DepthStencil,
				D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
				DXGI_FORMAT_R16G16B16A16_FLOAT,
				DXGI_FORMAT_D32_FLOAT_S8X24_UINT))))
			{
				return Error;
			}
		}

		return S_OK;
	}


	ErrorCode D3D::Pipelines::VolumetricLighting::ResolvePSPipelineMSAA::CreateRootSignature()
	{
		ErrorCode Error;

		CD3DX12_ROOT_PARAMETER RootParameters[1];
		{
			CD3DX12_DESCRIPTOR_RANGE DescriptorRanges[3];
			{
				DescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, NumConstantBuffers, 0);
				DescriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, TARGET_DEPTH);
				DescriptorRanges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, TARGET_ACCUMULATION);
			}

			RootParameters[PARAMETER_CBV_SRV_DESCRIPTOR_TABLE].InitAsDescriptorTable(_countof(DescriptorRanges), DescriptorRanges, D3D12_SHADER_VISIBILITY_ALL);
		}

		D3D12_STATIC_SAMPLER_DESC Samplers[2];
		{
			Samplers[0] = SamplerStates::PointSampler
			(
				0,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP
			);

			Samplers[1] = SamplerStates::LinearSampler
			(
				1,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP
			);
		}

		RootSignature = new RRootSignature();
		{
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

	ErrorCode D3D::Pipelines::VolumetricLighting::ResolvePSPipelineMSAA::CreatePipelineState()
	{
		ErrorCode Error;

		static SharedPointer<CGrpShader> Shaders = new CGrpShader(this);
		{
			if ((Error = Shaders->CompileShaderType(CGrpShader::Vertex, L"VolumetricLighting\\QuadVS.hlsl", "VSMain2", "vs_5_0")))
			{
				return Error;
			}

			if ((Error = Shaders->CompileShaderType(CGrpShader::Pixel, L"VolumetricLighting\\ResolvePS.hlsl", "main", "ps_5_0")))
			{
				return Error;
			}
		}

		D3D12_RASTERIZER_DESC Rasterizer = RasterizerState::CullNone;
		D3D12_INPUT_LAYOUT_DESC InputLayout = {};
		{
			InputLayout.NumElements = 0;
			InputLayout.pInputElementDescs = NULL;
		}
		D3D12_BLEND_DESC Blend = BlendState::NoBlend;

		PipelineState = new RPipelineState(RootSignature);
		{
			DXGI_FORMAT RTVFormats[2] =
			{
				DXGI_FORMAT_R16G16B16A16_FLOAT,
				DXGI_FORMAT_R16G16_FLOAT
			};

			if ((Error = PipelineState->Create(new RPipelineState::InitializeOptionsGraphics(
				Shaders,
				InputLayout,
				Blend,
				Rasterizer,
				D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
				_countof(RTVFormats),
				RTVFormats))))
			{
				return Error;
			}
		}

		return S_OK;
	}

	ErrorCode D3D::Pipelines::VolumetricLighting::ResolvePSPipeline::CreateRootSignature()
	{
		ErrorCode Error;

		CD3DX12_ROOT_PARAMETER RootParameters[1];
		{
			CD3DX12_DESCRIPTOR_RANGE DescriptorRanges[3];
			{
				DescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, NumConstantBuffers, 0);
				DescriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, TARGET_DEPTH);
				DescriptorRanges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, TARGET_ACCUMULATION);
			}

			RootParameters[PARAMETER_CBV_SRV_DESCRIPTOR_TABLE].InitAsDescriptorTable(_countof(DescriptorRanges), DescriptorRanges, D3D12_SHADER_VISIBILITY_ALL);
		}

		D3D12_STATIC_SAMPLER_DESC Samplers[2];
		{
			Samplers[0] = SamplerStates::PointSampler
			(
				0,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP
			);

			Samplers[1] = SamplerStates::LinearSampler
			(
				1,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP
			);
		}

		RootSignature = new RRootSignature();
		{
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

	ErrorCode D3D::Pipelines::VolumetricLighting::ResolvePSPipeline::CreatePipelineState()
	{
		ErrorCode Error;

		static SharedPointer<CGrpShader> Shaders = new CGrpShader(this);
		{
			if ((Error = Shaders->CompileShaderType(CGrpShader::Vertex, L"VolumetricLighting\\QuadVS.hlsl", "VSMain2", "vs_5_0")))
			{
				return Error;
			}

			if ((Error = Shaders->CompileShaderType(CGrpShader::Pixel, L"VolumetricLighting\\ResolvePS.hlsl", "main", "ps_5_0")))
			{
				return Error;
			}
		}

		D3D12_RASTERIZER_DESC Rasterizer = RasterizerState::CullNone;
		D3D12_INPUT_LAYOUT_DESC InputLayout = {};
		{
			InputLayout.NumElements = 0;
			InputLayout.pInputElementDescs = NULL;
		}
		D3D12_BLEND_DESC Blend = BlendState::NoBlend;

		PipelineState = new RPipelineState(RootSignature);
		{
			DXGI_FORMAT RTVFormats[2] =
			{
				DXGI_FORMAT_R16G16B16A16_FLOAT,
				DXGI_FORMAT_R16G16_FLOAT
			};

			if ((Error = PipelineState->Create(new RPipelineState::InitializeOptionsGraphics(
				Shaders,
				InputLayout,
				Blend,
				Rasterizer,
				D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
				_countof(RTVFormats),
				RTVFormats))))
			{
				return Error;
			}
		}

		return S_OK;
	}


	ErrorCode D3D::Pipelines::VolumetricLighting::TemporalFilter::CreateRootSignature()
	{
		ErrorCode Error;

		CD3DX12_ROOT_PARAMETER RootParameters[2];
		{
			CD3DX12_DESCRIPTOR_RANGE DescriptorRanges[3];
			{
				DescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, NumConstantBuffers, 0);
				DescriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, TARGET_ACCUMULATION_RESOLVED);
				DescriptorRanges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0, TARGET_DEPTH_RESOLVED);
			}

			RootParameters[PARAMETER_CBV_SRV_DESCRIPTOR_TABLE].InitAsDescriptorTable(_countof(DescriptorRanges), DescriptorRanges, D3D12_SHADER_VISIBILITY_ALL);

			CD3DX12_DESCRIPTOR_RANGE DescriptorRangesPerFrame[2];
			{
				DescriptorRangesPerFrame[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, TARGET_ACCUMULATION_FILTERED);
				DescriptorRangesPerFrame[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 0, TARGET_DEPTH_FILTERED);
			}

			RootParameters[PARAMETER_SRV_FRAME_DESCRIPTOR_TABLE].InitAsDescriptorTable(_countof(DescriptorRangesPerFrame), DescriptorRangesPerFrame, D3D12_SHADER_VISIBILITY_PIXEL);
		}

		D3D12_STATIC_SAMPLER_DESC Samplers[2];
		{
			Samplers[0] = SamplerStates::PointSampler
			(
				0,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP
			);

			Samplers[1] = SamplerStates::LinearSampler
			(
				1,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP
			);
		}

		RootSignature = new RRootSignature();
		{
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

	ErrorCode D3D::Pipelines::VolumetricLighting::TemporalFilter::CreatePipelineState()
	{
		ErrorCode Error;

		static SharedPointer<CGrpShader> Shaders = new CGrpShader(this);
		{
			if ((Error = Shaders->CompileShaderType(CGrpShader::Vertex, L"VolumetricLighting\\QuadVS.hlsl", "VSMain2", "vs_5_0")))
			{
				return Error;
			}

			if ((Error = Shaders->CompileShaderType(CGrpShader::Pixel, L"VolumetricLighting\\TemporalFilterPS.hlsl", "main", "ps_5_0")))
			{
				return Error;
			}
		}
		
		D3D12_RASTERIZER_DESC Rasterizer = RasterizerState::CullNone;
		D3D12_INPUT_LAYOUT_DESC InputLayout = {};
		{
			InputLayout.NumElements = 0;
			InputLayout.pInputElementDescs = NULL;
		}
		D3D12_DEPTH_STENCIL_DESC DepthStencil = DepthStencilState::None;
		D3D12_BLEND_DESC Blend = BlendState::NoBlend;

		PipelineState = new RPipelineState(RootSignature);
		{
			DXGI_FORMAT RTVFormats[] =
			{
				DXGI_FORMAT_R16G16B16A16_FLOAT,
				DXGI_FORMAT_R16G16_FLOAT
			};

			if ((Error = PipelineState->Create(new RPipelineState::InitializeOptionsGraphics(
				Shaders,
				InputLayout,
				Blend,
				Rasterizer,
				D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
				_countof(RTVFormats),
				RTVFormats))))
			{
				return Error;
			}
		}

		return S_OK;
	}


	ErrorCode D3D::Pipelines::VolumetricLighting::ApplyLighting::CreateRootSignature()
	{
		ErrorCode Error;

		CD3DX12_ROOT_PARAMETER RootParameters[3];
		{
			CD3DX12_DESCRIPTOR_RANGE DescriptorRanges[2];
			{
				DescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, NumConstantBuffers, 0);
				DescriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 0, TARGET_LUT);
			}

			RootParameters[PARAMETER_CBV_SRV_DESCRIPTOR_TABLE].InitAsDescriptorTable(_countof(DescriptorRanges), DescriptorRanges, D3D12_SHADER_VISIBILITY_ALL);

			CD3DX12_DESCRIPTOR_RANGE DescriptorRangesPerFrame[2];
			{
				DescriptorRangesPerFrame[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0, TARGET_DEPTH_FILTERED);
				DescriptorRangesPerFrame[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, TARGET_ACCUMULATION_FILTERED);
			}

			RootParameters[PARAMETER_SRV_FRAME_DESCRIPTOR_TABLE].InitAsDescriptorTable(_countof(DescriptorRangesPerFrame), DescriptorRangesPerFrame, D3D12_SHADER_VISIBILITY_PIXEL);

			CD3DX12_DESCRIPTOR_RANGE DescriptorRangeScene[1];
			{
				DescriptorRangeScene[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, SRV_DEPTH);
			}

			RootParameters[PARAMETER_SRV_DEPTH_DESCRIPTOR_TABLE].InitAsDescriptorTable(_countof(DescriptorRangeScene), DescriptorRangeScene, D3D12_SHADER_VISIBILITY_PIXEL);
		}

		D3D12_STATIC_SAMPLER_DESC Samplers[2];
		{
			Samplers[0] = SamplerStates::PointSampler
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
		}

		RootSignature = new RRootSignature();
		{
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

	ErrorCode D3D::Pipelines::VolumetricLighting::ApplyLighting::CreatePipelineState()
	{
		ErrorCode Error;

		D3D_SHADER_MACRO PSMacros[4] = {};
		{
			PSMacros[0].Name		= "FOGMODE";
			PSMacros[0].Definition	= "FOGMODE_FULL";

			PSMacros[1].Name		= "UPSAMPLEMODE";
			PSMacros[1].Definition	= "UPSAMPLEMODE_BILINEAR";

			PSMacros[2].Name		= "SAMPLEMODE";
			PSMacros[2].Definition	= "SAMPLEMODE_SINGLE";
		}

		static SharedPointer<CGrpShader> Shaders = new CGrpShader(this);
		{
			if ((Error = Shaders->CompileShaderType(CGrpShader::Vertex, L"VolumetricLighting\\QuadVS.hlsl", "VSMain2", "vs_5_0")))
			{
				return Error;
			}

			if ((Error = Shaders->CompileShaderType(CGrpShader::Pixel, L"VolumetricLighting\\ApplyPS.hlsl", "main", "ps_5_0", 0, PSMacros)))
			{
				return Error;
			}
		}

		D3D12_RASTERIZER_DESC Rasterizer = RasterizerState::CullNone;
		D3D12_INPUT_LAYOUT_DESC InputLayout = {};
		{
			InputLayout.NumElements = 0;
			InputLayout.pInputElementDescs = NULL;
		}
		D3D12_DEPTH_STENCIL_DESC DepthStencil = DepthStencilState::None;
		D3D12_BLEND_DESC Blend = BlendState::AdditiveModulate;

		PipelineState = new RPipelineState(RootSignature);
		{
			DXGI_FORMAT RTVFormats[] =
			{
				CScene::ColorSceneFormat,
				CScene::ColorSceneFormat
			};

			if ((Error = PipelineState->Create(new RPipelineState::InitializeOptionsGraphics(
				Shaders,
				InputLayout,
				Blend,
				Rasterizer,
				D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
				_countof(RTVFormats),
				RTVFormats))))
			{
				return Error;
			}
		}

		return S_OK;
	}


	ErrorCode D3D::Pipelines::VolumetricLighting::ApplyLightingMSAA::CreateRootSignature()
	{
		ErrorCode Error;

		CD3DX12_ROOT_PARAMETER RootParameters[3];
		{
			CD3DX12_DESCRIPTOR_RANGE DescriptorRanges[3];
			{
				DescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, NumConstantBuffers, 0);
				DescriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, TARGET_ACCUMULATION_RESOLVED);
				DescriptorRanges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 0, TARGET_LUT);
			}

			RootParameters[PARAMETER_CBV_SRV_DESCRIPTOR_TABLE].InitAsDescriptorTable(_countof(DescriptorRanges), DescriptorRanges, D3D12_SHADER_VISIBILITY_ALL);

			CD3DX12_DESCRIPTOR_RANGE DescriptorRangesPerFrame[1];
			{
				DescriptorRangesPerFrame[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0, TARGET_DEPTH_FILTERED);
			}

			RootParameters[PARAMETER_SRV_FRAME_DESCRIPTOR_TABLE].InitAsDescriptorTable(_countof(DescriptorRangesPerFrame), DescriptorRangesPerFrame, D3D12_SHADER_VISIBILITY_PIXEL);

			CD3DX12_DESCRIPTOR_RANGE DescriptorRangeScene[1];
			{
				DescriptorRangeScene[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, 0);
			}

			RootParameters[PARAMETER_SRV_DEPTH_DESCRIPTOR_TABLE].InitAsDescriptorTable(_countof(DescriptorRangeScene), DescriptorRangeScene, D3D12_SHADER_VISIBILITY_PIXEL);
		}

		D3D12_STATIC_SAMPLER_DESC Samplers[2];
		{
			Samplers[0] = SamplerStates::PointSampler
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
		}

		RootSignature = new RRootSignature();
		{
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


	ErrorCode D3D::Pipelines::VolumetricLighting::ApplyLightingMSAA::CreatePipelineState()
	{
		ErrorCode Error;

		D3D_SHADER_MACRO PSMacros[4] = {};
		{
			PSMacros[0].Name		= "FOGMODE";
			PSMacros[0].Definition	= "FOGMODE_FULL";

			PSMacros[1].Name		= "UPSAMPLEMODE";
			PSMacros[1].Definition	= "UPSAMPLEMODE_BILATERAL";

			PSMacros[2].Name		= "SAMPLEMODE";
			PSMacros[2].Definition	= "SAMPLEMODE_SINGLE";
		}

		static SharedPointer<CGrpShader> Shaders = new CGrpShader(this);
		{
			if ((Error = Shaders->CompileShaderType(CGrpShader::Vertex, L"VolumetricLighting\\QuadVS.hlsl", "VSMain2", "vs_5_0")))
			{
				return Error;
			}

			if ((Error = Shaders->CompileShaderType(CGrpShader::Pixel, L"VolumetricLighting\\ApplyPS.hlsl", "main", "ps_5_0", 0, PSMacros)))
			{
				return Error;
			}
		}

		D3D12_RASTERIZER_DESC Rasterizer = RasterizerState::CullNone;
		D3D12_INPUT_LAYOUT_DESC InputLayout = {};
		{
			InputLayout.NumElements = 0;
			InputLayout.pInputElementDescs = NULL;
		}
		D3D12_DEPTH_STENCIL_DESC DepthStencil = DepthStencilState::None;
		D3D12_BLEND_DESC Blend = BlendState::AdditiveModulate;

		PipelineState = new RPipelineState(RootSignature);
		{
			if ((Error = PipelineState->Create(new RPipelineState::InitializeOptionsGraphics(
				Shaders,
				InputLayout,
				Blend,
				Rasterizer,
				D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
				DXGI_FORMAT_R8G8B8A8_UNORM))))
			{
				return Error;
			}
		}

		return S_OK;
	}


	ErrorCode D3D::Pipelines::VolumetricLighting::ComputePhaseLookup::CreateRootSignature()
	{
		CD3DX12_ROOT_PARAMETER RootParameters[1];
		{
			CD3DX12_DESCRIPTOR_RANGE DescriptorRanges[1];
			{
				DescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, NumConstantBuffers, 0);
			}

			RootParameters[PARAMETER_CBV_SRV_DESCRIPTOR_TABLE].InitAsDescriptorTable(_countof(DescriptorRanges), DescriptorRanges, D3D12_SHADER_VISIBILITY_ALL);
		}

		ErrorCode Error;

		RootSignature = new RRootSignature();
		{
			if ((Error = RootSignature->Create(RRootSignature::InitializeOptions(
				_countof(RootParameters),
				RootParameters,
				0,
				NULL))))
			{
				return Error;
			}
		}

		return S_OK;
	}

	ErrorCode D3D::Pipelines::VolumetricLighting::ComputePhaseLookup::CreatePipelineState()
	{
		ErrorCode Error;

		static SharedPointer<CGrpShader> Shaders = new CGrpShader(this);
		{
			if ((Error = Shaders->CompileShaderType(CGrpShader::Vertex, L"VolumetricLighting\\QuadVS.hlsl", "VSMain2", "vs_5_0")))
			{
				return Error;
			}

			if ((Error = Shaders->CompileShaderType(CGrpShader::Pixel, L"VolumetricLighting\\ComputePhaseLookupPS.hlsl", "main", "ps_5_0")))
			{
				return Error;
			}
		}

		D3D12_RASTERIZER_DESC Rasterizer = RasterizerState::CullNone;
		D3D12_INPUT_LAYOUT_DESC InputLayout = {};
		{
			InputLayout.NumElements = 0;
			InputLayout.pInputElementDescs = NULL;
		}
		D3D12_BLEND_DESC Blend = BlendState::NoBlend;

		PipelineState = new RPipelineState(RootSignature);
		{
			if ((Error = PipelineState->Create(new RPipelineState::InitializeOptionsGraphics(
				Shaders,
				InputLayout,
				Blend,
				Rasterizer,
				D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
				DXGI_FORMAT_R16G16B16A16_FLOAT))))
			{
				return Error;
			}
		}

		return S_OK;
	}

	ErrorCode Pipelines::VolumetricLighting::Directional::PipelineSetup::CreateRootSignature()
	{
		ErrorCode Error;

		CD3DX12_ROOT_PARAMETER RootParameters[2];
		{
			CD3DX12_DESCRIPTOR_RANGE DescriptorRanges[5];
			{
				DescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, NumConstantBuffers, 0);
				DescriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 0, TARGET_LUT);
				DescriptorRanges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5, 0, TARGET_LUT_P_0);
				DescriptorRanges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6, 0, TARGET_LUT_S1_0);
				DescriptorRanges[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 7, 0, TARGET_LUT_S2_0);
			}

			RootParameters[PARAMETER_CBV_SRV_DESCRIPTOR_TABLE].InitAsDescriptorTable(_countof(DescriptorRanges), DescriptorRanges, D3D12_SHADER_VISIBILITY_ALL);

			CD3DX12_DESCRIPTOR_RANGE DescriptorRangeScene[2];
			{
				DescriptorRangeScene[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0, SRV_DEPTH);
				DescriptorRangeScene[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, SRV_SHADOW);
			}

			RootParameters[PARAMETER_SRV_DEPTH_DESCRIPTOR_TABLE].InitAsDescriptorTable(_countof(DescriptorRangeScene), DescriptorRangeScene, D3D12_SHADER_VISIBILITY_ALL);
		}

		D3D12_STATIC_SAMPLER_DESC Samplers[2];
		{
			Samplers[0] = SamplerStates::PointSampler
			(
				0,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP
			);

			Samplers[1] = SamplerStates::LinearSampler
			(
				1,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP
			);
		}

		RootSignature = new RRootSignature();
		{
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

	namespace Pipelines
	{
		namespace VolumetricLighting
		{
			namespace Directional
			{
				void PipelineSetup::InitializeMacros(const EPassModes Pass, const ELightModes Light, const EFallOffModes FallOff, const EMeshModes Mesh, const EVolumeModes Volume)
				{
					PSMacros[0].Definition = GetPassMode(Pass);
					PSMacros[0].Name = "PASSMODE";
					PSMacros[1].Definition = GetLightMode(Light);
					PSMacros[1].Name = "LIGHTMODE";
					PSMacros[2].Definition = GetFallOffMode(FallOff);
					PSMacros[2].Name = "FALLOFFMODE";

					VSMacros[0].Definition = GetMeshMode(Mesh);
					VSMacros[0].Name = "MESHMODE";

					DSMacros[0].Definition = GetVolumeMode(Volume);
					DSMacros[0].Name = "VOLUMETYPE";
					DSMacros[1].Definition = "SHADOWMAPTYPE_ATLAS";
					DSMacros[1].Name = "SHADOWMAPTYPE";
					DSMacros[2].Definition = "CASCADECOUNT_4";
					DSMacros[2].Name = "CASCADECOUNT";

					HSMacros[0].Definition = GetVolumeMode(Volume);
					HSMacros[0].Name = "VOLUMETYPE";
					HSMacros[1].Definition = "SHADOWMAPTYPE_ATLAS";
					HSMacros[1].Name = "SHADOWMAPTYPE";
					HSMacros[2].Definition = "CASCADECOUNT_4";
					HSMacros[2].Name = "CASCADECOUNT";
					HSMacros[3].Definition = "MAXTESSFACTOR_HIGH";
					HSMacros[3].Name = "MAXTESSFACTOR";
				}

				ErrorCode PipelineSetup::InitializePipelineStateGeometryGrid()
				{
					ErrorCode Error;

					static SharedPointer<CGrpShader> Shaders = new CGrpShader(this);
					{
						if ((Error = Shaders->CompileShaderType(CGrpShader::Vertex, L"VolumetricLighting\\RenderVolumeVS.hlsl", "Main", "vs_5_0", NULL, VSMacros)))
						{
							return Error;
						}

						if ((Error = Shaders->CompileShaderType(CGrpShader::Pixel, L"VolumetricLighting\\RenderVolumePS.hlsl", "Main", "ps_5_0", NULL, PSMacros)))
						{
							return Error;
						}

						if ((Error = Shaders->CompileShaderType(CGrpShader::Hull, L"VolumetricLighting\\RenderVolumeHS.hlsl", "Main", "hs_5_0", NULL, HSMacros)))
						{
							return Error;
						}

						if ((Error = Shaders->CompileShaderType(CGrpShader::Domain, L"VolumetricLighting\\RenderVolumeDS.hlsl", "Main", "ds_5_0", NULL, DSMacros)))
						{
							return Error;
						}
					}

					D3D12_RASTERIZER_DESC Rasterizer = RasterizerState::CullNone;
					D3D12_INPUT_LAYOUT_DESC InputLayout = {};
					{
						InputLayout.NumElements = 0;
						InputLayout.pInputElementDescs = NULL;
					}
					D3D12_BLEND_DESC Blend = BlendState::Additive;
					CD3DX12_DEPTH_STENCIL_DESC DepthStencil(D3D12_DEFAULT);
					{
						DepthStencil.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
						DepthStencil.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
						DepthStencil.StencilEnable = TRUE;
						DepthStencil.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
						DepthStencil.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
						DepthStencil.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_INCR;
						DepthStencil.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
						DepthStencil.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
						DepthStencil.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
						DepthStencil.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_DECR;
						DepthStencil.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
					}

					PipelineState = new RPipelineState(RootSignature);
					{
						if ((Error = PipelineState->Create(new RPipelineState::InitializeOptionsGraphics(
							Shaders,
							InputLayout,
							Blend,
							Rasterizer,
							DepthStencil,
							D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH,
							DXGI_FORMAT_R16G16B16A16_FLOAT,
							DXGI_FORMAT_D32_FLOAT_S8X24_UINT))))
						{
							return Error;
						}
					}

					return S_OK;
				}

				ErrorCode PipelineSetup::InitializePipelineStateGeometryBase()
				{
					ErrorCode Error;

					static SharedPointer<CGrpShader> Shaders = new CGrpShader(this);
					{
						if ((Error = Shaders->CompileShaderType(CGrpShader::Vertex, L"VolumetricLighting\\RenderVolumeVS.hlsl", "Main", "vs_5_0", NULL, VSMacros)))
						{
							return Error;
						}

						if ((Error = Shaders->CompileShaderType(CGrpShader::Pixel, L"VolumetricLighting\\RenderVolumePS.hlsl", "Main", "ps_5_0", NULL, PSMacros)))
						{
							return Error;
						}
					}

					D3D12_RASTERIZER_DESC Rasterizer = RasterizerState::CullNone;
					D3D12_INPUT_LAYOUT_DESC InputLayout = {};
					{
						InputLayout.NumElements = 0;
						InputLayout.pInputElementDescs = NULL;
					}
					D3D12_BLEND_DESC Blend = BlendState::Additive;
					CD3DX12_DEPTH_STENCIL_DESC DepthStencil(D3D12_DEFAULT);
					{
						DepthStencil.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
						DepthStencil.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
						DepthStencil.StencilEnable = TRUE;
						DepthStencil.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
						DepthStencil.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
						DepthStencil.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_INCR;
						DepthStencil.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
						DepthStencil.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
						DepthStencil.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
						DepthStencil.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_DECR;
						DepthStencil.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
					}

					PipelineState = new RPipelineState(RootSignature);
					{
						if ((Error = PipelineState->Create(new RPipelineState::InitializeOptionsGraphics(
							Shaders,
							InputLayout,
							Blend,
							Rasterizer,
							DepthStencil,
							D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
							DXGI_FORMAT_R16G16B16A16_FLOAT,
							DXGI_FORMAT_D32_FLOAT_S8X24_UINT))))
						{
							return Error;
						}
					}
					
					return S_OK;
				}

				ErrorCode PipelineSetup::InitializePipelineStateFinal()
				{
					ErrorCode Error;

					static SharedPointer<CGrpShader> Shaders = new CGrpShader(this);
					{
						if ((Error = Shaders->CompileShaderType(CGrpShader::Vertex, L"VolumetricLighting\\QuadVS.hlsl", "VSMain2", "vs_5_0", NULL, VSMacros)))
						{
							return Error;
						}

						if ((Error = Shaders->CompileShaderType(CGrpShader::Pixel, L"VolumetricLighting\\RenderVolumePS.hlsl", "Main", "ps_5_0", NULL, PSMacros)))
						{
							return Error;
						}
					}

					D3D12_RASTERIZER_DESC Rasterizer = RasterizerState::CullNone;
					D3D12_INPUT_LAYOUT_DESC InputLayout = {};
					{
						InputLayout.NumElements = 0;
						InputLayout.pInputElementDescs = NULL;
					}
					D3D12_BLEND_DESC Blend = BlendState::Additive;
					CD3DX12_DEPTH_STENCIL_DESC DepthStencil(D3D12_DEFAULT);
					{
						DepthStencil.DepthEnable = FALSE;
						DepthStencil.StencilEnable = TRUE;
						DepthStencil.StencilWriteMask = 0x00;
						DepthStencil.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
						DepthStencil.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
						DepthStencil.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
						DepthStencil.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
						DepthStencil.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_GREATER;
						DepthStencil.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
						DepthStencil.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
						DepthStencil.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
					}

					PipelineState = new RPipelineState(RootSignature);
					{
						if ((Error = PipelineState->Create(new RPipelineState::InitializeOptionsGraphics(
							Shaders,
							InputLayout,
							Blend,
							Rasterizer,
							DepthStencil,
							D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
							DXGI_FORMAT_R16G16B16A16_FLOAT,
							DXGI_FORMAT_D32_FLOAT_S8X24_UINT))))
						{
							return Error;
						}
					}

					return S_OK;
				}

				ErrorCode PipelineSetup::InitializePipelineStateSky()
				{
					ErrorCode Error;

					static SharedPointer<CGrpShader> Shaders = new CGrpShader(this);
					{
						if ((Error = Shaders->CompileShaderType(CGrpShader::Vertex, L"VolumetricLighting\\QuadVS.hlsl", "VSMain2", "vs_5_0", NULL, VSMacros)))
						{
							return Error;
						}

						if ((Error = Shaders->CompileShaderType(CGrpShader::Pixel, L"VolumetricLighting\\RenderVolumePS.hlsl", "Main", "ps_5_0", NULL, PSMacros)))
						{
							return Error;
						}
					}

					D3D12_RASTERIZER_DESC Rasterizer = RasterizerState::CullNone;
					D3D12_INPUT_LAYOUT_DESC InputLayout = {};
					{
						InputLayout.NumElements = 0;
						InputLayout.pInputElementDescs = NULL;
					}
					D3D12_BLEND_DESC Blend = BlendState::Additive;
					CD3DX12_DEPTH_STENCIL_DESC DepthStencil(D3D12_DEFAULT);
					{
						DepthStencil.StencilEnable = TRUE;
						DepthStencil.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
						DepthStencil.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
						DepthStencil.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
						DepthStencil.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
						DepthStencil.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
						DepthStencil.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
						DepthStencil.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
						DepthStencil.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
						DepthStencil.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_DECR;
						DepthStencil.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
					}

					PipelineState = new RPipelineState(RootSignature);
					{
						if ((Error = PipelineState->Create(new RPipelineState::InitializeOptionsGraphics(
							Shaders,
							InputLayout,
							Blend,
							Rasterizer,
							DepthStencil,
							D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
							DXGI_FORMAT_R16G16B16A16_FLOAT,
							DXGI_FORMAT_D32_FLOAT_S8X24_UINT))))
						{
							return Error;
						}
					}

					return S_OK;
				}


				ErrorCode PipelineSky::CreatePipelineState()
				{
					return InitializePipelineStateSky();
				}

				ErrorCode PipelineGeometryFrustumGrid::CreatePipelineState()
				{
					return InitializePipelineStateGeometryGrid();
				}

				ErrorCode PipelineGeometryFrustumBase::CreatePipelineState()
				{
					return InitializePipelineStateGeometryBase();
				}

				ErrorCode PipelineFinal::CreatePipelineState()
				{
					return InitializePipelineStateFinal();
				}
			}
		}
	}
}
