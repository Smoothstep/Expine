#pragma once

#include "RawResource.h"
#include "RawDescriptorHeap.h"

namespace D3D
{
	class RUnorderedAccessView
	{
	private:

		SharedPointer<RResource> Resource;

	private:

		DescriptorHeapEntry DescriptorHeap;

	public:

		inline RResource * GetResource() const
		{
			return Resource.Get();
		}

		inline const DescriptorHeapEntry & GetDescriptorHeapEntry() const
		{
			return DescriptorHeap;
		}

		inline void SetAsUnorderedAccessView
		(
			const RGrpCommandList & CmdList,
			const UINT				ParameterIndex = 0
		)
		{
			CmdList->SetGraphicsRootUnorderedAccessView(ParameterIndex, Resource.GetRef()->GetGPUVirtualAddress());
		}

		inline void SetAsUnorderedAccessViewCompute
		(
			const RGrpCommandList & CmdList,
			const UINT				ParameterIndex = 0
		)
		{
			CmdList->SetComputeRootUnorderedAccessView(ParameterIndex, Resource.GetRef()->GetGPUVirtualAddress());
		}

		inline void SetRWBarrier
		(
			const RGrpCommandList & CmdList
		)
		{
			CmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(Resource.GetRef()));
		}

	public:

		inline RUnorderedAccessView() = default;
		inline RUnorderedAccessView
		(
			const SharedPointer<RResource> & pResource
		)
		{
			Resource = pResource;
		}

	public:

		ErrorCode CreateFromResource
		(
			const SharedPointer<RResource>	&	pResource,
			const DescriptorHeapEntry		&	HeapEntry,
			const DXGI_FORMAT					Format		= DXGI_FORMAT_UNKNOWN,
			const D3D12_BUFFER_UAV			*	pBufferDesc = 0
		);
	};

	template
	<
		size_t Count
	> 
	struct UnorderedAccessViewGroup
	{
		SharedPointer<RDescriptorHeap> DescriptorHeap;
		SharedPointer<RUnorderedAccessView> UnorderedAccessViews[Count];

		RResource * ResourceObjects[Count];

		ErrorCode Initialize
		(
			const SharedPointer<RResource>			ppResources[Count],
			const DXGI_FORMAT						Formats[Count],
			const D3D12_DESCRIPTOR_HEAP_FLAGS		Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
		);

		ErrorCode Initialize
		(
			RResource							*	ppResources[Count],
			const DXGI_FORMAT						Formats[Count],
			const D3D12_DESCRIPTOR_HEAP_FLAGS		Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
		);

		ErrorCode Initialize
		(
			const SharedPointer<RResource>			ppResources[Count],
			const DXGI_FORMAT						Formats[Count],
			const SharedPointer<RDescriptorHeap> &	pDescriptorHeap,
			const UINT								Offset = 0
		);

		ErrorCode Initialize
		(
			RResource							 *	ppResources[Count],
			const DXGI_FORMAT						Formats[Count],
			const SharedPointer<RDescriptorHeap> &	pDescriptorHeap,
			const UINT								Offset = 0
		);

		ErrorCode Initialize
		(
			RResource							 *	pResource,
			const D3D12_BUFFER_UAV				 *	pBufferDesc = 0,
			const UINT								Offset = 0
		);

		ErrorCode Initialize
		(
			RResource							 *	pResource,
			const DXGI_FORMAT						Format,
			const UINT								Offset = 0
		);

		ErrorCode InitializeHeap
		(
			const D3D12_DESCRIPTOR_HEAP_FLAGS		Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
		);

		inline const SharedPointer<RUnorderedAccessView> & operator[]
		(
			const UINT N
		)	const
		{
			return UnorderedAccessViews[N];
		}
	};

	template
	<
		size_t Count
	>
	ErrorCode UnorderedAccessViewGroup<Count>::Initialize
	(
		const SharedPointer<RResource>		ppResources[Count],
		const DXGI_FORMAT					Formats[Count],
		const D3D12_DESCRIPTOR_HEAP_FLAGS	Flags
	)
	{
		ErrorCode Error;

		if (DescriptorHeap == NULL)
		{
			DescriptorHeap = new RDescriptorHeap();

			if ((Error = DescriptorHeap->Create_CBV_SRV_UAV(Count, Flags)))
			{
				return Error;
			}
		}

		for (UINT Index = 0; Index < Count; ++Index)
		{
			if (ppResources[Index])
			{
				UnorderedAccessViews[Index] = new RUnorderedAccessView();

				if ((Error = UnorderedAccessViews[Index]->CreateFromResource(ppResources[Index], DescriptorHeapEntry(DescriptorHeap.Get(), Index))))
				{
					return Error;
				}

				ResourceObjects[Index] = ppResources[Index].Get();
			}
		}

		return S_OK;
	}

	template
	<
		size_t Count
	>
	inline ErrorCode UnorderedAccessViewGroup<Count>::Initialize
	(
				RResource					*	ppResources[Count],
		const	DXGI_FORMAT						Formats[Count],
		const	D3D12_DESCRIPTOR_HEAP_FLAGS		Flags
	)
	{
		ErrorCode Error;

		if (DescriptorHeap == NULL)
		{
			DescriptorHeap = new RDescriptorHeap();

			if ((Error = DescriptorHeap->Create_CBV_SRV_UAV(Count, Flags)))
			{
				return Error;
			}
		}

		for (UINT Index = 0; Index < Count; ++Index)
		{
			if (ppResources[Index])
			{
				UnorderedAccessViews[Index] = new RUnorderedAccessView();

				if ((Error = UnorderedAccessViews[Index]->CreateFromResource(ppResources[Index], DescriptorHeapEntry(DescriptorHeap.Get(), Index), Formats[Index])))
				{
					return Error;
				}

				ResourceObjects[Index] = ppResources[Index];
			}
		}

		return S_OK;
	}

	template
	<
		size_t Count
	>
	ErrorCode UnorderedAccessViewGroup<Count>::Initialize
	(
		const SharedPointer<RResource>			ppResources[Count],
		const DXGI_FORMAT						Formats[Count],
		const SharedPointer<RDescriptorHeap> &	pDescriptorHeap,
		const UINT								Offset
	)
	{
		CHECK_NULL_ARG(DescriptorHeap = pDescriptorHeap);

		ErrorCode Error;

		for (UINT Index = 0; Index < Count; ++Index)
		{
			if (ppResources[Index])
			{
				UnorderedAccessViews[Index] = new RUnorderedAccessView();

				if ((Error = UnorderedAccessViews[Index]->CreateFromResource(ppResources[Index], DescriptorHeapEntry(pDescriptorHeap.Get(), Offset + Index), Formats[Index])))
				{
					return Error;
				}

				ResourceObjects[Index] = ppResources[Index].Get();
			}
		}

		return S_OK;
	}

	template
	<
		size_t Count
	>
	ErrorCode UnorderedAccessViewGroup<Count>::Initialize
	(
		RResource							 *	ppResources[Count],
		const DXGI_FORMAT						Formats[Count],
		const SharedPointer<RDescriptorHeap> &	pDescriptorHeap,
		const UINT								Offset
	)
	{
		CHECK_NULL_ARG(DescriptorHeap = pDescriptorHeap);

		ErrorCode Error;

		for (UINT Index = 0; Index < Count; ++Index)
		{
			if (ppResources[Index])
			{
				UnorderedAccessViews[Index] = new RUnorderedAccessView();

				if ((Error = UnorderedAccessViews[Index]->CreateFromResource(ppResources[Index], DescriptorHeapEntry(pDescriptorHeap.Get(), Offset + Index), Formats[Index])))
				{
					return Error;
				}

				ResourceObjects[Index] = ppResources[Index].Get();
			}
		}

		return S_OK;
	}

	template
	<
		size_t Count
	>
	ErrorCode UnorderedAccessViewGroup<Count>::Initialize
	(
		RResource							*	pResource,
		const D3D12_BUFFER_UAV				*	pBufferDesc,
		const UINT								Offset
	)
	{
		ErrorCode Error;

		if (DescriptorHeap == NULL)
		{
			DescriptorHeap = new RDescriptorHeap();

			if ((Error = DescriptorHeap->Create_CBV_SRV_UAV(Count, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)))
			{
				return Error;
			}
		}

		if (pResource)
		{
			UnorderedAccessViews[Offset] = new RUnorderedAccessView();

			if ((Error = UnorderedAccessViews[Offset]->CreateFromResource(pResource, DescriptorHeapEntry(DescriptorHeap.Get(), Offset), DXGI_FORMAT_UNKNOWN, pBufferDesc)))
			{
				return Error;
			}

			ResourceObjects[Offset] = pResource;
		}

		return S_OK;
	}

	template
	<
		size_t Count
	>
	ErrorCode UnorderedAccessViewGroup<Count>::Initialize
	(
		RResource							*	pResource,
		const DXGI_FORMAT						Format,
		const UINT								Offset
	)
	{
		ErrorCode Error;

		if (DescriptorHeap == NULL)
		{
			DescriptorHeap = new RDescriptorHeap();

			if ((Error = DescriptorHeap->Create_CBV_SRV_UAV(Count, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)))
			{
				return Error;
			}
		}

		if (pResource)
		{
			UnorderedAccessViews[Offset] = new RUnorderedAccessView();

			if ((Error = UnorderedAccessViews[Offset]->CreateFromResource(pResource, DescriptorHeapEntry(DescriptorHeap.Get(), Offset), Format, 0)))
			{
				return Error;
			}

			ResourceObjects[Offset] = pResource;
		}

		return S_OK;
	}

	template
	<
		size_t Count
	>
	ErrorCode UnorderedAccessViewGroup<Count>::InitializeHeap
	(
		const D3D12_DESCRIPTOR_HEAP_FLAGS		Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
	)
	{
		ErrorCode Error;

		if (DescriptorHeap == NULL)
		{
			DescriptorHeap = new RDescriptorHeap();

			if ((Error = DescriptorHeap->Create_CBV_SRV_UAV(Count, Flags)))
			{
				return Error;
			}
		}

		return S_OK;
	}

	inline void RGrpCommandList::SetUnorderedAccessView(const UINT RootParameter, const RUnorderedAccessView & UnorderedAccessView) const
	{
		CommandList->SetComputeRootUnorderedAccessView(RootParameter, UnorderedAccessView.GetResource()->GetGPUAddress());
	}

	inline void RGrpCommandList::SetUnorderedAccessViewCompute
	(
		const UINT						RootParameter,
		const RUnorderedAccessView	&	UnorderedAccessView
	)	const
	{
		CommandList->SetComputeRootUnorderedAccessView(RootParameter, UnorderedAccessView.GetResource()->GetGPUAddress());
	}
}