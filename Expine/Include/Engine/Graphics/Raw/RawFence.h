#pragma once

#include "RawDevice.h"

namespace D3D
{
	class RCommandQueue;
	class RFence
	{
	private:

		HANDLE Event;
		HANDLE EventCB;

		ComPointer<IFence> Fence;

	protected:

		UINT64 Values[FRAME_COUNT] = {};
		UINT64 LastValueCompleted = 0;
		TMutex Mutex;

	public:

		inline UINT64 GetValue
		(
			const UINT Frame
		)	const
		{
			return Values[Frame];
		}

		inline void SetValue
		(
			const UINT		Frame,
			const UINT64	Value
		)
		{
			Values[Frame] = Value;
		}

		inline UINT64 GetCompletedValue() const
		{
			return LastValueCompleted;
		}

		inline UINT64 RefreshCompletedValue()
		{
			return LastValueCompleted = max(Fence->GetCompletedValue(), LastValueCompleted);
		}

		inline UINT64 RefreshCompletedValueSafe()
		{
			boost::unique_lock<TMutex> Lock(Mutex);
			{
				return RefreshCompletedValue();
			}
		}

	public:

		ErrorCode Create
		(
			D3D12_FENCE_FLAGS Flags = D3D12_FENCE_FLAG_NONE
		);

	public:

		inline UINT64 Signal
		(
			const UINT				Frame,
			const RCommandQueue &	CmdQueue
		);

		inline UINT64 Increment
		(
			const UINT				Frame,
			const RCommandQueue &	CmdQueue
		);

		inline void GPUWait
		(
			const UINT				Frame,
			const RCommandQueue &	CmdQueue
		)	const;

		inline void Flush
		(
			const UINT				Frame,
			const RCommandQueue &	CmdQueue
		);

		inline void WaitForFence
		(
			const UINT64 Value
		)
		{
			boost::mutex::scoped_lock Lock(Mutex);

			if (RefreshCompletedValue() < Value)
			{
				ThrowOnError
				(
					Fence->SetEventOnCompletion(Value, Event)
				);
				
				WaitForSingleObjectEx(EventCB, INFINITE, FALSE);
				
				LastValueCompleted = Value;
			}
		}

		template
		<
			class Context
		>
		inline void RegisterCompletionEventHandler(VOID(NTAPI Context::*Func) (BOOLEAN), Context * Ctx)
		{
			RegisterWaitForSingleObject(&EventCB, Event, *reinterpret_cast<WAITORTIMERCALLBACK*>(&Func), Ctx, INFINITE, 0);
		}

		inline void WaitForPreviousFrame
		(
			const UINT Frame
		)
		{
			boost::mutex::scoped_lock Lock(Mutex);

			if (RefreshCompletedValue() < Values[Frame])
			{
				ThrowOnError
				(
					Fence->SetEventOnCompletion(Values[Frame], Event)
				);

				WaitForSingleObjectEx(EventCB, INFINITE, FALSE);
			}
		}

		inline void WaitForCompletion
		(
			const UINT Frame
		)
		{
			boost::mutex::scoped_lock Lock(Mutex);

			ThrowOnError
			(
				Fence->SetEventOnCompletion(Values[Frame], Event)
			);

			WaitForSingleObjectEx(EventCB, INFINITE, FALSE);

			Values[Frame]++;
		}
	};
}