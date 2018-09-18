#include "Precompiled.h"

#include "Raw/RawFence.h"

#include "WindowsH.h"

namespace D3D
{
	ErrorCode RFence::Create(D3D12_FENCE_FLAGS Flags)
	{
		ErrorCode Error;

		if ((Error = DEVICE->CreateFence(0, Flags, IID_PPV_ARGS(&Fence))))
		{
			return Error;
		}

		if (IS_INVALID_HANDLE(Event))
		{
			EventCB = Event = CreateEvent(NULL, FALSE, FALSE, NULL);
		}

		if (IS_INVALID_HANDLE(Event))
		{
			return HRESULT_LAST_ERROR;
		}

		return S_OK;
	}
}
