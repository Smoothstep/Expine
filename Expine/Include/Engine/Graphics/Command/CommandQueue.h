#pragma once

#include "RawCommandQueue.h"

namespace D3D
{
	class CCommandQueueDirect : public CSingleton<CCommandQueueDirect>, public RCommandQueue
	{
	public:

		ErrorCode Create()
		{
			ErrorCode Error = RCommandQueue::Create(D3D12_COMMAND_LIST_TYPE_DIRECT);

			if (Error)
			{
				return Error;
			}

			RCommandQueue::Get()->SetName(L"Command Queue Direct");

			return S_OK;
		}
	};

	class CCommandQueueCopy : public CSingleton<CCommandQueueCopy>, public RCommandQueue
	{
	public:

		ErrorCode Create()
		{
			ErrorCode Error = RCommandQueue::Create(D3D12_COMMAND_LIST_TYPE_COPY);

			if (Error)
			{
				return Error;
			}

			RCommandQueue::Get()->SetName(L"Command Queue Copy");

			return S_OK;
		}
	};

	class CCommandQueueCompute : public CSingleton<CCommandQueueCompute>, public RCommandQueue
	{
	public:

		ErrorCode Create()
		{
			ErrorCode Error = RCommandQueue::Create(D3D12_COMMAND_LIST_TYPE_COMPUTE);

			if (Error)
			{
				return Error;
			}

			RCommandQueue::Get()->SetName(L"Command Queue Compute");

			return S_OK;
		}
	};

	template
	<
		size_t NumThreads
	>
	inline void CCommandListGroup<NumThreads>::Finish(const UINT Frame, const BOOL WaitForCompletion)
	{
		if (CommandListContext[0].GetType() == D3D12_COMMAND_LIST_TYPE_DIRECT)
		{
			RCommandQueue & CmdQueue = CCommandQueueDirect::Instance();

			CmdQueue.ExecuteCommandLists(*this);

			UINT64 FenceValue = CmdQueue.IncrementFence(Frame);

			if (WaitForCompletion)
			{
				CmdQueue.WaitForFence(FenceValue);
			}
		}
	}
}