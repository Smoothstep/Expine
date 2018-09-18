#include "Precompiled.h"

#include "Raw/RawCommandQueue.h"

namespace D3D
{
	ErrorCode RCommandQueue::Create(const D3D12_COMMAND_LIST_TYPE Type)
	{
		D3D12_COMMAND_QUEUE_DESC CmdQueueDesc = {};
		{
			CmdQueueDesc.Type = Type;
		}

		ErrorCode Error = DEVICE->CreateCommandQueue(&CmdQueueDesc, IID_PPV_ARGS(&CommandQueue));
		
		if (Error)
		{
			return Error;
		}

		Fence = new RFence();

		if ((Error = Fence->Create(D3D12_FENCE_FLAG_NONE)))
		{
			return Error;
		}

		this->Type = Type;

		return S_OK;
	}

	RCommandQueue::RCommandQueue() :
		CompletionCBHandler(this)
	{}

	RCommandQueue::~RCommandQueue()
	{}
	
	RCommandQueue::CCompletionCallbackHandler::CCompletionCallbackHandler(RCommandQueue * Queue) :
		CommandQueue(Queue)
	{}

	void RCommandQueue::CCompletionCallbackHandler::OnCompletion(BOOLEAN B)
	{
		if (!B)
		{
			UINT64 Value = CommandQueue->Fence->RefreshCompletedValueSafe();

			for (CCompletionEventListener * Listener : EventListener)
			{
				Listener->OnComplete(Value);
			}
		}
	}
}
