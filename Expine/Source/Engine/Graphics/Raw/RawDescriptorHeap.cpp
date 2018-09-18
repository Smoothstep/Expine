#include "Precompiled.h"

#include "Raw/RawDescriptorHeap.h"

namespace D3D
{
	ErrorCode RDescriptorHeap::Create
	(
		const D3D12_DESCRIPTOR_HEAP_TYPE	Type,
		const UINT							Count,
		const D3D12_DESCRIPTOR_HEAP_FLAGS	Flags
	)
	{
		DescriptorHeapDesc.Type				= Type;
		DescriptorHeapDesc.Flags			= Flags;
		DescriptorHeapDesc.NumDescriptors	= Count;
		DescriptorHeapDesc.NodeMask			= 0;

		HeapType		= Type;
		IsCPUOnlyHeap	= Flags == D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		OffsetMax		= Count;

		ErrorCode Error = DEVICE->CreateDescriptorHeap
		(
			&DescriptorHeapDesc, IID_PPV_ARGS(&DescriptorHeap)
		);

		if (Error)
		{
			return Error;
		}
			
		IncrementSize = DEVICE->GetDescriptorHandleIncrementSize(Type);
			
		return S_OK;
	}
}
