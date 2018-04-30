#pragma once

#include "RawDescriptorHeap.h"
#include "RawDevice.h"
#include "RawCommandList.h"
#include "RawResource.h"
#include "RawDepthStencilView.h"

namespace D3D
{
	class RRenderTargetView
	{
	public:

		static constexpr Float4 DefaultClearColor = 
		{ 
			0.0f, 
			0.0f, 
			0.0f, 
			1.0f 
		};

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

	public:

		inline void Clear
		(
			const RGrpCommandList & CmdList
		)	const
		{
			CmdList->ClearRenderTargetView(DescriptorHeap, DefaultClearColor, 0, NULL); 
			{
				Resource->ResourceState = D3D12_RESOURCE_STATE_RENDER_TARGET;
			}
		}

		inline void Clear
		(
			const RGrpCommandList & CmdList,
			const Float4			Color
		)	const
		{
			CmdList->ClearRenderTargetView(DescriptorHeap, Color, 0, NULL);
			{
				Resource->ResourceState = D3D12_RESOURCE_STATE_RENDER_TARGET;
			}
		}

		/***************************************************************************
		*
		*	Only render to this RTV without any DSV.
		*
		****************************************************************************/

		inline void SetAsRenderTarget
		(
			const RGrpCommandList & CmdList
		)
		{
			D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHandles[] = 
			{ 
				DescriptorHeap 
			};

			CmdList->OMSetRenderTargets(1, DescriptorHandles, FALSE, NULL);
		}

	public:

		inline RRenderTargetView()
		{}

		inline RRenderTargetView
		(
			RResource * pResource
		)
		{
			Resource = pResource;
		}

		ErrorCode CreateFromResource
		(
			const SharedPointer<RResource>	& pResource,
			const DescriptorHeapEntry		& HeapEntry
		);
	};

	/**************************************************************************
	*
	*	When the amount of RTVs is known at compile time.
	*
	**************************************************************************/

	template
	<
		size_t Count
	> 
	struct RenderTargetViewGroup
	{
		SharedPointer<RDescriptorHeap> DescriptorHeap;
		SharedPointer<RRenderTargetView> RenderTargetView[Count];

		RenderTargetViewGroup() 
		{};

		RenderTargetViewGroup
		(
			SharedPointer<RDescriptorHeap> & pDescriptorHeap
		)
		{
			DescriptorHeap = pDescriptorHeap;
		};

		RenderTargetViewGroup
		(
			SharedPointer<RDescriptorHeap>	 & pDescriptorHeap,
			SharedPointer<RRenderTargetView> * ppRenderTargetViews
		)
		{
			DescriptorHeap = pDescriptorHeap;

			for (UINT N = 0; N < Count; ++N)
			{
				RenderTargetView[N] = ppRenderTargetViews[N];
			}
		}

		ErrorCode Initialize
		(
			const SharedPointer<RResource> ppResources[Count]
		);

		ErrorCode Initialize
		(
			RResource * ppResources[Count]
		);

		ErrorCode Initialize
		(
			const SharedPointer<RResource>			ppResources[Count],
			const SharedPointer<RDescriptorHeap> &	pDescriptorHeap,
			const UINT								Offset = 0
		);

		inline const SharedPointer<RRenderTargetView> & operator[]
		(
			const UINT N
		)	const
		{
			return RenderTargetView[N];
		}

		inline void SetAsRenderTargets
		(
			const RGrpCommandList & CmdList
		)
		{
			D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHandles[Count];

#pragma unroll(1)
			for (UINT N = 0; N < Count; ++N)
			{
				DescriptorHandles[N] = RenderTargetView[N]->GetDescriptorHeapEntry();
			}

			CmdList->OMSetRenderTargets(Count, DescriptorHandles, FALSE, NULL);
		}
	};

	template
	<
		size_t Count
	>
		inline ErrorCode RenderTargetViewGroup<Count>::Initialize
	(
		const SharedPointer<RResource> ppResources[Count]
	)
	{
		ErrorCode Error;

		if (DescriptorHeap == NULL)
		{
			DescriptorHeap = new RDescriptorHeap();

			if ((Error = DescriptorHeap->Create_RTV(Count)))
			{
				return Error;
			}
		}

		for (UINT Index = 0; Index < Count; ++Index)
		{
			if (ppResources[Index])
			{
				RenderTargetView[Index] = new RRenderTargetView();

				if ((Error = RenderTargetView[Index]->CreateFromResource(ppResources[Index], DescriptorHeapEntry(DescriptorHeap.Get(), Index))))
				{
					return Error;
				}
			}
		}

		return S_OK;
	}

	template
	<
		size_t Count
	>
	inline ErrorCode RenderTargetViewGroup<Count>::Initialize(RResource * ppResources[Count])
	{
		ErrorCode Error;

		if (DescriptorHeap == NULL)
		{
			DescriptorHeap = new RDescriptorHeap();

			if ((Error = DescriptorHeap->Create_RTV(Count)))
			{
				return Error;
			}
		}

		for (UINT Index = 0; Index < Count; ++Index)
		{
			if (ppResources[Index])
			{
				RenderTargetView[Index] = new RRenderTargetView();

				if ((Error = RenderTargetView[Index]->CreateFromResource(ppResources[Index], DescriptorHeapEntry(DescriptorHeap.Get(), Index))))
				{
					return Error;
				}
			}
		}

		return S_OK;
	}

	template
	<
		size_t Count
	>
	inline ErrorCode RenderTargetViewGroup<Count>::Initialize
	(
		const SharedPointer<RResource>			ppResources[Count], 
		const SharedPointer<RDescriptorHeap> &	pDescriptorHeap,
		const UINT								Offset
	)
	{
		CHECK_NULL_ARG(pDescriptorHeap);

		DescriptorHeap = pDescriptorHeap;

		for (UINT Index = 0; Index < Count; ++Index)
		{
			if (ppResources[Index])
			{
				RenderTargetView[Index] = new RRenderTargetView();

				if ((Error = RenderTargetView[Index]->CreateFromResource(ppResources[Index], DescriptorHeapEntry(DescriptorHeap.Get(), Offset + Index))))
				{
					return Error;
				}
			}
		}

		return S_OK;
	}

	template
	<
		size_t NumRenderTargets
	>
	inline void RGrpCommandList::SetRenderTargets
	(
		const RRenderTargetView * RenderTargets[NumRenderTargets],
		const RDepthStencilView & DepthStencil
	)	const
	{
		D3D12_CPU_DESCRIPTOR_HANDLE DSVDescriptorHandle = DepthStencil.GetDescriptorHeapEntry();
		D3D12_CPU_DESCRIPTOR_HANDLE RTVDescriptorHandles[NumRenderTargets];
		{
#pragma unroll(1)
			for (UINT N = 0; N < NumRenderTargets; ++N)
			{
				RTVDescriptorHandles[N] = RenderTargets[N]->GetDescriptorHeapEntry();
			}
		}

		CommandList->OMSetRenderTargets(NumRenderTargets, RTVDescriptorHandles, FALSE, &DSVDescriptorHandle);
	}

	template
	<
		size_t NumRenderTargets
	>
	inline void RGrpCommandList::SetRenderTargets
	(
		const RRenderTargetView * RenderTargets[NumRenderTargets]
	)	const
	{
		D3D12_CPU_DESCRIPTOR_HANDLE RTVDescriptorHandles[NumRenderTargets];
		{
#pragma unroll(1)
			for (UINT N = 0; N < NumRenderTargets; ++N)
			{
				RTVDescriptorHandles[N] = RenderTargets[N]->GetDescriptorHeapEntry();
			}
		}

		CommandList->OMSetRenderTargets(NumRenderTargets, RTVDescriptorHandles, FALSE, NULL);
	}

	inline void RGrpCommandList::SetRenderTarget
	(
		const RRenderTargetView & RenderTarget,
		const RDepthStencilView & DepthStencil
	)	const
	{
		D3D12_CPU_DESCRIPTOR_HANDLE RTVDescriptorHandle = RenderTarget.GetDescriptorHeapEntry();
		D3D12_CPU_DESCRIPTOR_HANDLE DSVDescriptorHandle = DepthStencil.GetDescriptorHeapEntry();

		CommandList->OMSetRenderTargets(1, &RTVDescriptorHandle, FALSE, &DSVDescriptorHandle);
	}

	inline void RGrpCommandList::SetRenderTarget
	(
		const RRenderTargetView & RenderTarget
	)	const
	{
		D3D12_CPU_DESCRIPTOR_HANDLE RTVDescriptorHandle = RenderTarget.GetDescriptorHeapEntry();

		CommandList->OMSetRenderTargets(1, &RTVDescriptorHandle, FALSE, NULL);
	}

	inline void RGrpCommandList::Clear
	(
		const RRenderTargetView & RenderTarget
	)	const
	{
		RenderTarget.Clear(*this);
	}
}