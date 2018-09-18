#pragma once

#include "Raw/RawCommandList.h"
#include "Raw/RawPipelineState.h"

#include "Buffer/BufferConstant.h"

namespace D3D
{
	namespace Pipelines
	{
		class PipelineComposite : public CSingleton<PipelineComposite>, public PipelineObject<PipelineComposite>
		{
		public:

			enum ERootParameters
			{
				SceneConstants,
				SceneResources
			};

			enum EDescriptors
			{
				BloomOverlay,
				Noise,
				LightingGGX,
				NumDescriptors
			};

		public:

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
}