#pragma once

#include "Raw/RawDevice.h"

namespace D3D
{
	class RHeap
	{
	private:

		ComPointer<IHeap> Heap;

	public:

		struct InitializeOptions : public D3D12_HEAP_DESC
		{
			inline InitializeOptions
			(
				const UINT64					ByteSize,
				const UINT64					ByteAlignment			= 512,
				const D3D12_HEAP_FLAGS			Flags					= D3D12_HEAP_FLAG_NONE,
				const D3D12_HEAP_TYPE			Type					= D3D12_HEAP_TYPE_DEFAULT,
				const D3D12_CPU_PAGE_PROPERTY	CPUPageProperty			= D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
				const UINT						CreationNodeMask		= 0,
				const UINT						VisibleNodeMask			= 0,
				const D3D12_MEMORY_POOL			MemoryPoolPreference	= D3D12_MEMORY_POOL_UNKNOWN

			)
			{
				this->SizeInBytes						= ByteSize;
				this->Alignment							= ByteAlignment;
				this->Flags								= Flags;
				this->Properties.CPUPageProperty		= CPUPageProperty;
				this->Properties.CreationNodeMask		= CreationNodeMask;
				this->Properties.VisibleNodeMask		= VisibleNodeMask;
				this->Properties.MemoryPoolPreference	= MemoryPoolPreference;
				this->Properties.Type					= Type;
			}
		};

		inline operator IHeap*()
		{
			return Heap.Get();
		}

		inline IHeap * operator()()
		{
			return Heap.Get();
		}

	public:

		ErrorCode Create
		(
			const InitializeOptions & InitializeOptions
		)
		{
			ErrorCode Error;

			if ((Error = DEVICE->CreateHeap(&InitializeOptions, IID_PPV_ARGS(&Heap))))
			{
				return Error;
			}

			return S_OK;
		}

		inline UINT64 GetByteSize() const
		{
			return Heap->GetDesc().SizeInBytes;
		}

		inline UINT64 GetByteAlignment() const
		{
			return Heap->GetDesc().Alignment;;
		}

		inline D3D12_HEAP_PROPERTIES GetProperties() const
		{
			return Heap->GetDesc().Properties;
		}
	};

	struct HeapEntry
	{
		RHeap* Heap;
		UINT64 HeapOffset;

		inline HeapEntry
		(
					RHeap* Heap,
			const	UINT64 HeapOffset = 0
		) :
			Heap(Heap), HeapOffset(HeapOffset)
		{
			Ensure(Heap);
		}

		inline bool Valid() const
		{
			return Heap && Heap->GetByteSize() < HeapOffset && (HeapOffset % Heap->GetByteAlignment() == 0);
		}
	};
}