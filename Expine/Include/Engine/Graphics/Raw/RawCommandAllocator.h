#pragma once

#include "RawDevice.h"

namespace D3D
{
	class RCommandAllocator
	{
	private:

		ComPointer<ICommandAllocator> CommandAllocator;

	protected:

		D3D12_COMMAND_LIST_TYPE CommandListType;

	public:

		inline D3D12_COMMAND_LIST_TYPE GetType() const
		{
			return CommandListType;
		}

		inline operator ICommandAllocator*() const
		{
			return CommandAllocator.Get();
		}

		inline ICommandAllocator * Get() const
		{
			return CommandAllocator.Get();
		}

	public:

		ErrorCode Create
		(
			const D3D12_COMMAND_LIST_TYPE CmdListType
		)
		{
			CommandListType = CmdListType;

			ErrorCode Error = DEVICE->CreateCommandAllocator
			(
				CommandListType,
				IID_PPV_ARGS(&CommandAllocator)
			);

			if (Error)
			{
				return Error;
			}

			return S_OK;
		}

	public:

		inline ErrorCode Reset() const
		{
			return CommandAllocator->Reset();
		}
	};
}