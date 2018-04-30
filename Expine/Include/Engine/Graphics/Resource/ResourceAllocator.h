#pragma once

#include "Resource.h"
#include "CommandQueue.h"

namespace D3D
{
	enum PageTypes
	{
		PageUnorderedAccess,
		PageGenericRead,
		NumPageTypes
	};

	class CResourceAllocator;
	class CResourceAllocatorPage
	{
	private:

		SharedPointer<RResource> Buffer;

	private:

		VirtualResourceAddress VirtualAddresses;

	public:

		inline const SharedPointer<RResource> & GetBuffer() const
		{
			return Buffer;
		}

		inline const VirtualResourceAddress & GetVirtualAddresses() const
		{
			return VirtualAddresses;
		}

	public:

		ErrorCode CreatePage
		(
			const PageTypes Type,
			const UINT64	Size
		);
	};

	class CResourceAllocatorManager : public CSingleton<CResourceAllocatorManager>
	{
		friend class CResourceAllocator;

	protected:

		TQueue<TPair<UINT64, CResourceAllocatorPage*> >					PagesRetired[NumPageTypes];
		TQueue<TPair<UINT64, SharedPointer<CResourceAllocatorPage> > >	PagesUnused[NumPageTypes];
		TQueue<CResourceAllocatorPage*>									PagesAvailable[NumPageTypes];
		TVector<UniquePointer<CResourceAllocatorPage> >					Pages[NumPageTypes];

	protected:

		TMutex Mutex;

		void ReleaseUnusedPages
		(
			const PageTypes Type,
			const UINT64	FenceValue
		);

	public:

		CResourceAllocatorPage * RequestPage
		(
			const PageTypes Type,
			const UINT64	Size
		);
	};

	class CResourceAllocator
	{
	private:

		TVector<ConstPointer<CResourceAllocatorPage> >	PagesRetired;
		TVector<ConstPointer<CResourceAllocatorPage> >	PagesOversized;

	private:

		ConstPointer<CResourceAllocatorPage> CurrentPage;

	private:

		PageTypes	Type;

		UINT64		PageOffset	= 0;
		UINT64		PageSize	= 0;

	public:

		CResourceAllocator
		(
			PageTypes PageType
		);

		void DiscardPages
		(
			const UINT64 FenceValue
		);

		ResourceEntry AllocateOversized
		(
			const UINT64 Size
		);

		ResourceEntry Allocate
		(
			const UINT64 Size,
			const UINT64 Alignment
		);
	};
}