#include "Precompiled.h"

#include "Descriptor/DescriptorHeap.h"

namespace D3D
{
	CDescriptorHeapAssignments::~CDescriptorHeapAssignments()
	{}

	void CDescriptorHeapAssignments::Initialize(const UINT Size)
	{
		Ensure(Size);

		IndexEnd = Size;
		Ranges.resize(Size);
		Handles.resize(Size);

		Clear();
	}

	void CDescriptorHeapAssignments::Clear()
	{
		Ranges.Fill();
		Handles.Fill();

		Occupied = 0;
		IndexStart = 0;
	}

	void CDescriptorHeapAssignments::DissociateRange(const UINT Index)
	{
		if (IndexStart > Ranges[Index].Last)
		{
			IndexStart = Ranges[Index].Last;
		}

		Occupied -= Ranges[Index].Range;

		Ranges[Ranges[Index].Last].Next = Ranges[Index].Next;
		Ranges[Ranges[Index].Next].Last = Ranges[Index].Last;

		Ranges[Index].Range = 0;
		Ranges[Index].Next = 0;
		Ranges[Index].Last = 0;
	}

	UINT CDescriptorHeapAssignments::AssignRange(const UINT Size)
	{
		Ensure(Size);

		UINT Curr = IndexStart;
		UINT Last = 0;
		UINT Next;

		bool FlagInBetween = false;

		while (Curr < IndexEnd - Size)
		{
			UINT M = Ranges[Curr].Range;

			if (Ranges[Curr + M].Range == 0)
			{
				int N = Ranges[Curr].Next - M - Curr;

				if (N > Size)
				{
					Last = Curr;
					Curr += M;

					Next = Ranges[Last].Next;

					Ranges[Next].Last = Curr;
					Ranges[Curr].Next = Ranges[Next].Next;
					Ranges[Curr].Range = Size;
					Ranges[Curr].Last = Last;
					Ranges[Last].Next = Curr;
					

					if (IndexStart > Curr)
					{
						IndexStart = Curr;
					}

					Occupied += Size;

					return Curr;
				}

				if (Ranges[Curr].Next)
				{
					FlagInBetween = true;
				}
			}

			Last = Curr;
			Curr = Ranges[Curr].Next;

			if (Curr == 0)
			{
				Curr = Last + Ranges[Last].Range;

				if (!FlagInBetween)
				{
					IndexStart = Curr;
				}

				Ranges[Curr].Range = Size;
				Ranges[Curr].Last = Last;
				Ranges[Curr].Next = Ranges[Last].Next;
				Ranges[Last].Next = Curr;

				Occupied += Size;

				return Curr;
			}
		}

		return -1;
	}

	void CDescriptorHeapAssignments::AssignHandles(const D3D12_CPU_DESCRIPTOR_HANDLE Handles[], const UINT NumHandles, const UINT Offset)
	{
		CopyMemory(&this->Handles[Offset], Handles, NumHandles);
	}

	void CDescriptorHeapAssignments::AssignHandle(const D3D12_CPU_DESCRIPTOR_HANDLE Handle, const UINT Offset)
	{
		this->Handles[Offset] = Handle;
	}

	void CDescriptorHeapAssignments::AssignHandles(const DescriptorHeapRange & Range, const UINT Offset)
	{
		for (UINT N = 0; N < Range.Range; ++N)
		{
			this->Handles[Offset + N] = Range[N];
		}
	}

	CDescriptorHeap::CDescriptorHeap()
	{}

	CDescriptorHeap::~CDescriptorHeap()
	{}

	ErrorCode CDescriptorHeap::Create(const D3D12_DESCRIPTOR_HEAP_TYPE Type, const UINT Size, const D3D12_DESCRIPTOR_HEAP_FLAGS Flags)
	{
		DescriptorTableCache.Initialize(Size);
		{
			return RDescriptorHeap::Create(Type, Size, Flags);
		}
	}
	
	CDescriptorHeapTableDistributor::CDescriptorHeapTableDistributor(RDescriptorHeap * Heap, UINT Size, UINT Offset) :
		Size(Size)
		, Begin(0)
		, End(0)
		, Offset(Offset)
		, Heap(Heap)
	{
		Ranges = new DescriptorHeapRange[Size]();

		for (size_t N = 0; N < Size; ++N)
		{
			Ranges[N].DescriptorHeap = Heap;
		}
	}
	
	CDescriptorHeapTableDistributor::~CDescriptorHeapTableDistributor()
	{
		if (Ranges)
		{
			SafeReleaseArray(Ranges);
		}
	}
	
	DescriptorHeapRange CDescriptorHeapTableDistributor::RequestRange(UINT Range)
	{
		Ensure(Range);

		if (Ranges[End].Offset >= Ranges[Begin].Offset)
		{
			if (Range >= Size - End)
			{
				if (Range > Begin - Offset)
				{
					return RangeInvalid;
				}

				Begin -= Range;

				Ranges[Begin].Range = Range;
				Ranges[Begin].Offset = Begin + Offset;

				return Ranges[Begin];
			}
			else
			{
				End += Ranges[End].Range;

				Ranges[End].Range = Range;
				Ranges[End].Offset = End + Offset;

				return Ranges[End];
			}
		}
		else
		{
			if (Range > Size - Begin)
			{
				if (Range >= End - Offset)
				{
					return RangeInvalid;
				}

				End = Ranges[End].Offset + Range - Offset;

				Ranges[End].Range = Range;
				Ranges[End].Offset = End + Offset;

				return Ranges[End];
			}
			else
			{
				Begin -= Range;

				Ranges[Begin].Range = Range;
				Ranges[Begin].Offset = Begin + Offset;

				return Ranges[Begin];
			}
		}
	}
	
	void CDescriptorHeapTableDistributor::FreeAll()
	{
		Begin	= 0;
		End		= 0;
	}

	void CDescriptorHeapTableDistributor::FreeFront()
	{
		Begin += Ranges[Begin].Range;
	}

	void CDescriptorHeapTableDistributor::FreeFront(UINT Num)
	{
		while (Num-- > 0)
		{
			Begin = Ranges[Begin + Ranges[Begin].Range].Offset;
		}
	}
	
	void CDescriptorHeapTableDistributor::FreeBack()
	{
		End -= Ranges[End].Range;
	}
	
	void CDescriptorHeapTableDistributor::FreeBack(UINT Num)
	{
		while (Num-- > 0)
		{
			End = Ranges[End - Ranges[End].Range].Offset;
		}
	}
}
