#pragma once

#include "DescriptorHeap.h"

namespace D3D
{
	static inline constexpr UINT DescriptorHeapSizeForType
	(
		const D3D12_DESCRIPTOR_HEAP_TYPE Type
	)
	{
		return Type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER ? 16 : 64;
	}

	class CDescriptorHeapPool 
	{
	protected:

		D3D12_DESCRIPTOR_HEAP_TYPE Type;

	public:

		CDescriptorHeapPool
		(
			const D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapType
		)
		{
			Type = DescriptorHeapType;
		}
	};

	class CCPUDescriptorHeapManager : public CSingleton<CCPUDescriptorHeapManager>
	{
		friend class CCPUDescriptorHeapPool;

	protected:

		TVector<SharedPointer<CDescriptorHeap> > DescriptorHeaps;

	protected:

		TMutex Mutex;

	public:

		CDescriptorHeap * RequestDescriptorHeap
		(
			const	D3D12_DESCRIPTOR_HEAP_TYPE	  Type,
			const	UINT						  Size
		);
	};

	class CCPUDescriptorHeapPool : public CDescriptorHeapPool
	{
	private:

		TVector<ConstPointer<CDescriptorHeap> > DescriptorHeaps;

	public:

		using CDescriptorHeapPool::CDescriptorHeapPool;

		DescriptorHeapRange RequestDescriptorHeapRange
		(
			const UINT Size = 1
		);
	};

	class CGPUDescriptorHeapManager : public CSingleton<CGPUDescriptorHeapManager>
	{
		friend class CGPUDescriptorHeapPool;

	protected:

		TQueue<TPair<UINT64, CMutableDescriptorHeap*> >	DescriptorHeapsRetired[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
		TQueue<CMutableDescriptorHeap*>					DescriptorHeapsAvailable[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
		TVector<UniquePointer<CMutableDescriptorHeap> >	DescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
	
	protected:

		TMutex Mutex;

		CMutableDescriptorHeap * RequestDescriptorHeap
		(
					CMutableDescriptorHeap	*	Copy,
			const	UINT						Size
		);

	public:

		CMutableDescriptorHeap * RequestDescriptorHeap
		(
			const	D3D12_DESCRIPTOR_HEAP_TYPE	Type
		);

		void RequestDescriptorHeapCopy
		(
					CMutableDescriptorHeap *	Heap,
			const	UINT						NewHeapSize,
			const	UINT64						FenceValue
		);

		void RetireDescriptorHeap
		(
					CMutableDescriptorHeap *	Heap,
			const	UINT64						FenceValue
		);
	};

	class CGPUDescriptorHeapPool : public CDescriptorHeapPool
	{
	private:

		TVector<CMutableDescriptorHeap*>	DescriptorHeaps;
		TVector<CMutableDescriptorHeap*>	DescriptorHeapsFixed;
		TVector<CMutableDescriptorHeap*>	DescriptorHeapsRetired;
		TVector<ID3D12DescriptorHeap*>		DescriptorHeapObjects;

	public:
		
		using CDescriptorHeapPool::CDescriptorHeapPool;

		void RevokeDescriptorHeaps
		(
			const UINT64 FenceValue
		);

		DescriptorHeapRange RequestDescriptorHeapRange
		(
			const UINT Size
		);

		DescriptorHeapRange RequestFixedDescriptorHeapRange
		(
			const UINT Size
		);

		inline UINT GetHeapObjects
		(
			ID3D12DescriptorHeap ** ppHeaps
		)
		{
			boost::mutex::scoped_lock Lock(CGPUDescriptorHeapManager::Instance().Mutex);

			ppHeaps = DescriptorHeapObjects.data();

			return DescriptorHeapObjects.size();
		}
	};
}