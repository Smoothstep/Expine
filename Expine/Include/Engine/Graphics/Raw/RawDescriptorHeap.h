#pragma once

#include "Raw/RawDevice.h"
#include "Raw/RawCommandList.h"

namespace D3D
{
	static constexpr UINT MaxHandlesPerCopy = 16;

	struct DescriptorHeapEntry;
	struct DescriptorHeapRange;

	class RDescriptorHeap
	{
	protected:

		ComPointer<IDescriptorHeap> DescriptorHeap;

		inline void operator=
		(
			RDescriptorHeap & Other
		)
		{
			DescriptorHeap = Other.DescriptorHeap;

			std::swap(IsCPUOnlyHeap,	Other.IsCPUOnlyHeap);
			std::swap(HeapType,			Other.HeapType);
			std::swap(IncrementSize,	Other.IncrementSize);
			std::swap(OffsetMax,		Other.OffsetMax);
			std::swap(OffsetCurrent,	Other.OffsetCurrent);
		}

	private:

		UINT IncrementSize	= 0;
		UINT OffsetMax		= 0;
		UINT OffsetCurrent	= 0;
		
		BOOL IsCPUOnlyHeap;

		D3D12_DESCRIPTOR_HEAP_TYPE HeapType;

	private:

		D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc;

	public:

		inline operator IDescriptorHeap *() const
		{
			return DescriptorHeap.Get();
		}

		inline IDescriptorHeap * operator->() const
		{
			return DescriptorHeap.Get();
		}

		inline IDescriptorHeap * Get() const
		{
			return DescriptorHeap.Get();
		}

		inline UINT GetDescriptorsCount() const
		{
			return OffsetMax;
		}

		inline UINT GetIncrementSize() const
		{
			return IncrementSize;
		}

		inline bool IsCPUOnly() const
		{
			return IsCPUOnlyHeap;
		}

		inline D3D12_DESCRIPTOR_HEAP_TYPE GetType() const
		{
			return HeapType;
		}

		inline const D3D12_DESCRIPTOR_HEAP_DESC & GetDesc() const
		{
			return DescriptorHeapDesc;
		}

	public:

		inline DescriptorHeapRange AsDescriptorHeapRange() const;

		inline D3D12_CPU_DESCRIPTOR_HANDLE GetCPUAddress() const
		{
			return DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		}

		inline D3D12_CPU_DESCRIPTOR_HANDLE GetCPUAddress
		(
			const UINT Offset
		)	const
		{
			return 
			CD3DX12_CPU_DESCRIPTOR_HANDLE
			(
				DescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 
				Offset, 
				IncrementSize
			);
		}

		inline D3D12_GPU_DESCRIPTOR_HANDLE GetGPUAddress() const
		{
			return DescriptorHeap->GetGPUDescriptorHandleForHeapStart();
		}

		inline D3D12_GPU_DESCRIPTOR_HANDLE GetGPUAddress
		(
			const UINT Offset
		)	const
		{
			return 
			CD3DX12_GPU_DESCRIPTOR_HANDLE
			(
				DescriptorHeap->GetGPUDescriptorHandleForHeapStart(),
				Offset,
				IncrementSize
			);
		}

		virtual ErrorCode Create
		(
			const D3D12_DESCRIPTOR_HEAP_TYPE	Type,
			const UINT							Count = 1,
			const D3D12_DESCRIPTOR_HEAP_FLAGS	Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
		);

		inline ErrorCode Create_RTV
		(
			const UINT Count = 1
		)
		{
			return Create
			(
				D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 
				Count
			);
		}

		inline ErrorCode Create_DSV
		(
			const UINT Count = 1
		)
		{
			return Create
			(
				D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 
				Count
			);
		}

		inline ErrorCode Create_CBV_SRV_UAV
		(
			const UINT Count = 1,
			const D3D12_DESCRIPTOR_HEAP_FLAGS Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
		)
		{
			return Create
			(
				D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 
				Count, 
				Flags
			);
		}

		inline ErrorCode Create_Sampler
		(
			const UINT Count = 1,
			const D3D12_DESCRIPTOR_HEAP_FLAGS Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
		)
		{
			return Create
			(
				D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
				Count,
				Flags
			);
		}

		template
		<
			size_t CountSource,
			size_t CountDesctination
		>
		static inline void CopyInto
		(
			const DescriptorHeapRange * pRangesSource,
			const DescriptorHeapRange * pRangesDestination
		);

		static inline void CopyInto
		(
			const DescriptorHeapRange & RangeSource,
			const DescriptorHeapRange & RangeDestination
		);

		static inline void CopyInto
		(
			const D3D12_CPU_DESCRIPTOR_HANDLE	DescriptorHandleDestination,
			const D3D12_CPU_DESCRIPTOR_HANDLE *	DescriptorHandleSource,
			const D3D12_DESCRIPTOR_HEAP_TYPE	DescriptorHeapType,
			const UINT							DescriptorCountSource
		);

		static inline void CopyInto
		(
			const D3D12_CPU_DESCRIPTOR_HANDLE *	DescriptorHandleDestination,
			const D3D12_CPU_DESCRIPTOR_HANDLE *	DescriptorHandleSource,
			const D3D12_DESCRIPTOR_HEAP_TYPE	DescriptorHeapType,
			const UINT							DescriptorCountSource
		);

		template
		<
			size_t CountSource,
			size_t CountDesctination
		>
		static inline void CopyInto
		(
			const RDescriptorHeap	** ppDescriptorHeapsSource,
			const RDescriptorHeap	** ppDescriptorHeapsDestination,
			const UINT				* pOffsetsSource,
			const UINT				* pOffsetsDestination,
			const UINT				* pRanges
		);

		static inline void CopyInto
		(
			const DescriptorHeapEntry & EntryDestination,
			const DescriptorHeapEntry * EntriesSource,
			const UINT					NumEntries
		);

		static inline void CopyInto
		(
			const RDescriptorHeap * pDescriptorHeapSource,
			const RDescriptorHeap * pDescriptorHeapDestination,
			const UINT				OffsetSource = 0,
			const UINT				OffsetDestination = 0
		);

		virtual void CopyDescriptorHeapEntries
		(
			const DescriptorHeapEntry & Destination,
			const DescriptorHeapEntry * Entries,
			const UINT					NumEntries
		);

		virtual void CopyDescriptorHeapEntries
		(
			const DescriptorHeapRange & RangeDestination,
			const DescriptorHeapRange & RangeSource
		);

		virtual void CopyDescriptorHeapEntries
		(
			const DescriptorHeapRange & DestinationRange,
			const DescriptorHeapEntry * Entries
		);
	};

	struct DescriptorHeapEntry
	{
		inline DescriptorHeapEntry() : Offset(0)
		{}

		inline operator D3D12_CPU_DESCRIPTOR_HANDLE() const
		{
			return DescriptorHeap->GetCPUAddress(Offset);
		}

		virtual inline bool Valid() const
		{
			if (DescriptorHeap)
			{
				return DescriptorHeap->GetDescriptorsCount() > Offset;
			}

			return false;
		}

		inline DescriptorHeapEntry
		(
			const RDescriptorHeap *	pDescriptorHeap,
			const UINT				Offset = 0
		)
		{
			this->DescriptorHeap	= pDescriptorHeap;
			this->Offset			= Offset;
		}

		ConstPointer<RDescriptorHeap> DescriptorHeap;
		UINT Offset;
	};

	struct DescriptorHeapRange : public DescriptorHeapEntry
	{
		inline operator D3D12_CPU_DESCRIPTOR_HANDLE() const
		{
			return DescriptorHeap->GetCPUAddress(Offset);
		}

		virtual inline bool Valid() const override
		{
			if (DescriptorHeap)
			{
				return DescriptorHeap->GetDescriptorsCount() >= Offset + Range;
			}

			return false;
		}

		inline DescriptorHeapRange() 
			: DescriptorHeapEntry(0), Range(0)
		{}

		inline DescriptorHeapEntry operator[]
		(
			const UINT Index
		)	const
		{
			Ensure
			(
				Index < Range
			);

			return { DescriptorHeap.Get(), Offset + Index };
		}

		inline DescriptorHeapRange
		(
			const DescriptorHeapEntry & Entry
		) :
			DescriptorHeapEntry(Entry)
		{
			Range = 1;
		}

		inline DescriptorHeapRange
		(
			const RDescriptorHeap *	pDescriptorHeap,
			const UINT				Offset = 0,
			const UINT				Range  = 1
		) :
			DescriptorHeapEntry(pDescriptorHeap, Offset), Range(Range)
		{}

		UINT Range;
	};

	inline void RDescriptorHeap::CopyDescriptorHeapEntries
	(
		const DescriptorHeapRange & DestinationRange,
		const DescriptorHeapEntry * Entries
	)
	{
		Ensure
		(
			DestinationRange.DescriptorHeap.Get() == this
		);

		RDescriptorHeap::CopyInto(DestinationRange, Entries, DestinationRange.Range);
	}

	inline void RDescriptorHeap::CopyDescriptorHeapEntries
	(
		const DescriptorHeapRange & RangeDestination,
		const DescriptorHeapRange & RangeSource
	)
	{
		Ensure
		(
			RangeDestination.DescriptorHeap.Get() == this
		);

		RDescriptorHeap::CopyInto(RangeDestination, RangeSource);
	}

	inline void RDescriptorHeap::CopyDescriptorHeapEntries
	(
		const DescriptorHeapEntry & Destination,
		const DescriptorHeapEntry * Entries,
		const UINT					NumEntries
	)
	{
		Ensure
		(
			Destination.DescriptorHeap.Get() == this
		);

		RDescriptorHeap::CopyInto(Destination, Entries, NumEntries);
	}

	template
	<
		size_t CountSource,
		size_t CountDesctination
	>
	inline void RDescriptorHeap::CopyInto
	(
		const DescriptorHeapRange * pRangesSource,
		const DescriptorHeapRange * pRangesDestination
	)
	{
		Ensure
		(
			pRangesSource &&
			pRangesDestination
		);

		D3D12_CPU_DESCRIPTOR_HANDLE HandlesSource[CountSource];
		D3D12_CPU_DESCRIPTOR_HANDLE HandlesDestination[CountDesctination];

		D3D12_DESCRIPTOR_HEAP_TYPE Type = pRangesDestination->DescriptorHeap->GetType();

#pragma unroll(1)
		for (UINT N = 0; N < CountSource; ++N)
		{
			HandlesSource[N] = pRangesSource[N];
		}

#pragma unroll(1)
		for (UINT N = 0; N < CountDesctination; ++N)
		{
			HandlesDestination[N] = pRangesDestination[N];
		}

		UINT RangesSource[CountSource];
		UINT RangesDestination[CountDesctination];

#pragma unroll(1)
		for (UINT N = 0; N < CountDesctination; ++N)
		{
			RangesDestination[N] = pRangesDestination[N].Range;
		}

#pragma unroll(1)
		for (UINT N = 0; N < CountSource; ++N)
		{
			RangesSource[N] = pRangesSource[N].Range;
		}

		DEVICE->CopyDescriptors
		(
			CountDesctination,
			HandlesDestination,
			RangesDestination,
			CountSource,
			HandlesSource,
			RangesSource,
			Type
		);
	}

	inline void RGrpCommandList::SetDescriptorHeaps
	(
		const UINT						NumDescriptorHeaps,
		const RDescriptorHeap * const *	DescriptorHeaps
	)	const
	{
		LPDescriptorHeap * ppDescriptorHeaps = new LPDescriptorHeap[NumDescriptorHeaps];
		{
			for (UINT N = 0; N < NumDescriptorHeaps; ++N)
			{
				ppDescriptorHeaps[N] = DescriptorHeaps[N]->Get();
			}
		}

		CommandList->SetDescriptorHeaps(NumDescriptorHeaps, ppDescriptorHeaps);

		delete[] ppDescriptorHeaps;
	}

	inline void RDescriptorHeap::CopyInto
	(
		const DescriptorHeapRange & RangeSource,
		const DescriptorHeapRange & RangeDestination
	)
	{
		Ensure
		(
			RangeSource.Valid() &&
			RangeDestination.Valid()
		);

		const D3D12_CPU_DESCRIPTOR_HANDLE HandleSource[]		= { RangeSource			};
		const D3D12_CPU_DESCRIPTOR_HANDLE HandleDestination[]	= { RangeDestination	};

		const UINT RangesSource[]		= { RangeSource.Range		};
		const UINT RangesDestination[]	= { RangeDestination.Range	};

		const UINT NumDescriptorsSource			= 1;
		const UINT NumDescriptorsDestination	= 1;

		DEVICE->CopyDescriptors
		(
			NumDescriptorsDestination,
			HandleDestination,
			RangesDestination,
			NumDescriptorsSource,
			HandleSource,
			RangesSource,
			RangeDestination.DescriptorHeap->GetType()
		);
	}

	inline void RDescriptorHeap::CopyInto
	(
		const RDescriptorHeap * pDescriptorHeapSource,
		const RDescriptorHeap * pDescriptorHeapDestination,
		const UINT				OffsetSource,
		const UINT				OffsetDestination
	)
	{
		Ensure
		(
			pDescriptorHeapSource &&
			pDescriptorHeapDestination
		);

		Ensure
		(
			OffsetSource < pDescriptorHeapSource->GetDescriptorsCount()
		);

		Ensure
		(
			OffsetDestination < pDescriptorHeapDestination->GetDescriptorsCount()
		);

		Ensure
		(
			pDescriptorHeapSource->GetDesc().Type == pDescriptorHeapDestination->GetDesc().Type
		);

		Ensure
		(
			pDescriptorHeapSource->IsCPUOnly()
		);

		DEVICE->CopyDescriptorsSimple
		(
			1,
			pDescriptorHeapDestination->GetCPUAddress(OffsetDestination),
			pDescriptorHeapSource->GetCPUAddress(OffsetSource),
			pDescriptorHeapDestination->GetDesc().Type
		);
	}

	inline void RDescriptorHeap::CopyInto
	(
		const DescriptorHeapEntry & EntryDestination,
		const DescriptorHeapEntry * EntriesSource,
		const UINT					NumEntries
	)
	{
		Ensure
		(
			EntriesSource
		);

		Ensure
		(
			NumEntries &&
			NumEntries <= MaxHandlesPerCopy
		);

		D3D12_CPU_DESCRIPTOR_HANDLE HandlesSource[MaxHandlesPerCopy];
		D3D12_CPU_DESCRIPTOR_HANDLE HandleDestination = EntryDestination;

		for (UINT N = 0; N < NumEntries; ++N)
		{
			HandlesSource[N] = EntriesSource[N];
		}

		UINT RangesSource[MaxHandlesPerCopy] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
		UINT RangeDestination = NumEntries;

		DEVICE->CopyDescriptors
		(
			1,
			&HandleDestination,
			&RangeDestination,
			NumEntries,
			HandlesSource,
			RangesSource,
			EntryDestination.DescriptorHeap->GetType()
		);
	}

	inline void RDescriptorHeap::CopyInto
	(
		const D3D12_CPU_DESCRIPTOR_HANDLE	DescriptorHandleDestination,
		const D3D12_CPU_DESCRIPTOR_HANDLE *	DescriptorHandleSource,
		const D3D12_DESCRIPTOR_HEAP_TYPE	DescriptorHeapType,
		const UINT							DescriptorCountSource
	)
	{
		const UINT DescriptorHandlesRange = 1;

		DEVICE->CopyDescriptors
		(
			1,
			&DescriptorHandleDestination,
			&DescriptorHandlesRange,
			DescriptorCountSource,
			DescriptorHandleSource,
			&DescriptorHandlesRange,
			DescriptorHeapType
		);
	}

	inline void RDescriptorHeap::CopyInto
	(
		const D3D12_CPU_DESCRIPTOR_HANDLE *	DescriptorHandleDestination,
		const D3D12_CPU_DESCRIPTOR_HANDLE *	DescriptorHandleSource,
		const D3D12_DESCRIPTOR_HEAP_TYPE	DescriptorHeapType,
		const UINT							DescriptorCountSource
	)
	{
		const UINT DescriptorHandlesRangeSource[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
		const UINT DescriptorHandleCountDestination = 1;

		DEVICE->CopyDescriptors
		(
			DescriptorHandleCountDestination,
			DescriptorHandleDestination,
			&DescriptorCountSource,
			DescriptorCountSource,
			DescriptorHandleSource,
			DescriptorHandlesRangeSource,
			DescriptorHeapType
		);
	}

	template
	<
		size_t CountSource,
		size_t CountDesctination
	>
	inline void RDescriptorHeap::CopyInto
	(
		const RDescriptorHeap	** ppDescriptorHeapsSource,
		const RDescriptorHeap	** ppDescriptorHeapsDestination,
		const UINT				* pOffsetsSource,
		const UINT				* pOffsetsDestination,
		const UINT				* pRanges
	)
	{
		Ensure
		(
			ppDescriptorHeapsSource			&&
			ppDescriptorHeapsDestination	&&
			pOffsetsSource					&&
			pOffsetsDestination				&&
			pRanges
		);

		UINT SizesSource[CountSource];

		D3D12_CPU_DESCRIPTOR_HANDLE HandlesSource[CountSource];
		D3D12_CPU_DESCRIPTOR_HANDLE HandlesDestination[CountDesctination];

		D3D12_DESCRIPTOR_HEAP_TYPE Type = ppDescriptorHeapsSource[0]->GetType();

#pragma unroll(1)
		for (UINT N = 0; N < CountSource; ++N)
		{
			SizesSource[N] = 1;

			Ensure
			(
				ppDescriptorHeapsSource[N]
			);

			Ensure
			(
				pOffsetsSource[N] < ppDescriptorHeapsSource[N]->GetDescriptorsCount()
			);

			Ensure
			(
				ppDescriptorHeapsSource[N]->GetDesc().Type == Type
			);

			Ensure
			(
				ppDescriptorHeapsSource[N]->IsCPUOnly()
			);

			HandlesSource[N] = ppDescriptorHeapsSource[N]->GetCPUAddress(pOffsetsSource[N]);
		}

#pragma unroll(1)
		for (UINT N = 0; N < CountDesctination; ++N)
		{
			Ensure
			(
				ppDescriptorHeapsDestination[N]
			);

			Ensure
			(
				pOffsetsDestination[N] < ppDescriptorHeapsDestination[N]->GetDescriptorsCount()
			);

			Ensure
			(
				ppDescriptorHeapsDestination[N]->GetType() == Type
			);

			HandlesDestination[N] = ppDescriptorHeapsDestination[N]->GetCPUAddress(pOffsetsDestination[N]);
		}

		DEVICE->CopyDescriptors
		(
			CountDesctination,
			HandlesDestination,
			pRanges,
			CountSource,
			HandlesSource,
			SizesSource,
			Type
		);
	}

	template
	<
		size_t NumDescriptorHeaps
	>
	inline void RGrpCommandList::SetDescriptorHeaps
	(
		const RDescriptorHeap * const * DescriptorHeaps
	)	const
	{
		IDescriptorHeap * ppDescriptorHeaps[NumDescriptorHeaps];
		{
			for (UINT N = 0; N < NumDescriptorHeaps; ++N)
			{
				ppDescriptorHeaps[N] = DescriptorHeaps[N]->Get();
			}
		}

		CommandList->SetDescriptorHeaps(NumDescriptorHeaps, ppDescriptorHeaps);
	}

	inline void RGrpCommandList::SetDescriptorHeap(const RDescriptorHeap * pDescriptorHeap) const
	{
		IDescriptorHeap * Heap = *pDescriptorHeap;
		{
			CommandList->SetDescriptorHeaps(1, &Heap);
		}
	}

	inline void RGrpCommandList::SetGraphicsRootDescriptorTable
	(
		const UINT				RootParameter,
		const RDescriptorHeap * pDescriptorHeap
	)	const
	{
		CommandList->SetGraphicsRootDescriptorTable(RootParameter, pDescriptorHeap->GetGPUAddress());
	}

	inline void RGrpCommandList::SetComputeRootDescriptorTable
	(
		const UINT				RootParameter,
		const RDescriptorHeap * pDescriptorHeap
	)	const
	{
		CommandList->SetComputeRootDescriptorTable(RootParameter, pDescriptorHeap->GetGPUAddress());
	}

	inline void RGrpCommandList::SetGraphicsRootDescriptorTable
	(
		const UINT				RootParameter,
		const UINT				Offset,
		const RDescriptorHeap * pDescriptorHeap
	)	const
	{
		CommandList->SetGraphicsRootDescriptorTable(RootParameter, pDescriptorHeap->GetGPUAddress(Offset));
	}

	inline void RGrpCommandList::SetComputeRootDescriptorTable
	(
		const UINT				RootParameter,
		const UINT				Offset,
		const RDescriptorHeap * pDescriptorHeap
	)	const
	{
		CommandList->SetComputeRootDescriptorTable(RootParameter, pDescriptorHeap->GetGPUAddress(Offset));
	}

	inline void RGrpCommandList::SetGraphicsRootDescriptorTable
	(
		const UINT					RootParameter,
		const DescriptorHeapEntry & DescriptorHeapEntry
	)	const
	{
		CommandList->SetGraphicsRootDescriptorTable(RootParameter, DescriptorHeapEntry.DescriptorHeap->GetGPUAddress(DescriptorHeapEntry.Offset));
	}

	inline void RGrpCommandList::SetComputeRootDescriptorTable
	(
		const UINT					RootParameter,
		const DescriptorHeapEntry & DescriptorHeapEntry
	)	const
	{
		CommandList->SetComputeRootDescriptorTable(RootParameter, DescriptorHeapEntry.DescriptorHeap->GetGPUAddress(DescriptorHeapEntry.Offset));
	}

	inline void RGrpCommandList::SetGraphicsRootDescriptorTable
	(
		const UINT				RootParameter, 
		const RDescriptorHeap * pDescriptorHeap, 
		const UINT				Offset
	)	const
	{
		CommandList->SetGraphicsRootDescriptorTable(RootParameter, pDescriptorHeap->GetGPUAddress(Offset));
	}

	inline void RGrpCommandList::SetComputeRootDescriptorTable
	(
		const UINT				RootParameter,
		const RDescriptorHeap * pDescriptorHeap,
		const UINT				Offset
	)	const
	{
		CommandList->SetComputeRootDescriptorTable(RootParameter, pDescriptorHeap->GetGPUAddress(Offset));
	}

	inline void RGrpCommandList::SetGraphicsRoot32BitConstants
	(
		const UINT				RootParameter,
		const UINT				Num32Bits,
		const void			*	pData
	)	const
	{
		CommandList->SetGraphicsRoot32BitConstants(RootParameter, Num32Bits, pData, 0);
	}

	inline void RGrpCommandList::SetComputeRoot32BitConstants
	(
		const UINT				RootParameter,
		const UINT				Num32Bits,
		const void			*	pData
	)	const
	{
		CommandList->SetComputeRoot32BitConstants(RootParameter, Num32Bits, pData, 0);
	}

	inline DescriptorHeapRange RDescriptorHeap::AsDescriptorHeapRange() const
	{
		return { this, OffsetCurrent, OffsetMax };
	}
}