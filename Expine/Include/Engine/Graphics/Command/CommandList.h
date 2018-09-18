#pragma once

#include "Raw/RawCommandList.h"
#include "Descriptor/DescriptorHeapPool.h"

namespace D3D
{
	class CResourceAllocator;
	class RCommandQueue;
	class CGrpBuffer;

	static constexpr UINT DescriptorEntriesPerHeap			= 1024;
	static constexpr UINT DescriptorEntriesPerDynamicRange	= 128;

	class CCompletionEventListener
	{
	public:
		virtual void OnComplete(const UINT64 Value) = 0;
	};

	class CCommandListBase : public RGrpCommandList
	{
	protected:

		UINT64 FenceValue = -1;

	protected:

		CMutableDescriptorHeap	*						DescriptorHeapViews;
		CMutableDescriptorHeap	*						DescriptorHeapSamplers;
		CMutableDescriptorHeap	*						DescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

		UniquePointer<CResourceAllocator>				ResourceAllocatorCPU;
		UniquePointer<CResourceAllocator>				ResourceAllocatorGPU;

		TVector<SharedPointer<RResource> >				FrameResources;

		TVector<CDescriptorHeapTableDistributor>		DescriptorTableReferrerViews;
		TVector<CDescriptorHeapTableDistributor>		DescriptorTableReferrerSamplers;

		TMap<UINT, DescriptorHeapRange>					DescriptorTableStaticsMap;

	protected:

		RCommandQueue * CommandQueue;

	protected:

		ErrorCode Create();

	public:

		CCommandListBase();

		void AddFrameResource
		(
			RResource * pResource
		);

		void AddFrameResource
		(
			const SharedPointer<RResource>& pResource
		);

		void AddFrameResource
		(
			const CGrpBuffer * pBuffer
		);

		void CopyDataToBuffer
		(
					RResource	*	pBuffer,
			const	void		*	pData,
			const	UINT			Size
		)	const;

		void CopyDataToTexture
		(
					RResource				*	pTexture,
					D3D12_SUBRESOURCE_DATA	*	pData,
			const	UINT						FirstSubResource,
			const	UINT						NumSubResources
		)	const;

		DescriptorHeapRange OccupyViewDescriptorRange
		(
			const UINT Range
		);

		DescriptorHeapRange OccupyViewDescriptorRange
		(
			const RRootSignature & RootSignature
		);

		DescriptorHeapRange OccupyViewDescriptorRangeDynamic
		(
			const UINT Range
		);

		void FreeDescriptorRange
		(
			const DescriptorHeapRange & Range
		);

		DescriptorHeapRange OccupySamplerDescriptorRange
		(
			const UINT Range
		);

		DescriptorHeapRange OccupySamplerDescriptorRange
		(
			const RRootSignature & RootSignature
		);

		void SetDescriptorHeapTableStaticRange
		(
			const UINT					Ident,
			const DescriptorHeapRange & Range
		);

		void CopyDescriptorHeapEntries
		(
			const DescriptorHeapRange & Destination,
			const DescriptorHeapEntry * Entries,
			const UINT					NumEntries
		);
	};

	class CCommandListContext : public CCommandListBase, CCompletionEventListener
	{
	private:

		SharedPointer<RCommandAllocator> CommandAllocator[FRAME_COUNT];

	private:

		virtual void OnComplete
		(
			const UINT64 Value
		) override;

		void SetDescriptorHeaps();

	public:

		CCommandListContext
		(
			CMutableDescriptorHeap * pDescriptorHeapViews = 0,
			CMutableDescriptorHeap * pDescriptorHeapSamplers = 0
		);
		
		inline void StartRecording
		(
			const UINT Frame
		)
		{
			SetCommandAllocator(CommandAllocator[Frame]);
		}

		inline CMutableDescriptorHeap * GetViewDescriptorHeap() const
		{
			return DescriptorHeapViews;
		}

		inline DescriptorHeapRange GetDescriptorHeapRange
		(
			const UINT Ident
		)	const
		{
			return DescriptorTableStaticsMap.at(Ident);
		}

		inline CMutableDescriptorHeap * GetSamplerDescriptorHeap() const
		{
			return DescriptorHeapSamplers;
		}

		inline const SharedPointer<RCommandAllocator> & GetCommandAllocator
		(
			const UINT Frame
		)	const
		{
			return CommandAllocator[Frame];
		}

		inline ErrorCode ResetCommandAllocator
		(
			const UINT Frame
		)	const
		{
			return CommandAllocator[Frame]->Reset();
		}

		inline ErrorCode ResetCommandList
		(
			const UINT Frame
		)
		{
			ErrorCode Error = RGrpCommandList::Reset(CommandAllocator[Frame]);
			{
				SetDescriptorHeaps();
			}

			return Error;
		}

		inline void SetGraphicsRootDescriptorTable
		(
			const UINT					RootParameter,
			const UINT					Ident
		)	const
		{
			const DescriptorHeapRange & Range = DescriptorTableStaticsMap.at(Ident);

			RGrpCommandList::SetGraphicsRootDescriptorTable
			(
				RootParameter, Range.Offset, Range.DescriptorHeap
			);
		}

		inline void SetGraphicsRootDescriptorTable
		(
			const UINT					RootParameter,
			const UINT					Ident,
			const UINT					Offset
		)	const
		{
			const DescriptorHeapRange & Range = DescriptorTableStaticsMap.at(Ident);

			RGrpCommandList::SetGraphicsRootDescriptorTable
			(
				RootParameter, Range.Offset + Offset, Range.DescriptorHeap
			);
		}

		inline void SetGraphicsRootDescriptorTable
		(
			const UINT					RootParameter,
			const UINT					Offset,
			const DescriptorHeapEntry & Entry
		)	const
		{
			RGrpCommandList::SetGraphicsRootDescriptorTable
			(
				RootParameter, Entry.Offset + Offset, Entry.DescriptorHeap
			);
		}

		inline void SetGraphicsRootDescriptorTable
		(
			const UINT					RootParameter,
			const DescriptorHeapEntry & Entry
		)	const
		{
			RGrpCommandList::SetGraphicsRootDescriptorTable
			(
				RootParameter, Entry.Offset, Entry.DescriptorHeap
			);
		}

		inline void SetGraphicsRootViewDescriptorTable
		(
			const UINT					RootParameter,
			const UINT					Offset
		)	const
		{
			RGrpCommandList::SetGraphicsRootDescriptorTable
			(
				RootParameter, Offset, DescriptorHeapViews
			);
		}

		inline void SetGraphicsRootSamplerDescriptorTable
		(
			const UINT					RootParameter,
			const UINT					Offset
		)	const
		{
			RGrpCommandList::SetGraphicsRootDescriptorTable
			(
				RootParameter, Offset, DescriptorHeapSamplers
			);
		}

		inline void SetComputeRootDescriptorTable
		(
			const UINT					RootParameter,
			const DescriptorHeapEntry & Entry
		)	const
		{
			RGrpCommandList::SetComputeRootDescriptorTable
			(
				RootParameter, Entry.Offset, Entry.DescriptorHeap.Get()
			);
		}

		inline void SetComputeRootViewDescriptorTable
		(
			const UINT					RootParameter,
			const UINT					Offset
		)	const
		{
			RGrpCommandList::SetGraphicsRootDescriptorTable
			(
				RootParameter, Offset, DescriptorHeapViews
			);
		}

		inline void SetComputeRootSamplerDescriptorTable
		(
			const UINT					RootParameter,
			const UINT					Offset
		)	const
		{
			RGrpCommandList::SetGraphicsRootDescriptorTable
			(
				RootParameter, Offset, DescriptorHeapViews
			);
		}

		inline void SetComputeRootDescriptorTable
		(
			const UINT					RootParameter,
			const UINT					Offset,
			const DescriptorHeapEntry & Entry
		)	const
		{
			RGrpCommandList::SetComputeRootDescriptorTable
			(
				RootParameter, Entry.Offset + Offset, Entry.DescriptorHeap.Get()
			);
		}

		ErrorCode Reset
		(
			const UINT								Frame,
			const SharedPointer<RPipelineState> &	pPipelineState
		);

		void Finish
		(
			const UINT Frame				= 0,
			const BOOL WaitForCompletion	= TRUE
		);

		void WaitForCompletion();

		ErrorCode Create
		(
			bool					Recording	= false,
			D3D12_COMMAND_LIST_TYPE Type		= D3D12_COMMAND_LIST_TYPE_DIRECT
		);
	};

	template
	<
		size_t NumThreads
	>
	class CCommandListGroup
	{
		static_assert(NumThreads != 0, "Thread count must be at least 1");

	private:

		ID3D12CommandList * CommandListPointers[NumThreads];

	private:

		CCommandListContext CommandListContext[NumThreads];

	public:

		ErrorCode Create(bool Recording = false);

	public:

		inline ErrorCode Close();

	public:

		inline CCommandListContext & operator[]
		(
			const UINT Index
		)
		{
			return CommandListContext[Index];
		}

		inline ID3D12CommandList * const * Data() const
		{
			return CommandListPointers;
		}

		inline void Finish
		(
			const UINT Frame,
			const BOOL WaitForCompletion = TRUE
		);
	};

	template
	<
		size_t NumThreads
	>
	ErrorCode CCommandListGroup<NumThreads>::Create(bool Recording)
	{
		ErrorCode Error;

		for (UINT N = 0; N < NumThreads; ++N)
		{
			if ((Error = CommandListContext[N].Create(Recording)))
			{
				return Error;
			}

			CommandListPointers[N] = CommandListContext[N];
		}

		return S_OK;
	}

	template
	<
		size_t NumThreads
	>
	inline ErrorCode CCommandListGroup<NumThreads>::Close()
	{
		ErrorCode Error;

#pragma unroll(1)
		for (UINT N = 0; N < NumThreads; ++N)
		{
			if ((Error = CommandListContext[N].Close()))
			{
				return Error;
			}
		}

		return S_OK;
	}
}