#pragma once

#include "ShaderGroup.h"
#include "RawRootSignature.h"

namespace D3D
{
	enum ShaderType
	{
		Graphics,
		Compute
	};

	class RPipelineState
	{
		static constexpr DXGI_FORMAT DefaultFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	public:

		struct InitializeOptions 
		{
			SharedPointer<CGrpShader> ShaderGroup;

			inline virtual ShaderType Type() = 0;
		};

		struct InitializeOptionsCompute : public InitializeOptions, public D3D12_COMPUTE_PIPELINE_STATE_DESC
		{
			inline virtual ShaderType Type()
			{
				return Compute;
			}

			inline void SetShader
			(
				const SharedPointer<CGrpShader> & pShader
			)
			{
				ShaderGroup = pShader;

				if(pShader)
				{
					RShader * pComputeShader = pShader->GetShader(CGrpShader::Compute);
					
					if (pComputeShader)
					{
						CS = CD3DX12_SHADER_BYTECODE(pComputeShader->GetByteCode());
					}
					else
					{
						CS.BytecodeLength = 0; CS.pShaderBytecode = 0;
					}
				}
			}

			inline explicit InitializeOptionsCompute
			(
				const SharedPointer<CGrpShader> & pShader
			)
			{
				NodeMask = 0;
				Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

				CachedPSO.CachedBlobSizeInBytes = 0;
				CachedPSO.pCachedBlob = 0;

				SetShader(pShader);
			}
		};

		struct InitializeOptionsGraphics : public InitializeOptions, public D3D12_GRAPHICS_PIPELINE_STATE_DESC
		{
			inline virtual ShaderType Type()
			{
				return Graphics;
			}

			inline void operator=(const InitializeOptionsGraphics& Other)
			{
				CopyMemory(this, &Other, 1);
				{
					ShaderGroup = Other.ShaderGroup;
				}
			}

			inline void SetShader
			(
				const SharedPointer<CGrpShader> & pShader
			)
			{
				ShaderGroup = pShader;

				if(pShader)
				{
					RShader * pPixelShader		= pShader->GetShader(CGrpShader::Pixel);
					RShader * pVertexShader		= pShader->GetShader(CGrpShader::Vertex);
					RShader * pHullShader		= pShader->GetShader(CGrpShader::Hull);
					RShader * pGeometryShader	= pShader->GetShader(CGrpShader::Geometry);
					RShader * pDomainShader		= pShader->GetShader(CGrpShader::Domain);

					if (pPixelShader)
					{
						PS = CD3DX12_SHADER_BYTECODE(pPixelShader->GetByteCode());
					}
					else
					{
						PS.BytecodeLength = 0; PS.pShaderBytecode = 0;
					}

					if (pVertexShader)
					{
						VS = CD3DX12_SHADER_BYTECODE(pVertexShader->GetByteCode());
					}
					else
					{
						VS.BytecodeLength = 0; VS.pShaderBytecode = 0;
					}

					if (pGeometryShader)
					{
						GS = CD3DX12_SHADER_BYTECODE(pGeometryShader->GetByteCode());
					}
					else
					{
						GS.BytecodeLength = 0; GS.pShaderBytecode = 0;
					}

					if (pHullShader)
					{
						HS = CD3DX12_SHADER_BYTECODE(pHullShader->GetByteCode());
					}
					else
					{
						HS.BytecodeLength = 0; HS.pShaderBytecode = 0;
					}

					if (pDomainShader)
					{
						DS = CD3DX12_SHADER_BYTECODE(pDomainShader->GetByteCode());
					}
					else
					{
						DS.BytecodeLength = 0; DS.pShaderBytecode = 0;
					}
				}
			}

			inline explicit InitializeOptionsGraphics()
			{
				BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
				RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
				
				InputLayout.NumElements = 0;
				InputLayout.pInputElementDescs = 0;

				DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC
				(
					FALSE,
					D3D12_DEPTH_WRITE_MASK_ALL,
					D3D12_COMPARISON_FUNC_LESS,
					FALSE,
					D3D12_DEFAULT_STENCIL_READ_MASK,
					D3D12_DEFAULT_STENCIL_WRITE_MASK,
					D3D12_STENCIL_OP_KEEP,
					D3D12_STENCIL_OP_KEEP,
					D3D12_STENCIL_OP_KEEP,
					D3D12_COMPARISON_FUNC_ALWAYS,
					D3D12_STENCIL_OP_KEEP,
					D3D12_STENCIL_OP_KEEP,
					D3D12_STENCIL_OP_KEEP,
					D3D12_COMPARISON_FUNC_ALWAYS
				);

				PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

				if (DefaultFormat != DXGI_FORMAT_UNKNOWN)
				{
					NumRenderTargets = 1;
				}
				else
				{
					NumRenderTargets = 0;
				}

				RTVFormats[0] = DefaultFormat;
				RTVFormats[1] = DXGI_FORMAT_UNKNOWN;
				RTVFormats[2] = DXGI_FORMAT_UNKNOWN;
				RTVFormats[3] = DXGI_FORMAT_UNKNOWN;
				RTVFormats[4] = DXGI_FORMAT_UNKNOWN;
				RTVFormats[5] = DXGI_FORMAT_UNKNOWN;
				RTVFormats[6] = DXGI_FORMAT_UNKNOWN;
				RTVFormats[7] = DXGI_FORMAT_UNKNOWN;

				DSVFormat = DXGI_FORMAT_UNKNOWN;

				NodeMask						= 0;
				SampleMask						= D3D12_DEFAULT_SAMPLE_MASK;
				Flags							= D3D12_PIPELINE_STATE_FLAG_NONE;
				IBStripCutValue					= D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

				StreamOutput.NumEntries			= 0;
				StreamOutput.NumStrides			= 0;
				StreamOutput.pBufferStrides		= 0;
				StreamOutput.pSODeclaration		= 0;
				StreamOutput.RasterizedStream	= 0;

				SampleDesc.Count				= 1;
				SampleDesc.Quality				= 0;

				CachedPSO.pCachedBlob			= 0;
				CachedPSO.CachedBlobSizeInBytes = 0;
			}

			inline explicit InitializeOptionsGraphics
			(
				const	SharedPointer<CGrpShader>		& pShader,
				const	D3D12_INPUT_LAYOUT_DESC			& InputLayoutDesc,
				const	D3D12_BLEND_DESC				& BlendDesc,
				const	D3D12_RASTERIZER_DESC			& RasterizerDesc,
						D3D12_PRIMITIVE_TOPOLOGY_TYPE	PrimitiveTopology,
						DXGI_FORMAT						RTVFormat
			)
			{
				BlendState			= BlendDesc;
				InputLayout			= InputLayoutDesc;
				RasterizerState		= RasterizerDesc;

				DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC
				(
					FALSE,
					D3D12_DEPTH_WRITE_MASK_ALL,
					D3D12_COMPARISON_FUNC_LESS,
					FALSE,
					D3D12_DEFAULT_STENCIL_READ_MASK,
					D3D12_DEFAULT_STENCIL_WRITE_MASK,
					D3D12_STENCIL_OP_KEEP,
					D3D12_STENCIL_OP_KEEP,
					D3D12_STENCIL_OP_KEEP,
					D3D12_COMPARISON_FUNC_ALWAYS,
					D3D12_STENCIL_OP_KEEP,
					D3D12_STENCIL_OP_KEEP,
					D3D12_STENCIL_OP_KEEP,
					D3D12_COMPARISON_FUNC_ALWAYS
				);

				PrimitiveTopologyType = PrimitiveTopology;

				if (RTVFormat != DXGI_FORMAT_UNKNOWN)
				{
					NumRenderTargets = 1;
				}
				else
				{
					NumRenderTargets = 0;
				}

				RTVFormats[0] = RTVFormat;
				RTVFormats[1] = DXGI_FORMAT_UNKNOWN;
				RTVFormats[2] = DXGI_FORMAT_UNKNOWN;
				RTVFormats[3] = DXGI_FORMAT_UNKNOWN;
				RTVFormats[4] = DXGI_FORMAT_UNKNOWN;
				RTVFormats[5] = DXGI_FORMAT_UNKNOWN;
				RTVFormats[6] = DXGI_FORMAT_UNKNOWN;
				RTVFormats[7] = DXGI_FORMAT_UNKNOWN;

				DSVFormat = DXGI_FORMAT_UNKNOWN;

				NodeMask						= 0;
				SampleMask						= D3D12_DEFAULT_SAMPLE_MASK;
				Flags							= D3D12_PIPELINE_STATE_FLAG_NONE;
				IBStripCutValue					= D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

				StreamOutput.NumEntries			= 0;
				StreamOutput.NumStrides			= 0;
				StreamOutput.pBufferStrides		= 0;
				StreamOutput.pSODeclaration		= 0;
				StreamOutput.RasterizedStream	= pShader->GetShader(CGrpShader::Pixel) == NULL ? D3D12_SO_NO_RASTERIZED_STREAM : 0;

				SampleDesc.Count				= 1;
				SampleDesc.Quality				= 0;

				CachedPSO.pCachedBlob			= 0;
				CachedPSO.CachedBlobSizeInBytes = 0;

				SetShader(pShader);
			}

			inline explicit InitializeOptionsGraphics
			(
				const	SharedPointer<CGrpShader>		& pShader,
				const	D3D12_INPUT_LAYOUT_DESC			& InputLayoutDesc,
				const	D3D12_BLEND_DESC				& BlendDesc,
				const	D3D12_RASTERIZER_DESC			& RasterizerDesc,
				const	D3D12_DEPTH_STENCIL_DESC		& DepthStencilDesc,
				const	D3D12_PRIMITIVE_TOPOLOGY_TYPE	PrimitiveTopology,
				const	DXGI_FORMAT						RTVFormat,
				const	DXGI_FORMAT						FormatDSV
			)
			{
				BlendState				= BlendDesc;
				InputLayout				= InputLayoutDesc;
				RasterizerState			= RasterizerDesc;
				DepthStencilState		= DepthStencilDesc;
				PrimitiveTopologyType	= PrimitiveTopology;

				if (RTVFormat != DXGI_FORMAT_UNKNOWN)
				{
					NumRenderTargets = 1;
				}
				else
				{
					NumRenderTargets = 0;
				}

				RTVFormats[0] = RTVFormat;
				RTVFormats[1] = DXGI_FORMAT_UNKNOWN;
				RTVFormats[2] = DXGI_FORMAT_UNKNOWN;
				RTVFormats[3] = DXGI_FORMAT_UNKNOWN;
				RTVFormats[4] = DXGI_FORMAT_UNKNOWN;
				RTVFormats[5] = DXGI_FORMAT_UNKNOWN;
				RTVFormats[6] = DXGI_FORMAT_UNKNOWN;
				RTVFormats[7] = DXGI_FORMAT_UNKNOWN;

				DSVFormat = FormatDSV;

				NodeMask						= 0;
				SampleMask						= D3D12_DEFAULT_SAMPLE_MASK;
				Flags							= D3D12_PIPELINE_STATE_FLAG_NONE;
				IBStripCutValue					= D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

				StreamOutput.NumEntries			= 0;
				StreamOutput.NumStrides			= 0;
				StreamOutput.pBufferStrides		= 0;
				StreamOutput.pSODeclaration		= 0;
				StreamOutput.RasterizedStream	= pShader->GetShader(CGrpShader::Pixel) == NULL ? D3D12_SO_NO_RASTERIZED_STREAM : 0;

				SampleDesc.Count				= 1;
				SampleDesc.Quality				= 0;

				CachedPSO.pCachedBlob			= 0;
				CachedPSO.CachedBlobSizeInBytes = 0;

				SetShader(pShader);
			}

			inline explicit InitializeOptionsGraphics
			(
				const	SharedPointer<CGrpShader>		& pShader,
				const	D3D12_INPUT_LAYOUT_DESC			& InputLayoutDesc,
				const	D3D12_BLEND_DESC				& BlendDesc,
				const	D3D12_RASTERIZER_DESC			& RasterizerDesc,
				const	D3D12_PRIMITIVE_TOPOLOGY_TYPE	PrimitiveTopology,
				const	UINT							RenderTargetCount,
				const	DXGI_FORMAT						* pRTVFormats
			)
			{
				Ensure
				(
					NumRenderTargets <= 8 &&
					NumRenderTargets >= 1
				);

				BlendState				= BlendDesc;
				InputLayout				= InputLayoutDesc;
				RasterizerState			= RasterizerDesc;
				NumRenderTargets		= RenderTargetCount;

				DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC
				(
					FALSE,
					D3D12_DEPTH_WRITE_MASK_ALL,
					D3D12_COMPARISON_FUNC_LESS,
					FALSE,
					D3D12_DEFAULT_STENCIL_READ_MASK,
					D3D12_DEFAULT_STENCIL_WRITE_MASK,
					D3D12_STENCIL_OP_KEEP,
					D3D12_STENCIL_OP_KEEP,
					D3D12_STENCIL_OP_KEEP,
					D3D12_COMPARISON_FUNC_ALWAYS,
					D3D12_STENCIL_OP_KEEP,
					D3D12_STENCIL_OP_KEEP,
					D3D12_STENCIL_OP_KEEP,
					D3D12_COMPARISON_FUNC_ALWAYS
				);

				PrimitiveTopologyType	= PrimitiveTopology;

				for (UINT N = 0; N < NumRenderTargets; ++N)
				{
					RTVFormats[N] = pRTVFormats[N];
				}

				for (UINT N = NumRenderTargets; N < 8; ++N)
				{
					RTVFormats[N] = DXGI_FORMAT_UNKNOWN;
				}

				DSVFormat = DXGI_FORMAT_UNKNOWN;

				NodeMask						= 0;
				SampleMask						= D3D12_DEFAULT_SAMPLE_MASK;
				Flags							= D3D12_PIPELINE_STATE_FLAG_NONE;
				IBStripCutValue					= D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

				StreamOutput.NumEntries			= 0;
				StreamOutput.NumStrides			= 0;
				StreamOutput.pBufferStrides		= 0;
				StreamOutput.pSODeclaration		= 0;
				StreamOutput.RasterizedStream	= pShader->GetShader(CGrpShader::Pixel) == NULL ? D3D12_SO_NO_RASTERIZED_STREAM : 0;

				SampleDesc.Count				= 1;
				SampleDesc.Quality				= 0;

				CachedPSO.pCachedBlob			= 0;
				CachedPSO.CachedBlobSizeInBytes = 0;

				SetShader(pShader);
			}

			inline explicit InitializeOptionsGraphics
			(
				const	SharedPointer<CGrpShader>		& pShader,
				const	D3D12_INPUT_LAYOUT_DESC			& InputLayoutDesc,
				const	D3D12_BLEND_DESC				& BlendDesc,
				const	D3D12_RASTERIZER_DESC			& RasterizerDesc,
				const	D3D12_DEPTH_STENCIL_DESC		& DepthStencilDesc,
				const	D3D12_PRIMITIVE_TOPOLOGY_TYPE	PrimitiveTopology,
				const	UINT							RenderTargetCount,
				const	DXGI_FORMAT						* pRTVFormats,
				const	DXGI_FORMAT						FormatDSV
			)
			{
				Ensure
				(
					NumRenderTargets <= 8 &&
					NumRenderTargets >= 1
				);

				BlendState				= BlendDesc;
				InputLayout				= InputLayoutDesc;
				RasterizerState			= RasterizerDesc;
				NumRenderTargets		= RenderTargetCount;
				DepthStencilState		= DepthStencilDesc;
				PrimitiveTopologyType	= PrimitiveTopology;

				for (UINT N = 0; N < NumRenderTargets; ++N)
				{
					RTVFormats[N] = pRTVFormats[N];
				}

				for (UINT N = NumRenderTargets; N < 8; ++N)
				{
					RTVFormats[N] = DXGI_FORMAT_UNKNOWN;
				}

				DSVFormat = FormatDSV;

				NodeMask						= 0;
				SampleMask						= D3D12_DEFAULT_SAMPLE_MASK;
				Flags							= D3D12_PIPELINE_STATE_FLAG_NONE;
				IBStripCutValue					= D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

				StreamOutput.NumEntries			= 0;
				StreamOutput.NumStrides			= 0;
				StreamOutput.pBufferStrides		= 0;
				StreamOutput.pSODeclaration		= 0;
				StreamOutput.RasterizedStream	= pShader->GetShader(CGrpShader::Pixel) == NULL ? D3D12_SO_NO_RASTERIZED_STREAM : 0;

				SampleDesc.Count				= 1;
				SampleDesc.Quality				= 0;

				CachedPSO.pCachedBlob			= 0;
				CachedPSO.CachedBlobSizeInBytes = 0;

				SetShader(pShader);
			}
		};

	private:

		SharedPointer<RRootSignature>		RootSignature;
		SharedPointer<CGrpShader>			ShaderGroup;
		UniquePointer<InitializeOptions>	InitOptions;
		ComPointer<IPipelineState>			PipelineState;

	public:

		inline operator IPipelineState *() const
		{
			return PipelineState.Get();
		}

		inline const RRootSignature & GetRootSignature() const
		{
			return RootSignature.GetRef();
		}

		inline const CGrpShader * GetShaderGroup() const
		{
			return ShaderGroup.Get();
		}

	public:

		RPipelineState
		(
			const SharedPointer<RRootSignature> & RootSignature
		);

		ErrorCode Create
		(
			InitializeOptions * Options
		);

		ErrorCode Create
		(
			InitializeOptionsCompute * Options
		);

		ErrorCode Create
		(
			InitializeOptionsGraphics * Options
		);

		ErrorCode ReCreate
		(
			SharedPointer<CGrpShader> & Shaders
		);
	};
}