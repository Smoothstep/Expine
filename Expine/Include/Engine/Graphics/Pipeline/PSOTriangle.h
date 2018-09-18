#pragma once

#include "Raw/RawCommandList.h"
#include "Raw/RawPipelineState.h"

#include "Buffer/BufferConstant.h"

namespace D3D
{
	namespace Pipelines
	{
		class PipelineColorTriangle : public CSingleton<PipelineColorTriangle>, public PipelineObject<PipelineColorTriangle>
		{
			enum ERootParameters
			{
				CBV_Transform
			};

		private:

			virtual ErrorCode CreateRootSignature() override;
			virtual ErrorCode CreatePipelineState() override;

		public:

			virtual ErrorCode Initialize();

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

		class PipelineWireframeTriangle : public CSingleton<PipelineWireframeTriangle>, public PipelineObject<PipelineWireframeTriangle>
		{
			enum ERootParameters
			{
				CBV_Transform
			};

		private:

			virtual ErrorCode CreateRootSignature() override;
			virtual ErrorCode CreatePipelineState() override;

		public:

			virtual ErrorCode Initialize();

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
	}
}