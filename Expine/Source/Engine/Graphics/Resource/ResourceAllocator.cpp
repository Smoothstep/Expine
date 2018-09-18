#include "Precompiled.h"
#include "Resource/ResourceAllocator.h"

namespace D3D
{
	static CResourceAllocatorManager G_Manager;

	static constexpr UINT DefaultPageSize = 1024 * 1024 * 32;

	void CResourceAllocatorManager::ReleaseUnusedPages(const PageTypes Type, const UINT64 FenceValue)
	{
		while (!PagesUnused[Type].empty() && CCommandQueueDirect::Instance().FenceCompleted(PagesUnused[Type].front().first))
		{
			PagesUnused[Type].pop();
		}
	}

	CResourceAllocatorPage * CResourceAllocatorManager::RequestPage(const PageTypes Type, const UINT64 Size)
	{
		std::scoped_lock<TMutex> Lock(Mutex);

		while (!PagesRetired[Type].empty() && CCommandQueueDirect::Instance().FenceCompleted(PagesRetired[Type].front().first))
		{
			PagesAvailable[Type].push(PagesRetired[Type].front().second); PagesRetired[Type].pop();
		}

		if (!PagesAvailable[Type].empty())
		{
			CResourceAllocatorPage * Page = PagesAvailable[Type].front();
			{
				PagesAvailable[Type].pop();
			}

			return Page;
		}

		CResourceAllocatorPage * Page = new CResourceAllocatorPage();

		ThrowOnError
		(
			Page->CreatePage(Type, Size)
		);

		Pages[Type].push_back(Page);

		return Page;
	}

	ErrorCode CResourceAllocatorPage::CreatePage(const PageTypes Type, const UINT64 Size)
	{
		ErrorCode Error;

		Buffer = new RResource();

		if ((Error = Buffer->Create(RResource::InitializeOptions::Buffer(
			Size,
			Type == PageUnorderedAccess ?	D3D12_HEAP_TYPE_DEFAULT						: D3D12_HEAP_TYPE_UPLOAD,
											D3D12_HEAP_FLAG_NONE,
			Type == PageUnorderedAccess ?	D3D12_RESOURCE_STATE_UNORDERED_ACCESS		: D3D12_RESOURCE_STATE_GENERIC_READ,
			Type == PageUnorderedAccess ?	D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS	: D3D12_RESOURCE_FLAG_NONE))))
		{
			return Error;
		}

		VirtualAddresses.GPUAddress.Address = Buffer->GetGPUAddress();

		if ((Error = Buffer->Map(0, 0, 0, reinterpret_cast<void**>(&VirtualAddresses.CPUAddress))))
		{
			return Error;
		}

		Buffer->Get()->SetName(L"Allocator Page");

		return S_OK;
	}

	CResourceAllocator::CResourceAllocator(PageTypes PageType)
	{
		Type = PageType;
		PageSize = DefaultPageSize;
	}

	void CResourceAllocator::DiscardPages(const UINT64 FenceValue)
	{
		std::scoped_lock<TMutex> Lock(CResourceAllocatorManager::Instance().Mutex);

		for (const auto & Iter : PagesRetired)
		{
			CResourceAllocatorManager::Instance().PagesRetired[Type].push(std::make_pair(FenceValue, Iter));
		}

		CResourceAllocatorManager::Instance().ReleaseUnusedPages(Type, FenceValue);

		for (const auto & Iter : PagesOversized)
		{
			CResourceAllocatorManager::Instance().PagesUnused[Type].push(std::make_pair(FenceValue, Iter));
		}

		if (CurrentPage)
		{
			CResourceAllocatorManager::Instance().PagesRetired[Type].push(std::make_pair(FenceValue, CurrentPage));
		}

		PagesRetired.clear();
		PagesOversized.clear();
	}

	ResourceEntry CResourceAllocator::AllocateOversized(const UINT64 Size)
	{
		CResourceAllocatorPage * Page = CResourceAllocatorManager::Instance().RequestPage(Type, Size);
		{
			PagesOversized.push_back(Page);
		}

		return ResourceEntry
		(
			Page->GetBuffer(), 
			0,
			Page->GetVirtualAddresses()
		);
	}

	ResourceEntry CResourceAllocator::Allocate(const UINT64 Size, const UINT64 Alignment)
	{
		const UINT AlignedSize = MakeBufferSizeAlign(Size, Alignment);

		if (AlignedSize > PageSize)
		{
			return AllocateOversized(AlignedSize);
		}

		// Can we even allocate more on the current page?

		if (AlignedSize > PageSize - PageOffset)
		{
			PagesRetired.push_back(CurrentPage);
			{
				CurrentPage = NULL;
			}
		}

		if (CurrentPage == NULL)
		{
			CurrentPage = CResourceAllocatorManager::Instance().RequestPage(Type, DefaultPageSize);
			{
				PageOffset = 0;
			}
		}

		ResourceEntry Entry
		(
			CurrentPage->GetBuffer(), 
			PageOffset, 
			CurrentPage->GetVirtualAddresses()
		);
		
		PageOffset += AlignedSize;

		return Entry;
	}
}