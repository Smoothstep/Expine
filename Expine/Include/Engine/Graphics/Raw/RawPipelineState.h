#pragma once

#include "Shader/ShaderGroup.h"
#include "Raw/RawShader.h"
#include "Raw/RawRootSignature.h"

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
			virtual ShaderType Type();

			void SetShader
			(
				const SharedPointer<CGrpShader> & pShader
			);

			explicit InitializeOptionsCompute
			(
				const SharedPointer<CGrpShader> & pShader
			);
		};

		struct InitializeOptionsGraphics : public InitializeOptions, public D3D12_GRAPHICS_PIPELINE_STATE_DESC
		{
			virtual ShaderType Type();

			void operator=
			(
				const InitializeOptionsGraphics& Other
			);

			void SetShader
			(
				const SharedPointer<CGrpShader> & pShader
			);

			explicit InitializeOptionsGraphics();
			explicit InitializeOptionsGraphics
			(
				const	SharedPointer<CGrpShader>		& pShader,
				const	D3D12_INPUT_LAYOUT_DESC			& InputLayoutDesc,
				const	D3D12_BLEND_DESC				& BlendDesc,
				const	D3D12_RASTERIZER_DESC			& RasterizerDesc,
						D3D12_PRIMITIVE_TOPOLOGY_TYPE	PrimitiveTopology,
						DXGI_FORMAT						RTVFormat
			);

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
			);

			inline explicit InitializeOptionsGraphics
			(
				const	SharedPointer<CGrpShader>		& pShader,
				const	D3D12_INPUT_LAYOUT_DESC			& InputLayoutDesc,
				const	D3D12_BLEND_DESC				& BlendDesc,
				const	D3D12_RASTERIZER_DESC			& RasterizerDesc,
				const	D3D12_PRIMITIVE_TOPOLOGY_TYPE	PrimitiveTopology,
				const	UINT							RenderTargetCount,
				const	DXGI_FORMAT						* pRTVFormats
			);

			explicit InitializeOptionsGraphics
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
			);
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