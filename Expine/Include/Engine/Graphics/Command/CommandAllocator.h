#pragma once

#include "RawCommandAllocator.h"

namespace D3D
{
#define COMMAND_LIST_TYPE_NUM 4

	class CCommandAllocator : public RCommandAllocator
	{};

	class CCommandAllocatorManager
	{
	private:

		TArray<TQueue<
			SharedPointer<CCommandAllocator> >, COMMAND_LIST_TYPE_NUM>  AvailableAllocators;
		TArray<TVector<
			SharedPointer<CCommandAllocator> >, COMMAND_LIST_TYPE_NUM>	Allocators;

		SharedPointer<CCommandAllocator> CreateAllocator
		(
			const D3D12_COMMAND_LIST_TYPE Type
		)
		{
			SharedPointer<CCommandAllocator> Allocator = new CCommandAllocator();
			{
				ThrowOnError(Allocator->Create(Type));
			}

			Allocators[Type].push_back(Allocator);

			return Allocator;
		}

	public:

		SharedPointer<CCommandAllocator> RequestAllocator
		(
			const D3D12_COMMAND_LIST_TYPE Type
		)
		{
			if (!AvailableAllocators[Type].empty())
			{
				SharedPointer<CCommandAllocator> Allocator = AvailableAllocators[Type].front(); AvailableAllocators[Type].pop();
				{
					return Allocator;
				}
			}

			return CreateAllocator(Type);
		}

		void RetireAllocator
		(
			const SharedPointer<CCommandAllocator> Allocator
		)
		{
			AvailableAllocators[Allocator->GetType()].push(Allocator);
		}
	};
}