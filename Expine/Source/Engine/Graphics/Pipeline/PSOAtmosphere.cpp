#include "Precompiled.h"

#include "Pipeline/PSOAtmosphere.h"
#include "Scene/Scene.h"
#include "Utils/State/StateRasterizer.h"
#include "Utils/State/StateDepthStencil.h"
#include "Utils/State/StateBlend.h"
#include "Utils/State/StateSampler.h"
#include "Scene/Outdoor/AtmosphericScattering.h"

namespace D3D
{
	namespace Pipelines
	{
		namespace AtmosphericScattering
		{
			static D3D12_STATIC_SAMPLER_DESC SamplerDesc[NumSRV + 1] = {};

			static SharedPointer<RRootSignature> DefaultRootSignature;

			static TArray<SharedPointer<RPipelineState>, eNumPipelines> Pipelines;
			static TArray<SharedPointer<CGrpShader>, eNumPipelines> Shaders;

			static ErrorCode CompileShaders(EPipelines PipelineType)
			{
				ErrorCode Error;

				switch (PipelineType)
				{
					case eTransmittancePS:
					{
						if ((Error = Shaders[eTransmittancePS]->CompileShaderType(CGrpShader::Pixel, L"Atmosphere.hlsl", "TransmittancePS", "ps_5_0")))
						{
							return Error;
						}

						if ((Error = Shaders[eTransmittancePS]->CompileShaderType(CGrpShader::Vertex, L"Atmosphere.hlsl", "MainVS", "vs_5_0")))
						{
							return Error;
						}
					}

					break;

					case eIrradiancePS:
					{
						if ((Error = Shaders[eIrradiancePS]->CompileShaderType(CGrpShader::Pixel, L"Atmosphere.hlsl", "Irradiance1PS", "ps_5_0")))
						{
							return Error;
						}

						if ((Error = Shaders[eIrradiancePS]->CompileShaderType(CGrpShader::Vertex, L"Atmosphere.hlsl", "MainVS", "vs_5_0")))
						{
							return Error;
						}
					}

					break;

					case eIrradianceNPS:
					{
						if ((Error = Shaders[eIrradianceNPS]->CompileShaderType(CGrpShader::Pixel, L"Atmosphere.hlsl", "IrradianceNPS", "ps_5_0")))
						{
							return Error;
						}

						if ((Error = Shaders[eIrradianceNPS]->CompileShaderType(CGrpShader::Vertex, L"Atmosphere.hlsl", "MainVS", "vs_5_0")))
						{
							return Error;
						}
					}

					break;

					case eIrradianceCopyPS:
					{
						if ((Error = Shaders[eIrradianceCopyPS]->CompileShaderType(CGrpShader::Pixel, L"Atmosphere.hlsl", "CopyIrradiancePS", "ps_5_0")))
						{
							return Error;
						}

						if ((Error = Shaders[eIrradianceCopyPS]->CompileShaderType(CGrpShader::Vertex, L"Atmosphere.hlsl", "MainVS", "vs_5_0")))
						{
							return Error;
						}
					}

					break;

					case eInscatterPS:
					{
						if ((Error = Shaders[eInscatterPS]->CompileShaderType(CGrpShader::Pixel, L"Atmosphere.hlsl", "Inscatter1PS", "ps_5_0")))
						{
							return Error;
						}

						if ((Error = Shaders[eInscatterPS]->CompileShaderType(CGrpShader::Vertex, L"Atmosphere.hlsl", "MainVS", "vs_5_0")))
						{
							return Error;
						}

						if ((Error = Shaders[eInscatterPS]->CompileShaderType(CGrpShader::Geometry, L"Atmosphere.hlsl", "AtmosphereGS", "gs_5_0")))
						{
							return Error;
						}
					}

					break;

					case eInscatterCopyPS:
					{
						if ((Error = Shaders[eInscatterCopyPS]->CompileShaderType(CGrpShader::Pixel, L"Atmosphere.hlsl", "CopyInscatter1PS", "ps_5_0")))
						{
							return Error;
						}

						if ((Error = Shaders[eInscatterCopyPS]->CompileShaderType(CGrpShader::Vertex, L"Atmosphere.hlsl", "MainVS", "vs_5_0")))
						{
							return Error;
						}

						if ((Error = Shaders[eInscatterCopyPS]->CompileShaderType(CGrpShader::Geometry, L"Atmosphere.hlsl", "AtmosphereGS", "gs_5_0")))
						{
							return Error;
						}
					}

					break;

					case eInscatterCopyNPS:
					{
						if ((Error = Shaders[eInscatterCopyNPS]->CompileShaderType(CGrpShader::Pixel, L"Atmosphere.hlsl", "CopyInscatterNPS", "ps_5_0")))
						{
							return Error;
						}

						if ((Error = Shaders[eInscatterCopyNPS]->CompileShaderType(CGrpShader::Vertex, L"Atmosphere.hlsl", "MainVS", "vs_5_0")))
						{
							return Error;
						}

						if ((Error = Shaders[eInscatterCopyNPS]->CompileShaderType(CGrpShader::Geometry, L"Atmosphere.hlsl", "AtmosphereGS", "gs_5_0")))
						{
							return Error;
						}
					}

					break;

					case eInscatterSPS:
					{
						if ((Error = Shaders[eInscatterSPS]->CompileShaderType(CGrpShader::Pixel, L"Atmosphere.hlsl", "InscatterSPS", "ps_5_0")))
						{
							return Error;
						}

						if ((Error = Shaders[eInscatterSPS]->CompileShaderType(CGrpShader::Vertex, L"Atmosphere.hlsl", "MainVS", "vs_5_0")))
						{
							return Error;
						}

						if ((Error = Shaders[eInscatterSPS]->CompileShaderType(CGrpShader::Geometry, L"Atmosphere.hlsl", "AtmosphereGS", "gs_5_0")))
						{
							return Error;
						}
					}

					break;

					case eInscatterNPS:
					{
						if ((Error = Shaders[eInscatterNPS]->CompileShaderType(CGrpShader::Pixel, L"Atmosphere.hlsl", "InscatterNPS", "ps_5_0")))
						{
							return Error;
						}

						if ((Error = Shaders[eInscatterNPS]->CompileShaderType(CGrpShader::Vertex, L"Atmosphere.hlsl", "MainVS", "vs_5_0")))
						{
							return Error;
						}

						if ((Error = Shaders[eInscatterNPS]->CompileShaderType(CGrpShader::Geometry, L"Atmosphere.hlsl", "AtmosphereGS", "gs_5_0")))
						{
							return Error;
						}
					}

					case eInscatterCopyFPS:
					{
						if ((Error = Shaders[eInscatterCopyFPS]->CompileShaderType(CGrpShader::Pixel, L"Atmosphere.hlsl", "CopyInscatterFPS", "ps_5_0")))
						{
							return Error;
						}

						if ((Error = Shaders[eInscatterCopyFPS]->CompileShaderType(CGrpShader::Vertex, L"Atmosphere.hlsl", "MainVS", "vs_5_0")))
						{
							return Error;
						}

						if ((Error = Shaders[eInscatterCopyFPS]->CompileShaderType(CGrpShader::Geometry, L"Atmosphere.hlsl", "AtmosphereGS", "gs_5_0")))
						{
							return Error;
						}
					}

					case eInscatterCopyFBackPS:
					{
						if ((Error = Shaders[eInscatterCopyFBackPS]->CompileShaderType(CGrpShader::Pixel, L"Atmosphere.hlsl", "CopyInscatterFBackPS", "ps_5_0")))
						{
							return Error;
						}

						if ((Error = Shaders[eInscatterCopyFBackPS]->CompileShaderType(CGrpShader::Vertex, L"Atmosphere.hlsl", "MainVS", "vs_5_0")))
						{
							return Error;
						}

						if ((Error = Shaders[eInscatterCopyFBackPS]->CompileShaderType(CGrpShader::Geometry, L"Atmosphere.hlsl", "AtmosphereGS", "gs_5_0")))
						{
							return Error;
						}
					}

					break;
				}

				return S_OK;
			}

			static ErrorCode CreatePipeline(EPipelines PipelineType)
			{
				ErrorCode Error;

				if ((Error = CompileShaders(PipelineType)))
				{
					return Error;
				}

				const D3D12_INPUT_LAYOUT_DESC InputLayoutDesc = CD3DX12_INPUT_LAYOUT_DESC
				(
					0,
					NULL
				);

				D3D12_BLEND_DESC BlendDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
				D3D12_RASTERIZER_DESC RasterizerDesc = RasterizerState::CullNone;
				D3D12_DEPTH_STENCIL_DESC DepthStencilDesc = DepthStencilState::None;

				DXGI_FORMAT Formats[2] =
				{
					Atmosphere::InscatterFormat,
					Atmosphere::InscatterFormat
				};

				UINT NumTargets = 1;

				switch (PipelineType)
				{
					case eIrradianceCopyPS:
					{
						BlendDesc.RenderTarget[0].BlendOp				= D3D12_BLEND_OP_ADD;
						BlendDesc.RenderTarget[0].BlendOpAlpha			= D3D12_BLEND_OP_ADD;
						BlendDesc.RenderTarget[0].BlendEnable			= TRUE;
						BlendDesc.RenderTarget[0].DestBlend				= D3D12_BLEND_ONE;
						BlendDesc.RenderTarget[0].SrcBlend				= D3D12_BLEND_ONE;
						BlendDesc.RenderTarget[0].DestBlendAlpha		= D3D12_BLEND_ONE;
						BlendDesc.RenderTarget[0].SrcBlendAlpha			= D3D12_BLEND_ONE;
						BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
					}
					default:
					{
						Formats[0] = Atmosphere::TransmittanceFormat;
					}

					break;

					case eInscatterPS:
					{
						NumTargets = 2;

						Formats[0] = Atmosphere::InscatterFormat;
						Formats[1] = Atmosphere::InscatterFormat;
					}

					break;

					case eInscatterCopyNPS:
					{
						BlendDesc.RenderTarget[0].BlendOp				= D3D12_BLEND_OP_ADD;
						BlendDesc.RenderTarget[0].BlendOpAlpha			= D3D12_BLEND_OP_ADD;
						BlendDesc.RenderTarget[0].BlendEnable			= TRUE;
						BlendDesc.RenderTarget[0].DestBlend				= D3D12_BLEND_ONE;
						BlendDesc.RenderTarget[0].SrcBlend				= D3D12_BLEND_ONE;
						BlendDesc.RenderTarget[0].DestBlendAlpha		= D3D12_BLEND_ONE;
						BlendDesc.RenderTarget[0].SrcBlendAlpha			= D3D12_BLEND_ONE;
						BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
					}
					case eInscatterCopyFBackPS:
					case eInscatterCopyFPS:
					case eInscatterCopyPS:
					case eInscatterSPS:
					case eInscatterNPS:
					{
						Formats[0] = Atmosphere::InscatterFormat;
						Formats[1] = Atmosphere::InscatterFormat;
					}

					break;
				}

				Pipelines[PipelineType] = new RPipelineState(DefaultRootSignature);

				if ((Error = Pipelines[PipelineType]->Create(new RPipelineState::InitializeOptionsGraphics(
					Shaders[PipelineType],
					InputLayoutDesc,
					BlendDesc,
					RasterizerDesc,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
					NumTargets,
					Formats))))
				{
					return Error;
				}

				return S_OK;
			}

			static ErrorCode InitializeRootSignatures()
			{
				ErrorCode Error;

				SamplerDesc[SRV_Transmittance].Filter				= D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
				SamplerDesc[SRV_Transmittance].AddressU				= D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				SamplerDesc[SRV_Transmittance].AddressV				= D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				SamplerDesc[SRV_Transmittance].AddressW				= D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				SamplerDesc[SRV_Transmittance].MipLODBias			= 0;
				SamplerDesc[SRV_Transmittance].MaxAnisotropy		= 1;
				SamplerDesc[SRV_Transmittance].ComparisonFunc		= D3D12_COMPARISON_FUNC_LESS_EQUAL;
				SamplerDesc[SRV_Transmittance].BorderColor			= D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
				SamplerDesc[SRV_Transmittance].MinLOD				= 0.0f;
				SamplerDesc[SRV_Transmittance].MaxLOD				= D3D12_FLOAT32_MAX;
				SamplerDesc[SRV_Transmittance].ShaderRegister		= SRV_Transmittance;
				SamplerDesc[SRV_Transmittance].RegisterSpace		= 0;
				SamplerDesc[SRV_Transmittance].ShaderVisibility		= D3D12_SHADER_VISIBILITY_PIXEL;

				// Irradiance
				SamplerDesc[SRV_Irradiance].Filter					= D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
				SamplerDesc[SRV_Irradiance].AddressU				= D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				SamplerDesc[SRV_Irradiance].AddressV				= D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				SamplerDesc[SRV_Irradiance].AddressW				= D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				SamplerDesc[SRV_Irradiance].MipLODBias				= 0;
				SamplerDesc[SRV_Irradiance].MaxAnisotropy			= 1;
				SamplerDesc[SRV_Irradiance].ComparisonFunc			= D3D12_COMPARISON_FUNC_LESS_EQUAL;
				SamplerDesc[SRV_Irradiance].BorderColor				= D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
				SamplerDesc[SRV_Irradiance].MinLOD					= 0.0f;
				SamplerDesc[SRV_Irradiance].MaxLOD					= D3D12_FLOAT32_MAX;
				SamplerDesc[SRV_Irradiance].ShaderRegister			= SRV_Irradiance;
				SamplerDesc[SRV_Irradiance].RegisterSpace			= 0;
				SamplerDesc[SRV_Irradiance].ShaderVisibility		= D3D12_SHADER_VISIBILITY_PIXEL;

				// Inscatter
				SamplerDesc[SRV_Inscatter].Filter					= D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
				SamplerDesc[SRV_Inscatter].AddressU					= D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				SamplerDesc[SRV_Inscatter].AddressV					= D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				SamplerDesc[SRV_Inscatter].AddressW					= D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				SamplerDesc[SRV_Inscatter].MipLODBias				= 0;
				SamplerDesc[SRV_Inscatter].MaxAnisotropy			= 1;
				SamplerDesc[SRV_Inscatter].ComparisonFunc			= D3D12_COMPARISON_FUNC_LESS_EQUAL;
				SamplerDesc[SRV_Inscatter].BorderColor				= D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
				SamplerDesc[SRV_Inscatter].MinLOD					= 0.0f;
				SamplerDesc[SRV_Inscatter].MaxLOD					= D3D12_FLOAT32_MAX;
				SamplerDesc[SRV_Inscatter].ShaderRegister			= SRV_Inscatter;
				SamplerDesc[SRV_Inscatter].RegisterSpace			= 0;
				SamplerDesc[SRV_Inscatter].ShaderVisibility			= D3D12_SHADER_VISIBILITY_PIXEL;

				// DeltaE
				SamplerDesc[SRV_DeltaE].Filter						= D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
				SamplerDesc[SRV_DeltaE].AddressU					= D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				SamplerDesc[SRV_DeltaE].AddressV					= D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				SamplerDesc[SRV_DeltaE].AddressW					= D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				SamplerDesc[SRV_DeltaE].MipLODBias					= 0;
				SamplerDesc[SRV_DeltaE].MaxAnisotropy				= 1;
				SamplerDesc[SRV_DeltaE].ComparisonFunc				= D3D12_COMPARISON_FUNC_LESS_EQUAL;
				SamplerDesc[SRV_DeltaE].BorderColor					= D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
				SamplerDesc[SRV_DeltaE].MinLOD						= 0.0f;
				SamplerDesc[SRV_DeltaE].MaxLOD						= D3D12_FLOAT32_MAX;
				SamplerDesc[SRV_DeltaE].ShaderRegister				= SRV_DeltaE;
				SamplerDesc[SRV_DeltaE].RegisterSpace				= 0;
				SamplerDesc[SRV_DeltaE].ShaderVisibility			= D3D12_SHADER_VISIBILITY_PIXEL;

				// DeltaSR
				SamplerDesc[SRV_DeltaSR].Filter						= D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
				SamplerDesc[SRV_DeltaSR].AddressU					= D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				SamplerDesc[SRV_DeltaSR].AddressV					= D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				SamplerDesc[SRV_DeltaSR].AddressW					= D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				SamplerDesc[SRV_DeltaSR].MipLODBias					= 0;
				SamplerDesc[SRV_DeltaSR].MaxAnisotropy				= 1;
				SamplerDesc[SRV_DeltaSR].ComparisonFunc				= D3D12_COMPARISON_FUNC_LESS_EQUAL;
				SamplerDesc[SRV_DeltaSR].BorderColor				= D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
				SamplerDesc[SRV_DeltaSR].MinLOD						= 0.0f;
				SamplerDesc[SRV_DeltaSR].MaxLOD						= D3D12_FLOAT32_MAX;
				SamplerDesc[SRV_DeltaSR].ShaderRegister				= SRV_DeltaSR;
				SamplerDesc[SRV_DeltaSR].RegisterSpace				= 0;
				SamplerDesc[SRV_DeltaSR].ShaderVisibility			= D3D12_SHADER_VISIBILITY_PIXEL;

				// DeltaSM
				SamplerDesc[SRV_DeltaSM].Filter						= D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
				SamplerDesc[SRV_DeltaSM].AddressU					= D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				SamplerDesc[SRV_DeltaSM].AddressV					= D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				SamplerDesc[SRV_DeltaSM].AddressW					= D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				SamplerDesc[SRV_DeltaSM].MipLODBias					= 0;
				SamplerDesc[SRV_DeltaSM].MaxAnisotropy				= 1;
				SamplerDesc[SRV_DeltaSM].ComparisonFunc				= D3D12_COMPARISON_FUNC_LESS_EQUAL;
				SamplerDesc[SRV_DeltaSM].BorderColor				= D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
				SamplerDesc[SRV_DeltaSM].MinLOD						= 0.0f;
				SamplerDesc[SRV_DeltaSM].MaxLOD						= D3D12_FLOAT32_MAX;
				SamplerDesc[SRV_DeltaSM].ShaderRegister				= SRV_DeltaSM;
				SamplerDesc[SRV_DeltaSM].RegisterSpace				= 0;
				SamplerDesc[SRV_DeltaSM].ShaderVisibility			= D3D12_SHADER_VISIBILITY_PIXEL;

				// DeltaJ
				SamplerDesc[SRV_DeltaJ].Filter						= D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
				SamplerDesc[SRV_DeltaJ].AddressU					= D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				SamplerDesc[SRV_DeltaJ].AddressV					= D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				SamplerDesc[SRV_DeltaJ].AddressW					= D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				SamplerDesc[SRV_DeltaJ].MipLODBias					= 0;
				SamplerDesc[SRV_DeltaJ].MaxAnisotropy				= 1;
				SamplerDesc[SRV_DeltaJ].ComparisonFunc				= D3D12_COMPARISON_FUNC_LESS_EQUAL;
				SamplerDesc[SRV_DeltaJ].BorderColor					= D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
				SamplerDesc[SRV_DeltaJ].MinLOD						= 0.0f;
				SamplerDesc[SRV_DeltaJ].MaxLOD						= D3D12_FLOAT32_MAX;
				SamplerDesc[SRV_DeltaJ].ShaderRegister				= SRV_DeltaJ;
				SamplerDesc[SRV_DeltaJ].RegisterSpace				= 0;
				SamplerDesc[SRV_DeltaJ].ShaderVisibility			= D3D12_SHADER_VISIBILITY_PIXEL;

				// Bilinear
				SamplerDesc[SRV_DeltaJ + 1] = SamplerStates::BilinearSampler
				(
					7,
					D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
					D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
					D3D12_TEXTURE_ADDRESS_MODE_CLAMP
				);

				CD3DX12_DESCRIPTOR_RANGE DescriptorRange[4] = {};
				{
					DescriptorRange[0].Init
					(
						D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
						NumCBV,
						0,
						0
					);

					DescriptorRange[1].Init
					(
						D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
						NumSRV,
						0,
						0,
						NumCBV
					);

					DescriptorRange[2].Init
					(
						D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
						2,
						NumCBV,
						0,
						0
					);
					
					DescriptorRange[3].Init
					(
						D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
						1,
						NumSRV,
						0,
						SRV_DEPTH
					);
				}

				CD3DX12_ROOT_PARAMETER RootParameter[8] = {};
				{
					RootParameter[0].InitAsDescriptorTable
					(
						1,
						&DescriptorRange[0],
						D3D12_SHADER_VISIBILITY_ALL
					);

					RootParameter[1].InitAsDescriptorTable
					(
						1,
						&DescriptorRange[1],
						D3D12_SHADER_VISIBILITY_PIXEL
					);

					RootParameter[2].InitAsDescriptorTable
					(
						1,
						&DescriptorRange[2],
						D3D12_SHADER_VISIBILITY_ALL
					);

					RootParameter[3].InitAsDescriptorTable
					(
						1,
						&DescriptorRange[3],
						D3D12_SHADER_VISIBILITY_PIXEL
					);

					RootParameter[4].InitAsConstants(4, 3);
					RootParameter[5].InitAsConstants(1, 4);
					RootParameter[6].InitAsConstants(1, 5);
					RootParameter[7].InitAsConstants(1, 6);
				}

				DefaultRootSignature = new RRootSignature();

				if ((Error = DefaultRootSignature->Create(RRootSignature::InitializeOptions(
					_countof(RootParameter),
					RootParameter,
					_countof(SamplerDesc),
					SamplerDesc))))
				{
					return Error;
				}

				return S_OK;
			}

			ErrorCode InitializePipelines()
			{
				if (DefaultRootSignature)
				{
					return S_OK;
				}

				Shaders[eTransmittancePS]		= new CGrpShader(PipelineTransmittance::New());
				Shaders[eIrradiancePS]			= new CGrpShader(PipelineIrradiance::New());
				Shaders[eInscatterPS]			= new CGrpShader(PipelineInscatter::New());
				Shaders[eInscatterCopyPS]		= new CGrpShader(PipelineInscatterCopy::New());
				Shaders[eInscatterCopyFPS]		= new CGrpShader(PipelineInscatterCopyF::New());
				Shaders[eInscatterCopyFBackPS]	= new CGrpShader(PipelineInscatterCopyFBack::New());
				Shaders[eInscatterCopyNPS]		= new CGrpShader(PipelineInscatterCopyN::New());
				Shaders[eInscatterSPS]			= new CGrpShader(PipelineInscatterS::New());
				Shaders[eInscatterNPS]			= new CGrpShader(PipelineInscatterN::New());
				Shaders[eIrradianceNPS]			= new CGrpShader(PipelineIrradianceN::New());
				Shaders[eIrradianceCopyPS]		= new CGrpShader(PipelineIrradianceCopy::New());

				ErrorCode Error;

				if ((Error = InitializeRootSignatures()))
				{
					return Error;
				}

				if ((Error = PipelineTransmittance::Instance().Initialize()))
				{
					return Error;
				}

				if ((Error = PipelineIrradiance::Instance().Initialize()))
				{
					return Error;
				}

				if ((Error = PipelineIrradianceN::Instance().Initialize()))
				{
					return Error;
				}

				if ((Error = PipelineInscatter::Instance().Initialize()))
				{
					return Error;
				}

				if ((Error = PipelineInscatterCopy::Instance().Initialize()))
				{
					return Error;
				}

				if ((Error = PipelineInscatterCopyF::Instance().Initialize()))
				{
					return Error;
				}

				if ((Error = PipelineInscatterCopyFBack::Instance().Initialize()))
				{
					return Error;
				}

				if ((Error = PipelineInscatterCopyN::Instance().Initialize()))
				{
					return Error;
				}

				if ((Error = PipelineIrradianceCopy::Instance().Initialize()))
				{
					return Error;
				}

				if ((Error = PipelineInscatterS::Instance().Initialize()))
				{
					return Error;
				}

				if ((Error = PipelineInscatterN::Instance().Initialize()))
				{
					return Error;
				}

				PipelineFog::New();

				if ((Error = PipelineFog::Instance().Initialize()))
				{
					return Error;
				}

				return S_OK;
			}

			ErrorCode PipelineTransmittance::CreateRootSignature()
			{
				return (this->RootSignature = DefaultRootSignature) == NULL ? E_FAIL : S_OK;
			}

			ErrorCode PipelineTransmittance::CreatePipelineState()
			{
				if (!Pipelines[eTransmittancePS])
				{
					ErrorCode Error = CreatePipeline(eTransmittancePS);

					if (Error)
					{
						return Error;
					}
				}

				PipelineState = Pipelines[eTransmittancePS];

				return S_OK;
			}

			ErrorCode PipelineIrradiance::CreateRootSignature()
			{
				return (this->RootSignature = DefaultRootSignature) == NULL ? E_FAIL : S_OK;
			}

			ErrorCode PipelineIrradiance::CreatePipelineState()
			{
				if (!Pipelines[eIrradiancePS])
				{
					ErrorCode Error = CreatePipeline(eIrradiancePS);

					if (Error)
					{
						return Error;
					}
				}

				PipelineState = Pipelines[eIrradiancePS];

				return S_OK;
			}

			ErrorCode PipelineIrradianceN::CreateRootSignature()
			{
				return (this->RootSignature = DefaultRootSignature) == NULL ? E_FAIL : S_OK;
			}

			ErrorCode PipelineIrradianceN::CreatePipelineState()
			{
				if (!Pipelines[eIrradianceNPS])
				{
					ErrorCode Error = CreatePipeline(eIrradianceNPS);

					if (Error)
					{
						return Error;
					}
				}

				PipelineState = Pipelines[eIrradianceNPS];

				return S_OK;
			}

			ErrorCode PipelineIrradianceCopy::CreateRootSignature()
			{
				return (this->RootSignature = DefaultRootSignature) == NULL ? E_FAIL : S_OK;
			}

			ErrorCode PipelineIrradianceCopy::CreatePipelineState()
			{
				if (!Pipelines[eIrradianceCopyPS])
				{
					ErrorCode Error = CreatePipeline(eIrradianceCopyPS);

					if (Error)
					{
						return Error;
					}
				}

				PipelineState = Pipelines[eIrradianceCopyPS];

				return S_OK;
			}

			ErrorCode PipelineInscatter::CreateRootSignature()
			{
				return (this->RootSignature = DefaultRootSignature) == NULL ? E_FAIL : S_OK;
			}

			ErrorCode PipelineInscatter::CreatePipelineState()
			{
				if (!Pipelines[eInscatterPS])
				{
					ErrorCode Error = CreatePipeline(eInscatterPS);

					if (Error)
					{
						return Error;
					}
				}

				PipelineState = Pipelines[eInscatterPS];

				return S_OK;
			}

			ErrorCode PipelineInscatterCopy::CreateRootSignature()
			{
				return (this->RootSignature = DefaultRootSignature) == NULL ? E_FAIL : S_OK;
			}

			ErrorCode PipelineInscatterCopy::CreatePipelineState()
			{
				if(!Pipelines[eInscatterCopyPS])
				{
					ErrorCode Error = CreatePipeline(eInscatterCopyPS);

					if (Error)
					{
						return Error;
					}
				}

				PipelineState = Pipelines[eInscatterCopyPS];

				return S_OK;
			}

			ErrorCode PipelineInscatterCopyN::CreateRootSignature()
			{
				return (this->RootSignature = DefaultRootSignature) == NULL ? E_FAIL : S_OK;
			}

			ErrorCode PipelineInscatterCopyN::CreatePipelineState()
			{
				if (!Pipelines[eInscatterCopyNPS])
				{
					ErrorCode Error = CreatePipeline(eInscatterCopyNPS);

					if (Error)
					{
						return Error;
					}
				}

				PipelineState = Pipelines[eInscatterCopyNPS];

				return S_OK;
			}

			ErrorCode PipelineInscatterS::CreateRootSignature()
			{
				return (this->RootSignature = DefaultRootSignature) == NULL ? E_FAIL : S_OK;
			}

			ErrorCode PipelineInscatterS::CreatePipelineState()
			{
				if (!Pipelines[eInscatterSPS])
				{
					ErrorCode Error = CreatePipeline(eInscatterSPS);

					if (Error)
					{
						return Error;
					}
				}

				PipelineState = Pipelines[eInscatterSPS];

				return S_OK;
			}

			ErrorCode PipelineInscatterN::CreateRootSignature()
			{
				return (this->RootSignature = DefaultRootSignature) == NULL ? E_FAIL : S_OK;
			}

			ErrorCode PipelineInscatterN::CreatePipelineState()
			{
				if (!Pipelines[eInscatterNPS])
				{
					ErrorCode Error = CreatePipeline(eInscatterNPS);

					if (Error)
					{
						return Error;
					}
				}

				PipelineState = Pipelines[eInscatterNPS];

				return S_OK;
			}

			ErrorCode PipelineInscatterCopyF::CreateRootSignature()
			{
				return (this->RootSignature = DefaultRootSignature) == NULL ? E_FAIL : S_OK;
			}

			ErrorCode PipelineInscatterCopyF::CreatePipelineState()
			{
				if (!Pipelines[eInscatterCopyFPS])
				{
					ErrorCode Error = CreatePipeline(eInscatterCopyFPS);

					if (Error)
					{
						return Error;
					}
				}

				PipelineState = Pipelines[eInscatterCopyFPS];

				return S_OK;
			}

			ErrorCode PipelineInscatterCopyFBack::CreateRootSignature()
			{
				return (this->RootSignature = DefaultRootSignature) == NULL ? E_FAIL : S_OK;
			}

			ErrorCode PipelineInscatterCopyFBack::CreatePipelineState()
			{
				if (!Pipelines[eInscatterCopyFBackPS])
				{
					ErrorCode Error = CreatePipeline(eInscatterCopyFBackPS);

					if (Error)
					{
						return Error;
					}
				}

				PipelineState = Pipelines[eInscatterCopyFBackPS];

				return S_OK;
			}

			ErrorCode PipelineFog::CreateRootSignature()
			{
				return (this->RootSignature = DefaultRootSignature) == NULL ? E_FAIL : S_OK;
			}

			ErrorCode PipelineFog::CreatePipelineState()
			{
				ErrorCode Error;

				static SharedPointer<CGrpShader> Shaders = new CGrpShader(this);
				
				if ((Error = Shaders->CompileShaderType(CGrpShader::Pixel, L"Atmosphere.hlsl", "AtmosphericPixelMain", "ps_5_0")))
				{
					return Error;
				}

				if ((Error = Shaders->CompileShaderType(CGrpShader::Vertex, L"Atmosphere.hlsl", "VSMain", "vs_5_0")))
				{
					return Error;
				}

				PipelineState = new RPipelineState(RootSignature);
				{
					D3D12_BLEND_DESC BlendDesc = BlendState::Additive;
					D3D12_RASTERIZER_DESC RasterizerDesc = RasterizerState::CullNone;
					D3D12_DEPTH_STENCIL_DESC DepthStencilDesc = DepthStencilState::None;
					D3D12_INPUT_ELEMENT_DESC InputElementDesc = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
					D3D12_INPUT_LAYOUT_DESC InputLayoutDesc = CD3DX12_INPUT_LAYOUT_DESC(0, NULL);

					if ((Error = PipelineState->Create(new RPipelineState::InitializeOptionsGraphics(
						Shaders,
						InputLayoutDesc,
						BlendDesc,
						RasterizerDesc,
						D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
						CScene::ColorSceneFormat))))
					{
						return Error;
					}
				}

				return S_OK;
			}
		}
	}
}
