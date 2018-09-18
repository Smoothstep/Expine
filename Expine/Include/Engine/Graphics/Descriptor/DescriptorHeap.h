#pragma once

#include "Raw/RawDescriptorHeap.h"

namespace D3D
{
	static constexpr UINT NumDescriptorsPerHeap = 1024;

	class CDescriptorHeapAssignments
	{
	private:

		UINT	IndexEnd;
		UINT	IndexStart;
		UINT	Occupied;

		struct RangeType
		{
			UINT Range;
			UINT Next;
			UINT Last;

			inline RangeType()
			{
				Range = Next = Last = 0;
			}
		};

		TVector<RangeType>						Ranges;
		TVector<D3D12_CPU_DESCRIPTOR_HANDLE>	Handles;

	public:

		inline bool IsFull() const
		{
			return Occupied == IndexEnd;
		}

		inline bool IsFree() const
		{
			return IndexStart == 0 && Ranges[IndexStart].Range == 0;
		}

		inline UINT GetFreeSpaceSize() const
		{
			return IndexEnd - Occupied;
		}

		inline UINT FirstIndex() const
		{
			return IndexStart;
		}

		inline const D3D12_CPU_DESCRIPTOR_HANDLE * GetHandles() const
		{
			return &Handles[0];
		}

		inline const RangeType& operator[]
		(
			const UINT Index
		)	const
		{
			return Ranges[Index];
		}

	public:

		~CDescriptorHeapAssignments();

		void Initialize
		(
			const UINT Size
		);

		void Clear();

		void DissociateRange
		(
			const UINT Index
		);

		UINT AssignRange
		(
			const UINT Size
		);

		void AssignHandles
		(
			const D3D12_CPU_DESCRIPTOR_HANDLE	Handles[],
			const UINT							NumHandles,
			const UINT							Offset
		);

		void AssignHandle
		(
			const D3D12_CPU_DESCRIPTOR_HANDLE	Handle,
			const UINT							Offset
		);

		void AssignHandles
		(
			const DescriptorHeapRange		&	Range,
			const UINT							Offset
		);

		inline void operator=(CDescriptorHeapAssignments & Other)
		{
			Occupied = Other.Occupied;

			if (Other.IndexEnd == IndexEnd)
			{
				Ranges.swap(Other.Ranges);
				Handles.swap(Other.Handles);

				Other.Initialize(IndexEnd);
			}
			else
			{
				if (Other.IndexEnd > IndexEnd)
				{
					std::copy(
						Other.Ranges.begin(),
						Other.Ranges.begin() + IndexEnd,
						Ranges.begin());

					std::copy(
						Other.Handles.begin(),
						Other.Handles.begin() + IndexEnd,
						Handles.begin());
				}
				else
				{
					std::copy(
						Other.Ranges.begin(), 
						Other.Ranges.end(),
						Ranges.begin());
					
					std::copy(
						Other.Handles.begin(), 
						Other.Handles.end(),
						Handles.begin());
				}

				Other.Initialize(Other.IndexEnd);
			}
		}
	};

	class CDescriptorHeapTableDistributor
	{
	private:

		UINT Size;
		UINT Begin;
		UINT Offset;
		UINT End;

		DescriptorHeapRange*Ranges;
		DescriptorHeapRange RangeInvalid;

		ConstPointer<RDescriptorHeap> Heap;

	public:

		inline UINT GetOffset() const
		{
			return Offset;
		}

		inline UINT GetSize() const
		{
			return Size;
		}

		inline UINT GetEnd() const
		{
			return End;
		}

		inline UINT GetBegin() const
		{
			return Begin;
		}

		inline RDescriptorHeap * GetHeap() const
		{
			return Heap.Get();
		}

	public:

		CDescriptorHeapTableDistributor
		(
			RDescriptorHeap * Heap, 
			UINT Size, 
			UINT Offset
		);

		~CDescriptorHeapTableDistributor();

		DescriptorHeapRange RequestRange
		(
			UINT Range
		);

		void FreeAll();
		void FreeFront();
		void FreeFront
		(
			UINT Num
		);
		void FreeBack();
		void FreeBack
		(
			UINT Num
		);
	};

	class CDescriptorHeap : public RDescriptorHeap
	{
		friend class CGPUDescriptorHeapManager;

	protected:

		CDescriptorHeapAssignments DescriptorTableCache;

	public:

		inline bool HasSpace() const
		{
			return !DescriptorTableCache.IsFull();
		}

	public:

		CDescriptorHeap();
		~CDescriptorHeap();

		virtual ErrorCode Create
		(
			const D3D12_DESCRIPTOR_HEAP_TYPE	Type,
			const UINT							Size,
			const D3D12_DESCRIPTOR_HEAP_FLAGS	Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
		) override;

		inline void UnregisterRange
		(
			const UINT Index
		)
		{
			DescriptorTableCache.DissociateRange(Index);
		}

		inline UINT RegisterRange
		(
			const UINT Size
		)
		{
			return DescriptorTableCache.AssignRange(Size);
		}
	};

	class CMutableDescriptorHeap : public CDescriptorHeap
	{
	private:

		template
		<
			size_t Count
		>
		void CopyInto
		(
			const	RDescriptorHeap	*	pCPUDescriptorHeap,
			const	UINT				Size,
			const	UINT				OffsetDestination,
			const	UINT			*	pOffsetsSource,
			const	UINT			*	pRangesSource
		);

	public:

		using CDescriptorHeap::CDescriptorHeap;
		using CDescriptorHeap::Create;

		ErrorCode Create
		(
					CMutableDescriptorHeap *	Copy,
			const	D3D12_DESCRIPTOR_HEAP_TYPE	Type,
			const	UINT						Size
		);

		void ClearAssignments();

		template
		<
			size_t Count
		>
		UINT Register
		(
			const	UINT					Size,
			const	RDescriptorHeap		*	pCPUDescriptorHeap,
			const	UINT				*	pOffsets,
			const	UINT				*	pRanges,
					DescriptorHeapRange &	Range
		);

		template
		<
			size_t Count
		>
		UINT Register
		(
			const	UINT					Size,
			const	RDescriptorHeap		*	pCPUDescriptorHeap,
			const	UINT				*	pOffsets,
			const	UINT				*	pRanges
		);

		virtual void CopyDescriptorHeapEntries
		(
			const DescriptorHeapRange	&	Destination,
			const DescriptorHeapRange	&	Source
		) override
		{
			DescriptorTableCache.AssignHandles(Source, Destination.Offset);
			RDescriptorHeap::CopyInto(Source, Destination);
		}

		virtual void CopyDescriptorHeapEntries
		(
			const DescriptorHeapEntry	&	Destination,
			const DescriptorHeapEntry	*	Entries,
			const UINT						NumEntries
		) override
		{
			Ensure
			(
				Destination.DescriptorHeap.Get() == this
			);

			D3D12_CPU_DESCRIPTOR_HANDLE HandlesSource[MaxHandlesPerCopy];
			D3D12_CPU_DESCRIPTOR_HANDLE HandlesDestination[] = { Destination };

			UINT Iterations = NumEntries / MaxHandlesPerCopy;
			UINT Offset = 0;

			for (UINT I = 0; I < Iterations; ++I, Offset += MaxHandlesPerCopy)
			{
				for (UINT N = 0; N < MaxHandlesPerCopy; ++N)
				{
					HandlesSource[N] = Entries[N];
				}

				DescriptorTableCache.AssignHandles(HandlesSource, MaxHandlesPerCopy, Offset);

				RDescriptorHeap::CopyInto
				(
					HandlesDestination, HandlesSource, GetType(), MaxHandlesPerCopy
				);
			}

			UINT Left = NumEntries - Offset;

			if (Left)
			{
				for (UINT N = 0; N < Left; ++N)
				{
					HandlesSource[N] = Entries[N];
				}

				DescriptorTableCache.AssignHandles(HandlesSource, Left, Offset);

				RDescriptorHeap::CopyInto
				(
					HandlesDestination, HandlesSource, GetType(), Left
				);
			}
		}

		virtual void CopyDescriptorHeapEntries
		(
			const DescriptorHeapRange & DestinationRange,
			const DescriptorHeapEntry * Entries
		) override
		{
			Ensure
			(
				DestinationRange.DescriptorHeap.Get() == this
			);

			D3D12_CPU_DESCRIPTOR_HANDLE HandlesSource[MaxHandlesPerCopy];
			D3D12_CPU_DESCRIPTOR_HANDLE HandlesDestination[] = { DestinationRange };

			UINT Iterations = DestinationRange.Range / MaxHandlesPerCopy;
			UINT Offset = 0;

			for (UINT I = 0; I < Iterations; ++I, Offset += MaxHandlesPerCopy)
			{
				for (UINT N = 0; N < MaxHandlesPerCopy; ++N)
				{
					HandlesSource[N] = Entries[Offset + N];
				}

				DescriptorTableCache.AssignHandles(HandlesSource, MaxHandlesPerCopy, Offset);

				RDescriptorHeap::CopyInto
				(
					HandlesDestination, HandlesSource, GetType(), MaxHandlesPerCopy
				);
			}

			UINT Left = DestinationRange.Range - Offset;

			if (Left)
			{
				for (UINT N = 0; N < Left; ++N)
				{
					HandlesSource[N] = Entries[Offset + N];
				}

				DescriptorTableCache.AssignHandles(HandlesSource, Left, Offset);

				RDescriptorHeap::CopyInto
				(
					HandlesDestination, HandlesSource, GetType(), Left
				);
			}
		}
	};

	template
	<
		size_t Count
	>
	void CMutableDescriptorHeap::CopyInto(const RDescriptorHeap * pCPUDescriptorHeap, const UINT Size, const UINT OffsetDestination, const UINT * pOffsetSource, const UINT * pRangeSource)
	{
		Ensure
		(
			pCPUDescriptorHeap
		);

		Ensure
		(
			pCPUDescriptorHeap->GetType() == GetType()
		);

		Ensure
		(
			pCPUDescriptorHeap->IsCPUOnly()
		);

		Ensure
		(
			pRangeSource
		);

		Ensure
		(
			pOffsetSource
		);

		CD3DX12_CPU_DESCRIPTOR_HANDLE CPUHandleSource[Count];
		CD3DX12_CPU_DESCRIPTOR_HANDLE CPUHandleDestination(GetCPUAddress(OffsetDestination));

		const UINT DestinationRange = Size;

#pragma unroll
		for (UINT N = 0; N < Count; ++N)
		{
			CPUHandleSource[N] = CD3DX12_CPU_DESCRIPTOR_HANDLE(pCPUDescriptorHeap->GetCPUAddress(pOffsetSource[N]));
		};

		DEVICE->CopyDescriptors
		(
			1,
			&CPUHandleDestination,
			&DestinationRange,
			Count,
			CPUHandleSource,
			pRangeSource,
			GetType()
		);
	}

	template
	<
		size_t Count
	>
	UINT CMutableDescriptorHeap::Register(const UINT Size, const RDescriptorHeap * pCPUDescriptorHeap, const UINT * pOffsets, const UINT * pRanges, DescriptorHeapRange & Range)
	{
		const UINT Index = DescriptorTableCache.AssignRange(Size);

		if (Index == -1)
		{
			return -1;
		}

		CopyInto<Count>
		(
			pCPUDescriptorHeap,
			Size,
			Index,
			pOffsets,
			pRanges
		);

		Range = DescriptorHeapRange(this, Index, Size);

		return Index;
	}

	template
	<
		size_t Count
	>
	UINT CMutableDescriptorHeap::Register(const UINT Size, const RDescriptorHeap * pCPUDescriptorHeap, const UINT * pOffsets, const UINT * pRanges)
	{
		const UINT Index = DescriptorTableCache.AssignRange(Size);

		if (Index == -1)
		{
			return -1;
		}

		CopyInto<Count>
		(
			pCPUDescriptorHeap,
			Size,
			Index,
			pOffsets,
			pRanges
		);

		return Index;
	}
}