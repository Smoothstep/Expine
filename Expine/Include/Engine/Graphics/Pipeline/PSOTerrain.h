#pragma once

#include "Raw/RawCommandList.h"
#include "Raw/RawPipelineState.h"
#include "Buffer/BufferConstant.h"

namespace D3D
{
	namespace Pipelines
	{
		namespace Terrain
		{
			enum EDescriptors
			{
				CBV_Transform,
				SRV_Height,
				SRV_Normal,
				SRV_TextureAtlas,
				SRV_TextureAtlasNormals,
				SRV_TextureAtlasParallax,
				SRV_Noise,
				SRV_ColorMap,
				SRV_NormalMap,
				NumDescriptors
			};

			class PipelineShadows : public CSingleton<PipelineShadows>, public PipelineObject<PipelineShadows>
			{
			private:

				virtual ErrorCode CreateRootSignature() override;
				virtual ErrorCode CreatePipelineState() override;

			public:

				virtual inline void Apply
				(
					RGrpCommandList * CmdList
				)	const
				{
					CmdList->ApplyPipelineState(PipelineState);
					CmdList->SetCommandSignature(CommandSignature);
				}

				virtual inline void SetConstantBuffer
				(
					const RGrpCommandList & CmdList,
					const CConstantBuffer & ConstantBuffer
				)	const
				{
					CmdList.SetConstantBuffer(CBV_Transform, ConstantBuffer);
				}
			};

			class PipelineGeometry : public CSingleton<PipelineGeometry>, public PipelineObject<PipelineGeometry>
			{
			private:

				virtual ErrorCode CreateRootSignature() override;
				virtual ErrorCode CreatePipelineState() override;

			public:

				virtual inline void Apply
				(
					RGrpCommandList * CmdList
				)	const
				{
					CmdList->ApplyPipelineState(PipelineState);
					CmdList->SetCommandSignature(CommandSignature);
				}

				virtual inline void SetConstantBuffer
				(
					const RGrpCommandList & CmdList,
					const CConstantBuffer & ConstantBuffer
				)	const
				{
					CmdList.SetConstantBuffer(CBV_Transform, ConstantBuffer);
				}
			};

			class PipelineComputeOcclusion : public CSingleton<PipelineComputeOcclusion>, public PipelineObject<PipelineComputeOcclusion>
			{
			public:
				enum ERootParameters
				{
					LightCBV,
					TerrainSRV,
					SizeConstant,
					TerrainPatch,
					OcclusionMapUAV,
					NumRootParameters
				};

			private:

				virtual ErrorCode CreateRootSignature() override;
				virtual ErrorCode CreatePipelineState() override;

			public:

				struct IndirectCommand
				{
					IntPoint TerrainPatch[2];
					D3D12_DISPATCH_ARGUMENTS DispatchArguments;
				};

				virtual inline void Apply
				(
					RGrpCommandList * CmdList
				)	const
				{
					CmdList->ApplyPipelineStateCompute(PipelineState);
					CmdList->SetCommandSignature(CommandSignature);
				}
			};
		}
	}
}