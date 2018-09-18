#include "Precompiled.h"

#include "Descriptor/DescriptorHeapPool.h"
#include "Command/CommandQueue.h"

namespace D3D
{
	static CCPUDescriptorHeapManager GCPUDescriptorHeapManager;
	static CGPUDescriptorHeapManager GGPUDescriptorHeapManager;

	CDescriptorHeap * CCPUDescriptorHeapManager::RequestDescriptorHeap(const D3D12_DESCRIPTOR_HEAP_TYPE Type, const UINT Size)
	{
		std::scoped_lock<TMutex> Lock(Mutex);
		{
			CDescriptorHeap * Heap = new CDescriptorHeap();

			ThrowOnError
			(
				Heap->Create(Type, DescriptorHeapSizeForType(Type))
			);

			DescriptorHeaps.push_back(Heap);

			return Heap;
		}
	}
	
	DescriptorHeapRange CCPUDescriptorHeapPool::RequestDescriptorHeapRange(const UINT Size)
	{
		Ensure
		(
			Size <= DescriptorHeapSizeForType(Type)
		);

		DescriptorHeapRange DescriptorRange;

		UINT Index;

		const auto Iter = std::find_if(DescriptorHeaps.begin(), DescriptorHeaps.end(), [&](CDescriptorHeap * pHeap)
		{
			return (Index = pHeap->RegisterRange(Size)) != -1;
		});

		if (Iter == DescriptorHeaps.end())
		{
			CDescriptorHeap * pHeap = CCPUDescriptorHeapManager::Instance().RequestDescriptorHeap(Type, Size);
			{
				Index = pHeap->RegisterRange(Size);

				if (pHeap->HasSpace())
				{
					DescriptorHeaps.push_back(pHeap);
				}
			}

			DescriptorRange = DescriptorHeapRange(pHeap, Index, Size);
		}
		else
		{
			DescriptorRange = DescriptorHeapRange(*Iter, Index, Size);

			if (!(*Iter)->HasSpace())
			{
				DescriptorHeaps.erase(Iter);
			}
		}

		return DescriptorRange;
	}
	
	CMutableDescriptorHeap * CGPUDescriptorHeapManager::RequestDescriptorHeap(const D3D12_DESCRIPTOR_HEAP_TYPE Type)
	{
		std::scoped_lock<TMutex> Lock(Mutex);

		while (!DescriptorHeapsRetired[Type].empty() && CCommandQueueDirect::Instance().FenceCompleted(DescriptorHeapsRetired[Type].front().first))
		{
			DescriptorHeapsAvailable[Type].push(DescriptorHeapsRetired[Type].front().second); DescriptorHeapsRetired[Type].pop();
		}

		if (!DescriptorHeapsAvailable[Type].empty())
		{
			CMutableDescriptorHeap * Heap = DescriptorHeapsAvailable[Type].front();
			{
				DescriptorHeapsAvailable[Type].pop();
			}

			return Heap;
		}

		UniquePointer<CMutableDescriptorHeap> Heap = new CMutableDescriptorHeap();

		ThrowOnError
		(
			Heap->Create(Type, NumDescriptorsPerHeap, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
		);

		return DescriptorHeaps[Type].emplace(DescriptorHeaps[Type].end(), std::move(Heap.Detach()))->Get();
	}

	CMutableDescriptorHeap * CGPUDescriptorHeapManager::RequestDescriptorHeap(CMutableDescriptorHeap * Copy, const UINT Size)
	{
		std::scoped_lock<TMutex> Lock(Mutex);

		UniquePointer<CMutableDescriptorHeap> Heap = new CMutableDescriptorHeap();
		{
			const D3D12_DESCRIPTOR_HEAP_TYPE Type = Copy->GetType();

			ThrowOnError
			(
				Heap->Create(Copy, Type, Size)
			);

			return DescriptorHeaps[Type].emplace(DescriptorHeaps[Type].end(), std::move(Heap.Detach()))->Get();
		}
	}

	void CGPUDescriptorHeapManager::RequestDescriptorHeapCopy(CMutableDescriptorHeap * Heap, const UINT HeapCopySize, const UINT64 FenceValue)
	{
		static constexpr UINT MaxDescriptorHandlesPerCopy = 16;

		D3D12_DESCRIPTOR_HEAP_TYPE HeapType = Heap->GetType();

		CMutableDescriptorHeap * HeapCopy = RequestDescriptorHeap(Heap, HeapCopySize);

		UINT First = HeapCopy->DescriptorTableCache.FirstIndex();
		UINT Count = 0;
		UINT Index = 0;
		UINT Last = 0;
		auto Start = HeapCopy->DescriptorTableCache[First];

		if (Start.Range == 0)
		{
			return;
		}

		D3D12_CPU_DESCRIPTOR_HANDLE SrcHandles[MaxDescriptorHandlesPerCopy];
		D3D12_CPU_DESCRIPTOR_HANDLE DstHandles[MaxDescriptorHandlesPerCopy];

		UINT DstRanges[MaxDescriptorHandlesPerCopy];
		UINT SrcRanges[MaxDescriptorHandlesPerCopy] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

		CD3DX12_CPU_DESCRIPTOR_HANDLE DestHandle(HeapCopy->GetCPUAddress());

		const UINT IncrementSize = Heap->GetIncrementSize();

		UINT DestDescriptorCount = 0;

		while (true)
		{
			UINT CountBefore = Count;
			UINT LastIndex = First;

			for (Last = First; ; First = Start.Next)
			{
				Start = HeapCopy->DescriptorTableCache[First];
				{
					Last  += Start.Range;
					Count += Start.Range;
				}

				if (First + Start.Range != Start.Next)
				{
					break;
				}
			}

			if (Count >= MaxDescriptorHandlesPerCopy)
			{
				DstRanges[DestDescriptorCount] = MaxDescriptorHandlesPerCopy - CountBefore;

#pragma unroll(1)
				for (UINT N = 0; CountBefore < Count; ++N)
				{
					SrcHandles[CountBefore++] = HeapCopy->DescriptorTableCache.GetHandles()[First + N];
				}

				while (true)
				{
					DEVICE->CopyDescriptors
					(
						DestDescriptorCount,
						DstHandles,
						DstRanges,
						MaxDescriptorHandlesPerCopy,
						SrcHandles,
						SrcRanges,
						HeapType
					);

					if (Last - First)
					{
						DestDescriptorCount = 1;

						if (Last - First >= MaxDescriptorHandlesPerCopy)
						{
							CopyMemory(SrcHandles, &HeapCopy->DescriptorTableCache.GetHandles()[First], DstRanges[0] = MaxDescriptorHandlesPerCopy);
						}
						else
						{
							DstRanges[0] = Count = Last - First;
							DstHandles[0] = DestHandle.Offset(First);
							break;
						}

						DstHandles[0] = DestHandle.Offset(First += MaxDescriptorHandlesPerCopy);
					}
					else
					{
						DestDescriptorCount = Count = 0;
						break;
					}
				}
			}
			else
			{
				DstHandles[DestDescriptorCount] = DestHandle.Offset(LastIndex, IncrementSize);
				DstRanges[DestDescriptorCount] = Count;

				DestDescriptorCount++;
			}
		
			if (Start.Next < First)
			{
				if (Count)
				{
					for (Index = 0; Index < Count; ++Index)
					{
						SrcHandles[Index] = HeapCopy->DescriptorTableCache.GetHandles()[LastIndex++];
					}

					DEVICE->CopyDescriptors
					(
						DestDescriptorCount,
						DstHandles,
						DstRanges,
						LastIndex - 1,
						SrcHandles,
						SrcRanges,
						HeapType
					);
				}

				break;
			}

			Start = HeapCopy->DescriptorTableCache[First = Start.Next];
		}

		*Heap = *HeapCopy;

		RetireDescriptorHeap(HeapCopy, FenceValue);
	}

	void CGPUDescriptorHeapManager::RetireDescriptorHeap(CMutableDescriptorHeap * Heap, const UINT64 FenceValue)
	{
		std::scoped_lock<TMutex> Lock(CGPUDescriptorHeapManager::Instance().Mutex);

		DescriptorHeapsRetired[Heap->GetType()].push(std::make_pair(FenceValue, Heap));
	}
	
	void CGPUDescriptorHeapPool::RevokeDescriptorHeaps(const UINT64 FenceValue)
	{
		std::scoped_lock<TMutex> Lock(CGPUDescriptorHeapManager::Instance().Mutex);

		for (const auto & Iter : DescriptorHeapsRetired)
		{
			CGPUDescriptorHeapManager::Instance().RetireDescriptorHeap(Iter, FenceValue);
		}

		DescriptorHeapsRetired.clear();
	}

	DescriptorHeapRange CGPUDescriptorHeapPool::RequestFixedDescriptorHeapRange(const UINT Size)
	{
		UINT Index;

		const auto Iter = std::find_if(DescriptorHeapsFixed.begin(), DescriptorHeapsFixed.end(), [&](CDescriptorHeap * pHeap)
		{
			return (Index = pHeap->RegisterRange(Size)) != -1;
		});

		DescriptorHeapRange DescriptorRange;

		if (Iter == DescriptorHeapsFixed.end())
		{
			CMutableDescriptorHeap * pHeap = CGPUDescriptorHeapManager::Instance().RequestDescriptorHeap(Type);
			{
				Index = pHeap->RegisterRange(Size);

				if (pHeap->HasSpace())
				{
					DescriptorHeapsFixed.push_back(pHeap);
				}
			}

			DescriptorRange = DescriptorHeapRange(pHeap, Index, Size);
		}
		else
		{
			DescriptorRange = DescriptorHeapRange(*Iter, Index, Size);

			if (!(*Iter)->HasSpace())
			{
				DescriptorHeapsFixed.erase(Iter);
			}
		}

		return DescriptorRange;
	}

	DescriptorHeapRange CGPUDescriptorHeapPool::RequestDescriptorHeapRange(const UINT Size)
	{
		UINT Index;

		const auto Iter = std::find_if(DescriptorHeaps.begin(), DescriptorHeaps.end(), [&](CDescriptorHeap * pHeap)
		{
			return (Index = pHeap->RegisterRange(Size)) != -1;
		});

		DescriptorHeapRange DescriptorRange;

		if (Iter == DescriptorHeaps.end())
		{
			CMutableDescriptorHeap * pHeap = CGPUDescriptorHeapManager::Instance().RequestDescriptorHeap(Type);
			{
				Index = pHeap->RegisterRange(Size);

				if (pHeap->HasSpace())
				{
					DescriptorHeaps.push_back(pHeap);
				}
			}

			DescriptorRange = DescriptorHeapRange(pHeap, Index, Size);
		}
		else
		{
			DescriptorRange = DescriptorHeapRange(*Iter, Index, Size);

			if (!(*Iter)->HasSpace())
			{
				DescriptorHeaps.erase(Iter);
			}
		}

		return DescriptorRange;
	}

	ErrorCode CMutableDescriptorHeap::Create(CMutableDescriptorHeap * Copy, const D3D12_DESCRIPTOR_HEAP_TYPE Type, const UINT Size)
	{
		DescriptorTableCache.Initialize(Size);
		DescriptorTableCache = Copy->DescriptorTableCache;
		{
			return RDescriptorHeap::Create(Type, Size, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		}
	}

	void CMutableDescriptorHeap::ClearAssignments()
	{
		DescriptorTableCache.Clear();
	}
}
