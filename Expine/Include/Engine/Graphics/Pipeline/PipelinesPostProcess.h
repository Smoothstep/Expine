#pragma once

#include "RawCommandList.h"
#include "RawPipelineState.h"
#include "ConstantBuffer.h"

namespace D3D
{
	namespace Pipelines
	{
		namespace PostProcess
		{
			namespace Bloom
			{
				class PipelineSetup : public CSingleton<PipelineSetup>, public PipelineObject<PipelineSetup>
				{
				public:

					enum EConstants
					{
						CBV_BloomPostProcess,
						CBV_BloomThreshold,
						NumCBV
					};

					enum ETextures
					{
						SRV_BloomColor,
						SRV_BloomOverlay,
						SRV_BloomCombined,
						NumSRV
					};

					enum ERootParameters
					{
						BloomPostProcess,
						BloomThreshold,
						BloomInput,
						NumBloomSetupParameter
					};

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
					}
				};

				class PipelineOverlay : public CSingleton<PipelineOverlay>, public PipelineObject<PipelineOverlay>
				{
				public:

					enum EConstants
					{
						CBV_BloomPostProcess,
						CBV_ColorScale,
						NumCBV
					};

					enum ETextures
					{
						SRV_BloomColor,
						SRV_BloomOverlay,
						SRV_BloomCombined,
						NumSRV
					};

					enum ERootParameters
					{
						BloomPostProcess,
						BloomColorScale,
						BloomInput,
						NumBloomSetupParameter
					};

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
					}
				};
			}

			extern ErrorCode InitializePipelines();
		}
	}
}