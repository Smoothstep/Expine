#include "Precompiled.h"

#include "Command/CommandList.h"
#include "Buffer/Buffer.h"
#include "Resource/ResourceAllocator.h"

namespace D3D
{
	ErrorCode CCommandListBase::Create()
	{
		switch (GetType())
		{
			case D3D12_COMMAND_LIST_TYPE_DIRECT:
			{
				CommandQueue = CCommandQueueDirect::Instance_Pointer();
			}

			break;

			case D3D12_COMMAND_LIST_TYPE_COMPUTE:
			{
				CommandQueue = CCommandQueueCompute::Instance_Pointer();
			}

			break;

			case D3D12_COMMAND_LIST_TYPE_COPY:
			{
				CommandQueue = CCommandQueueCopy::Instance_Pointer();
			}

			break;
		}

		return CommandQueue == NULL ? E_FAIL : S_OK;
	}

	CCommandListBase::CCommandListBase()
	{
		ResourceAllocatorCPU = new CResourceAllocator(PageGenericRead);
		ResourceAllocatorGPU = new CResourceAllocator(PageUnorderedAccess);
	}

	void CCommandListBase::AddFrameResource(RResource * pResource)
	{
		FrameResources.push_back(pResource);
	}

	void CCommandListBase::AddFrameResource(const SharedPointer<RResource>& pResource)
	{
		FrameResources.push_back(pResource);
	}

	void CCommandListBase::AddFrameResource(const CGrpBuffer * pBuffer)
	{
		FrameResources.push_back(pBuffer->Buffer);
	}

	void CCommandListBase::CopyDataToBuffer(RResource * pBuffer, const void * pData, const UINT Size) const
	{
		ResourceEntry Entry = ResourceAllocatorCPU->Allocate(Size, 512);

		CopyMemory
		(
			Entry.ResourceVA.CPUAddress.Pointer,
			Entry.ResourceOffset,
			pData,
			0,
			Size
		);

		CopyBufferRegion(pBuffer, 0, Entry.Resource.Get(), Entry.ResourceOffset, Size);
	}

	void CCommandListBase::CopyDataToTexture(RResource * pTexture, D3D12_SUBRESOURCE_DATA * pData, const UINT FirstSubResource, const UINT NumSubResources) const
	{
		UINT64 Size = GetRequiredIntermediateSize
		(
			pTexture->Get(), 
			FirstSubResource, 
			NumSubResources
		);

		ResourceEntry Entry = ResourceAllocatorCPU->Allocate(Size, 512);

		UpdateSubresources
		(
			RGrpCommandList::Get(),
			pTexture->Get(),
			Entry.Resource->Get(),
			Entry.ResourceOffset,
			FirstSubResource,
			NumSubResources,
			pData
		);

		pTexture->SetResourceState(D3D12_RESOURCE_STATE_COPY_DEST, *this);
	}

	DescriptorHeapRange CCommandListBase::OccupyViewDescriptorRange(const RRootSignature & RootSignature)
	{
		const UINT Size = RootSignature.GetDescriptorRangeViews();
		{
			return OccupyViewDescriptorRange(Size);
		}
	}

	DescriptorHeapRange CCommandListBase::OccupyViewDescriptorRangeDynamic(const UINT Range)
	{
		DescriptorHeapRange DHRange = DescriptorTableReferrerViews.back().RequestRange(Range);

		if (!DHRange.Valid())
		{
			UINT NumEntries = Math::Max(Range, DescriptorEntriesPerDynamicRange);

			return DescriptorTableReferrerViews.emplace(DescriptorTableReferrerViews.end(),
				DescriptorHeapViews, 
				NumEntries,
				OccupyViewDescriptorRange(NumEntries).Offset)->RequestRange(Range);
		}

		return DHRange;
	}

	void CCommandListBase::FreeDescriptorRange(const DescriptorHeapRange & Range)
	{
		CMutableDescriptorHeap * Heap = static_cast<CMutableDescriptorHeap*>(Range.DescriptorHeap.Get());
		{
			Heap->UnregisterRange(Range.Offset);
		}
	}

	DescriptorHeapRange CCommandListBase::OccupySamplerDescriptorRange(const RRootSignature & RootSignature)
	{
		const UINT Size = RootSignature.GetDescriptorRangeViews();
		{
			return OccupySamplerDescriptorRange(Size);
		}
	}

	void CCommandListBase::SetDescriptorHeapTableStaticRange(const UINT Ident, const DescriptorHeapRange & Range)
	{
		if (Range.Valid())
		{
			DescriptorTableStaticsMap.insert(std::make_pair(Ident, Range));
		}
	}

	void CCommandListBase::CopyDescriptorHeapEntries(const DescriptorHeapRange & Destination, const DescriptorHeapEntry * Entries, const UINT NumEntries)
	{
		const CMutableDescriptorHeap * Heap = static_cast<const CMutableDescriptorHeap*>(Destination.DescriptorHeap.Get());
	}

	DescriptorHeapRange CCommandListBase::OccupyViewDescriptorRange(const UINT Size)
	{
		Ensure(Size);
		Ensure(Size <= DescriptorEntriesPerHeap);

		UINT Offset;

		if ((Offset = DescriptorHeapViews->RegisterRange(Size)) == -1)
		{
			UINT NewSize = Math::Max(
				DescriptorHeapViews->GetDescriptorsCount() + Size,
				DescriptorHeapViews->GetDescriptorsCount() + NumDescriptorsPerHeap);

			CGPUDescriptorHeapManager::Instance().RequestDescriptorHeapCopy(
				DescriptorHeapViews, 
				NewSize,
				FenceValue
			);

			Offset = DescriptorHeapViews->RegisterRange(Size);
		}

		return DescriptorHeapRange(DescriptorHeapViews, Offset, Size);
	}

	DescriptorHeapRange CCommandListBase::OccupySamplerDescriptorRange(const UINT Size)
	{
		Ensure(Size);
		Ensure(Size <= DescriptorEntriesPerHeap);

		UINT Offset;

		if ((Offset = DescriptorHeapSamplers->RegisterRange(Size)) == -1)
		{
			UINT NewSize = Math::Max(
				DescriptorHeapViews->GetDescriptorsCount() + Size,
				DescriptorHeapViews->GetDescriptorsCount() + NumDescriptorsPerHeap);

			CGPUDescriptorHeapManager::Instance().RequestDescriptorHeapCopy(
				DescriptorHeapSamplers,
				NewSize,
				FenceValue
			);

			Offset = DescriptorHeapSamplers->RegisterRange(Size);
		}

		return DescriptorHeapRange(DescriptorHeapSamplers, Offset, Size);
	}

	void CCommandListContext::OnComplete(const UINT64 Value)
	{}

	CCommandListContext::CCommandListContext(CMutableDescriptorHeap * pDescriptorHeapViews, CMutableDescriptorHeap * pDescriptorHeapSamplers)
	{
		DescriptorHeapViews = pDescriptorHeapViews;
		DescriptorHeapSamplers = pDescriptorHeapSamplers;
	}

	ErrorCode CCommandListContext::Reset(const UINT Frame, const SharedPointer<RPipelineState>& pPipelineState)
	{
		DescriptorTableReferrerViews.front().FreeAll();

		if (DescriptorTableReferrerViews.size() > 1)
		{
			for (size_t N = DescriptorTableReferrerViews.size(); N > 0; --N)
			{
				CMutableDescriptorHeap * Heap = static_cast<CMutableDescriptorHeap*>(DescriptorTableReferrerViews[N].GetHeap());
				{
					Heap->UnregisterRange(DescriptorTableReferrerViews[N].GetOffset());
				}
			}

			DescriptorTableReferrerViews.erase
			(
				DescriptorTableReferrerViews.begin() + 1, 
				DescriptorTableReferrerViews.end()
			);
		}

		DescriptorTableReferrerSamplers.front().FreeAll();

		if (DescriptorTableReferrerSamplers.size() > 1)
		{
			for (size_t N = DescriptorTableReferrerSamplers.size(); N > 0; --N)
			{
				CMutableDescriptorHeap * Heap = static_cast<CMutableDescriptorHeap*>(DescriptorTableReferrerSamplers[N].GetHeap());
				{
					Heap->UnregisterRange(DescriptorTableReferrerSamplers[N].GetOffset());
				}
			}

			DescriptorTableReferrerSamplers.erase
			(
				DescriptorTableReferrerSamplers.begin() + 1,
				DescriptorTableReferrerSamplers.end()
			);
		}

		return RGrpCommandList::Reset(pPipelineState, CommandAllocator[Frame]);
	}

	void CCommandListContext::Finish(const UINT Frame, const BOOL WaitForCompletion)
	{
		CommandQueue->ExecuteCommandList(*this);

		FenceValue = CommandQueue->IncrementFence(Frame);

		ResourceAllocatorCPU->DiscardPages(FenceValue);
		ResourceAllocatorGPU->DiscardPages(FenceValue);

		if (WaitForCompletion)
		{
			this->WaitForCompletion();
		}

		FrameResources.clear();
	}

	void CCommandListContext::WaitForCompletion()
	{
		CommandQueue->WaitForFence(FenceValue);
	}

	ErrorCode CCommandListContext::Create(bool Recording, D3D12_COMMAND_LIST_TYPE Type)
	{
		ErrorCode Error;

		for (UINT N = 0; N < FRAME_COUNT; ++N)
		{
			CommandAllocator[N] = new RCommandAllocator();

			if ((Error = CommandAllocator[N]->Create(Type)))
			{
				return Error;
			}
		}

		if (Recording)
		{
			Error = RGrpCommandList::CreateRecording(CommandAllocator[0]);
		}
		else
		{
			Error = RGrpCommandList::Create(CommandAllocator[0]);
		}

		if (Error)
		{
			return Error;
		}

		if ((Error = CCommandListBase::Create()))
		{
			return Error;
		}

		if (Type == D3D12_COMMAND_LIST_TYPE_COPY)
		{
			return S_OK;
		}

		if (!DescriptorHeapViews)
		{
			try
			{
				DescriptorHeapViews = CGPUDescriptorHeapManager::Instance().RequestDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			}
			catch (ErrorCode EC)
			{
				return EC;
			}
		}

		if (!DescriptorHeapSamplers)
		{
			try
			{
				DescriptorHeapSamplers = CGPUDescriptorHeapManager::Instance().RequestDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
			}
			catch (ErrorCode EC)
			{
				return EC;
			}
		}

		if (Recording)
		{
			SetDescriptorHeaps();
		}

		return S_OK;
	}

	void CCommandListContext::SetDescriptorHeaps()
	{
		UINT NumHeaps = 0;
		IDescriptorHeap * Heaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

		if (DescriptorHeapViews)
		{
			Heaps[NumHeaps++] = DescriptorHeapViews->Get();
		}

		if (DescriptorHeapSamplers)
		{
			Heaps[NumHeaps++] = DescriptorHeapSamplers->Get();
		}

		RGrpCommandList::Get()->SetDescriptorHeaps(NumHeaps - 1, Heaps);
	}
}
