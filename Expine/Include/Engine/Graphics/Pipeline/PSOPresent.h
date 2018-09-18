#pragma once

#include "Raw/RawCommandList.h"
#include "Raw/RawPipelineState.h"

#include "Buffer/BufferConstant.h"

namespace D3D
{
	namespace Pipelines
	{
		class PipelinePresent : public CSingleton<PipelinePresent>, public PipelineObject<PipelinePresent>
		{
		public:

			enum ERootParameters
			{
				SceneComposite
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