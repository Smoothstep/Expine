#pragma once

#include "RawDescriptorHeap.h"
#include "RawResource.h"
#include "RawCommandList.h"

namespace D3D
{
	class RDepthStencilView
	{
	private:

		SharedPointer<RResource> Resource;

	private:

		DescriptorHeapEntry DescriptorHeap;

	public:

		inline const DescriptorHeapEntry & GetDescriptorHeapEntry() const
		{
			return DescriptorHeap;
		}

		inline RResource * GetResource() const
		{
			return Resource.Get();
		}

		inline void AsDepthRead
		(
			const RGrpCommandList & GrpCmdList
		)	const
		{
			Ensure(Resource != NULL);
			Resource->SetResourceState(D3D12_RESOURCE_STATE_DEPTH_READ, GrpCmdList);
		}

		inline void AsDepthWrite
		(
			const RGrpCommandList & GrpCmdList
		)	const
		{
			Ensure(Resource != NULL);
			Resource->SetResourceState(D3D12_RESOURCE_STATE_DEPTH_WRITE, GrpCmdList);
		}

		inline void AsShaderResource
		(
			const RGrpCommandList & GrpCmdList
		)	const
		{
			Ensure(Resource != NULL);
			Resource->SetResourceState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, GrpCmdList);
		}

		inline void ClearDepth
		(
			const RGrpCommandList & CmdList
		)	const
		{
			CmdList->ClearDepthStencilView(DescriptorHeap, D3D12_CLEAR_FLAG_DEPTH, 
				Resource->GetDefaultClearValue().DepthStencil.Depth, 
				Resource->GetDefaultClearValue().DepthStencil.Stencil, 0, NULL);
		}

		inline void ClearStencil
		(
			const RGrpCommandList & CmdList
		)	const
		{
			CmdList->ClearDepthStencilView(DescriptorHeap, D3D12_CLEAR_FLAG_STENCIL, 
				Resource->GetDefaultClearValue().DepthStencil.Depth, 
				Resource->GetDefaultClearValue().DepthStencil.Stencil, 0, NULL);
		}

		inline void Clear
		(
			const RGrpCommandList & CmdList
		)	const
		{
			CmdList->ClearDepthStencilView(DescriptorHeap, D3D12_CLEAR_FLAG_STENCIL | D3D12_CLEAR_FLAG_DEPTH, 
				Resource->GetDefaultClearValue().DepthStencil.Depth, 
				Resource->GetDefaultClearValue().DepthStencil.Stencil, 0, NULL);
		}

		inline void ClearDepth
		(
			const RGrpCommandList &	CmdList,
			const FLOAT				Depth
		)	const
		{
			CmdList->ClearDepthStencilView(DescriptorHeap, D3D12_CLEAR_FLAG_DEPTH, Depth, 
				Resource->GetDefaultClearValue().DepthStencil.Stencil, 0, NULL);
		}

		inline void Clear
		(
			const RGrpCommandList &	CmdList,
			const FLOAT				Depth
		)	const
		{
			CmdList->ClearDepthStencilView(DescriptorHeap, D3D12_CLEAR_FLAG_STENCIL | D3D12_CLEAR_FLAG_DEPTH, Depth, 
				Resource->GetDefaultClearValue().DepthStencil.Stencil, 0, NULL);
		}

		inline void Clear
		(
			const RGrpCommandList &	CmdList,
			const FLOAT				Depth,
			const UINT8				Stencil
		)	const
		{
			CmdList->ClearDepthStencilView(DescriptorHeap, D3D12_CLEAR_FLAG_STENCIL | D3D12_CLEAR_FLAG_DEPTH, Depth, Stencil, 0, NULL);
		}

		static inline DXGI_FORMAT ConvertFormat
		(
			const DXGI_FORMAT Format
		)
		{
			switch (Format)
			{
				case DXGI_FORMAT_R32_TYPELESS:
				{
					return DXGI_FORMAT_D32_FLOAT;
				}

				case DXGI_FORMAT_R32G8X24_TYPELESS:
				case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
				{
					return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
				}

				case DXGI_FORMAT_R24G8_TYPELESS:
				case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
				{
					return DXGI_FORMAT_D24_UNORM_S8_UINT;
				}

				case DXGI_FORMAT_R16_TYPELESS:
				{
					return DXGI_FORMAT_D16_UNORM;
				}
			}

			return Format;
		}

		ErrorCode CreateFromResource
		(
			const SharedPointer<RResource>	&	pResource,
			const DescriptorHeapEntry		&	HeapEntry,
			const BOOL							ReadOnly	= FALSE,
			const UINT							Size		= 1
		)
		{
			CHECK_NULL_ARG(pResource);

			if (!HeapEntry.Valid())
			{
				return E_INVALIDARG;
			}

			if (Size < 1)
			{
				return E_INVALIDARG;
			}

			D3D12_RESOURCE_DESC ResourceDesc = pResource.GetRef()->GetDesc();

			bool Multisample = ResourceDesc.SampleDesc.Count > 1;

			D3D12_DEPTH_STENCIL_VIEW_DESC DSVDesc = {};
			{
				DSVDesc.Format = ConvertFormat(ResourceDesc.Format);

				if (DSVDesc.Format != DXGI_FORMAT_D16_UNORM			&&
					DSVDesc.Format != DXGI_FORMAT_D24_UNORM_S8_UINT	&&
					DSVDesc.Format != DXGI_FORMAT_D32_FLOAT			&&
					DSVDesc.Format != DXGI_FORMAT_D32_FLOAT_S8X24_UINT)
				{
					return E_INVALIDARG;
				}

				switch(ResourceDesc.Dimension)
				{
					case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
					{
						if (Multisample)
						{
							return E_INVALIDARG;
						}

						DSVDesc.ViewDimension = Size == 1 ?
							D3D12_DSV_DIMENSION_TEXTURE1D :
							D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
					}

					break;

					case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
					{
						DSVDesc.ViewDimension = Size == 1 ?
							(Multisample ? D3D12_DSV_DIMENSION_TEXTURE2DMS		: D3D12_DSV_DIMENSION_TEXTURE2D) :
							(Multisample ? D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY	: D3D12_DSV_DIMENSION_TEXTURE2DARRAY);
					}
					
					break;

					default:
					{
						return E_INVALIDARG;
					}
				}
				
				UINT MipLevels = ResourceDesc.MipLevels - 1;

				switch (DSVDesc.ViewDimension)
				{
					case D3D12_DSV_DIMENSION_TEXTURE1D:
					{
						DSVDesc.Texture1D.MipSlice = MipLevels;
					}

					break;

					case D3D12_DSV_DIMENSION_TEXTURE1DARRAY:
					{
						DSVDesc.Texture1DArray.ArraySize		= Size;
						DSVDesc.Texture1DArray.MipSlice			= MipLevels;
						DSVDesc.Texture1DArray.FirstArraySlice	= 0;
					}

					break;

					case D3D12_DSV_DIMENSION_TEXTURE2D:
					{
						DSVDesc.Texture2D.MipSlice = MipLevels;
					}

					break;

					case D3D12_DSV_DIMENSION_TEXTURE2DARRAY:
					{
						DSVDesc.Texture2DArray.ArraySize		= Size;
						DSVDesc.Texture2DArray.MipSlice			= MipLevels;
						DSVDesc.Texture2DArray.FirstArraySlice	= 0;
					}

					break;

					case D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY:
					{
						DSVDesc.Texture2DMSArray.ArraySize			= Size;
						DSVDesc.Texture2DMSArray.FirstArraySlice	= 0;
					}

					break;

					default:
					{
						return E_INVALIDARG;
					}
				}

				if (ReadOnly)
				{
					DSVDesc.Flags |= D3D12_DSV_FLAG_READ_ONLY_DEPTH;

					if (DSVDesc.Format == DXGI_FORMAT_D24_UNORM_S8_UINT ||
						DSVDesc.Format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT)
					{
						DSVDesc.Flags |= D3D12_DSV_FLAG_READ_ONLY_STENCIL;
					}
				}
				else
				{
					DSVDesc.Flags |= D3D12_DSV_FLAG_NONE;
				}
			}

			Resource = pResource;

			DEVICE->CreateDepthStencilView
			(
				Resource.GetRef(),
				&DSVDesc,
				HeapEntry
			);

			DescriptorHeap = HeapEntry;

			return S_OK;
		}
	};

	template
	<
		size_t Count
	> 
	struct DepthStencilViewGroup
	{
		SharedPointer<RDescriptorHeap> DescriptorHeap;
		SharedPointer<RDepthStencilView> DepthStencilView[Count];

		DepthStencilViewGroup() = default;
		DepthStencilViewGroup
		(
			SharedPointer<RDescriptorHeap> & pDescriptorHeap
		)
		{
			DescriptorHeap = pDescriptorHeap;
		};

		DepthStencilViewGroup
		(
			SharedPointer<RDescriptorHeap>	 & pDescriptorHeap,
			SharedPointer<RDepthStencilView> * ppDepthStencilViews
		)
		{
			DescriptorHeap = pDescriptorHeap;

			for (UINT N = 0; N < Count; ++N)
			{
				DepthStencilView[N] = ppDepthStencilViews[N];
			}
		}

		ErrorCode Initialize
		(
			const SharedPointer<RResource> ppResources[Count]
		);

		ErrorCode Initialize
		(
			const SharedPointer<RResource>			ppResources[Count],
			const SharedPointer<RDescriptorHeap> &	pDescriptorHeap,
			const UINT								Offset = 0
		);

		inline const SharedPointer<RDepthStencilView> & operator[]
		(
			const UINT N
		)	const
		{
			return DepthStencilView[N];
		}
	};

	template
	<
		size_t Count
	>
	ErrorCode DepthStencilViewGroup<Count>::Initialize
	(
		const SharedPointer<RResource> ppResources[Count]
	)
	{
		ErrorCode Error;

		if (DescriptorHeap == NULL)
		{
			DescriptorHeap = new RDescriptorHeap();

			if ((Error = DescriptorHeap->Create_DSV(Count)))
			{
				return Error;
			}
		}

		for (UINT Index = 0; Index < Count; ++Index)
		{
			DepthStencilView[Index] = new RDepthStencilView();

			if ((Error = DepthStencilView[Index]->CreateFromResource(ppResources[Index], DescriptorHeapEntry(DescriptorHeap.Get(), Index))))
			{
				return Error;
			}
		}

		return S_OK;
	}

	template
	<
		size_t Count
	>
	ErrorCode DepthStencilViewGroup<Count>::Initialize
	(
		const SharedPointer<RResource>			ppResources[Count],
		const SharedPointer<RDescriptorHeap> &	pDescriptorHeap,
		const UINT								Offset
	)
	{
		CHECK_NULL_ARG(pDescriptorHeap);

		DescriptorHeap = pDescriptorHeap;

		ErrorCode Error;

		for (UINT Index = 0; Index < Count; ++Index)
		{
			DepthStencilView[Index] = new RDepthStencilView();

			if ((Error = DepthStencilView[Index]->CreateFromResource(ppResources[Index], DescriptorHeapEntry(pDescriptorHeap, Offset + Index))))
			{
				return Error;
			}
		}

		return S_OK;
	}

	inline void RGrpCommandList::SetRenderTarget
	(
		const RDepthStencilView & DepthStencil
	)	const
	{
		D3D12_CPU_DESCRIPTOR_HANDLE DSVDescriptorHandle = DepthStencil.GetDescriptorHeapEntry();

		CommandList->OMSetRenderTargets(0, NULL, FALSE, &DSVDescriptorHandle);
	}

	inline void RGrpCommandList::Clear
	(
		const RDepthStencilView & DepthStencil
	)	const
	{
		DepthStencil.Clear(*this);
	}
}