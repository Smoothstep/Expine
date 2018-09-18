#pragma once

#include "Raw/RawCommandAllocator.h"

namespace D3D
{
	constexpr Uint CommandListTypeNum = 4;

	class CCommandAllocator : public RCommandAllocator
	{};

	class CCommandAllocatorManager
	{
	private:

		TArray<TQueue<
			SharedPointer<CCommandAllocator> >, CommandListTypeNum>  AvailableAllocators;
		TArray<TVector<
			SharedPointer<CCommandAllocator> >, CommandListTypeNum>	Allocators;

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