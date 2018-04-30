#pragma once

#include "RawFence.h"
#include "CommandList.h"

namespace D3D
{
	class RFence;
	class RCommandQueue
	{
	private:

		ComPointer<ICommandQueue> CommandQueue;

	protected:

		SharedPointer<RFence> Fence;

	protected:

		D3D12_COMMAND_LIST_TYPE Type;

		class CCompletionCallbackHandler
		{
		public:

			TSet<CCompletionEventListener*> EventListener;

		private:

			RCommandQueue * CommandQueue;

		public:

			CCompletionCallbackHandler
			(
				RCommandQueue * Queue
			);

			void __stdcall OnCompletion
			(
				BOOLEAN B
			);
		};

		CCompletionCallbackHandler CompletionCBHandler;

	public:

		inline void RemoveListener
		(
			CCompletionEventListener * Listener
		)
		{
			CompletionCBHandler.EventListener.erase(Listener);
		}

		inline void AddListener
		(
			CCompletionEventListener * Listener
		)
		{
			CompletionCBHandler.EventListener.insert(Listener);
		}

		inline operator ICommandQueue * () const
		{
			return CommandQueue.Get();
		}

		inline ICommandQueue * operator->() const
		{
			return CommandQueue.Get();
		}

		inline ICommandQueue * Get() const
		{
			return CommandQueue.Get();
		}

		inline D3D12_COMMAND_LIST_TYPE GetType() const
		{
			return Type;
		}

		inline const RFence & GetFence() const
		{
			return Fence.GetRef();
		}

	public:

		inline void WaitForCompletion
		(
			const UINT Frame
		)	const;

		inline void WaitForGPU
		(
			const UINT Frame
		)	const;

		inline void WaitForFence
		(
			const UINT64 Value
		)	const;

		inline void GotoNextFrame
		(
			const UINT		Frame,
			const UINT64	Value
		)	const;

		inline UINT64 SignalFence
		(	
			const UINT Frame
		)	const;

		inline UINT64 IncrementFence
		(
			const UINT Frame
		)	const;

		inline bool FenceCompleted
		(
			const UINT64 Value
		)	const;

		RCommandQueue();
		~RCommandQueue();

	protected:

		ErrorCode Create
		(
			const D3D12_COMMAND_LIST_TYPE Type
		);

	public:

		inline void ExecuteCommandList
		(
			const RGrpCommandList & CommandList
		)	const
		{
			ID3D12CommandList * CommandLists[] =
			{
				CommandList
			};

			CommandQueue->ExecuteCommandLists
			(
				1, 
				CommandLists
			);
		}

		inline void ExecuteCommandLists
		(
			const UINT				NumCommandLists,
			const RGrpCommandList * pCommandLists
		)	const
		{
			ID3D12CommandList ** CommandLists = new ID3D12CommandList*[NumCommandLists];
			{
				for (UINT N = 0; N < NumCommandLists; ++N)
				{
					CommandLists[N] = *pCommandLists;
				}
			}

			CommandQueue->ExecuteCommandLists
			(
				NumCommandLists, 
				CommandLists
			);
		}

		template
		<
			size_t NumThreads
		>
		inline void ExecuteCommandLists
		(
			const CCommandListGroup<NumThreads> & CommandListGroup
		)	const
		{
			CommandQueue->ExecuteCommandLists
			(
				NumThreads,
				CommandListGroup.Data()
			);
		}
	};

	inline UINT64 RFence::Signal
	(
		const UINT				Frame,
		const RCommandQueue &	CmdQueue
	)
	{
		boost::mutex::scoped_lock Lock(Mutex);

		ThrowOnError
		(
			CmdQueue->Signal(Fence.Get(), Values[Frame])
		);
		
		return Values[Frame];
	}

	inline UINT64 RFence::Increment(const UINT Frame, const RCommandQueue & CmdQueue)
	{
		boost::mutex::scoped_lock Lock(Mutex);

		ThrowOnError
		(
			CmdQueue->Signal(Fence.Get(), Values[Frame])
		);

		return Values[Frame]++;
	}

	inline void RFence::GPUWait
	(
		const UINT				Frame,
		const RCommandQueue &	CmdQueue
	)	const
	{
		ThrowOnError
		(
			CmdQueue->Wait(Fence.Get(), Values[Frame])
		);
	}

	inline void RFence::Flush
	(
		const UINT				Frame,
		const RCommandQueue &	CmdQueue
	)
	{
		Signal(Frame, CmdQueue);
		GPUWait(Frame, CmdQueue);
	}

	inline void RCommandQueue::WaitForCompletion
	(
		const UINT Frame
	)	const
	{
		Fence->WaitForPreviousFrame(Frame);
	}

	inline void RCommandQueue::WaitForGPU
	(
		const UINT Frame
	)	const
	{
		Fence->Signal(Frame, *this);
		Fence->WaitForCompletion(Frame);
	}

	inline void RCommandQueue::WaitForFence(const UINT64 Value) const
	{
		Fence->WaitForFence(Value);
	}

	inline void RCommandQueue::GotoNextFrame
	(
		const UINT		Frame,
		const UINT64	Value
	)	const
	{
		Fence->SetValue(Frame, Value);
	}

	inline UINT64 RCommandQueue::SignalFence
	(
		const UINT Frame
	)	const
	{
		return Fence->Signal(Frame, *this);
	}

	inline UINT64 RCommandQueue::IncrementFence(const UINT Frame) const
	{
		return Fence->Increment(Frame, *this);
	}

	inline bool RCommandQueue::FenceCompleted
	(
		const UINT64 Value
	)	const
	{
		if (Value > Fence->GetCompletedValue())
		{
			return Value <= Fence->RefreshCompletedValue();
		}

		return true;
	}
}