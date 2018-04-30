#pragma once

#include "RawDescriptorHeap.h"
#include "RawDevice.h"
#include "RawCommandList.h"

#include "RawResource.h"

namespace D3D
{
	class RShaderResourceView
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

		inline void SetAsShaderResource
		(
			const RGrpCommandList & CmdList,
			const UINT				ParameterIndex = 0
		)	const
		{
			CmdList->SetGraphicsRootShaderResourceView(ParameterIndex, Resource.GetRef()->GetGPUVirtualAddress());
		}

	public:

		inline RShaderResourceView() = default;
		inline RShaderResourceView
		(
			const SharedPointer<RResource> & pResource
		)
		{
			Resource = pResource;
		}

		// DSV -> SRV

		static inline DXGI_FORMAT ConvertFormat
		(
			DXGI_FORMAT Format
		)
		{
			switch (Format)
			{
				case DXGI_FORMAT_R32G8X24_TYPELESS:
				{
					Format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
				}

				break;
				
				case DXGI_FORMAT_R24G8_TYPELESS:
				{
					Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
				}

				break;

				case DXGI_FORMAT_R32_TYPELESS:
				{
					Format = DXGI_FORMAT_R32_FLOAT;
				}

				break;

				case DXGI_FORMAT_R16_TYPELESS:
				{
					Format = DXGI_FORMAT_R16_UNORM;
				}

				break;
			}

			return Format;
		}

		ErrorCode CreateFromResource
		(
			const SharedPointer<RResource>	&	pResource,
			const DescriptorHeapEntry		&	HeapEntry,
			const DXGI_FORMAT					Format = DXGI_FORMAT_UNKNOWN,
			const D3D12_BUFFER_SRV			*	pBuffer = 0
		)
		{
			CHECK_NULL_ARG(pResource);
			
			if (!HeapEntry.Valid())
			{
				return E_INVALIDARG;
			}

			D3D12_RESOURCE_DESC ResourceDesc = pResource.GetRef()->GetDesc();

			if (ResourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE1D &&
				ResourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D &&
				ResourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE3D &&
				ResourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER)
			{
				return E_INVALIDARG;
			}

			bool Multisample = ResourceDesc.SampleDesc.Count > 1;

			D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
			{
				if (Format != DXGI_FORMAT_UNKNOWN)
				{
					SRVDesc.Format = Format;
				}
				else
				{
					SRVDesc.Format = ConvertFormat(ResourceDesc.Format);
				}

				if (ResourceDesc.DepthOrArraySize > 1)
				{
					if (ResourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE1D &&
						ResourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D &&
						ResourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE3D)
					{
						return E_INVALIDARG;
					}

					if (Multisample)
					{
						if (ResourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D)
						{
							return E_INVALIDARG;
						}
	
						SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
					}
					else
					{
						SRVDesc.ViewDimension = ResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D ? D3D12_SRV_DIMENSION_TEXTURE1DARRAY	: 
												ResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D ? D3D12_SRV_DIMENSION_TEXTURE3D		: D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
					}
				}
				else
				{
					if (ResourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER &&
						ResourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE1D &&
						ResourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D &&
						ResourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE3D)
					{
						return E_INVALIDARG;
					}

					if (Multisample)
					{
						if (ResourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D)
						{
							return E_INVALIDARG;
						}

						SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
					}
					else
					{
						SRVDesc.ViewDimension =
							ResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D ? D3D12_SRV_DIMENSION_TEXTURE1D :
							ResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D ? D3D12_SRV_DIMENSION_TEXTURE2D :
							ResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D ? D3D12_SRV_DIMENSION_TEXTURE3D : D3D12_SRV_DIMENSION_BUFFER;
					}
				}
				

				SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				
				UINT MipLevels = ResourceDesc.MipLevels;

				switch (SRVDesc.ViewDimension)
				{
					case D3D12_SRV_DIMENSION_TEXTURE1D:
					{
						SRVDesc.Texture1D.MipLevels = MipLevels;
					}

					break;

					case D3D12_SRV_DIMENSION_TEXTURE2D:
					{
						SRVDesc.Texture2D.MipLevels = MipLevels;
					}

					break;

					case D3D12_SRV_DIMENSION_TEXTURE3D:
					{
						SRVDesc.Texture3D.MipLevels = MipLevels;
					}

					break;

					case D3D12_SRV_DIMENSION_BUFFER:
					{
						if (!pBuffer)
						{
							SRVDesc.Buffer.NumElements = ResourceDesc.Width * ResourceDesc.Height * ResourceDesc.MipLevels;
						}
						else if(Format == DXGI_FORMAT_UNKNOWN)
						{
							SRVDesc.Buffer = *pBuffer;
						}
					}

					break;

					case D3D12_SRV_DIMENSION_TEXTURE1DARRAY:
					{
						SRVDesc.Texture1DArray.ArraySize = ResourceDesc.DepthOrArraySize;
						SRVDesc.Texture1DArray.MipLevels = MipLevels;
					}

					break;

					case D3D12_SRV_DIMENSION_TEXTURE2DARRAY:
					{
						SRVDesc.Texture2DArray.ArraySize = ResourceDesc.DepthOrArraySize;
						SRVDesc.Texture2DArray.MipLevels = MipLevels;
					}

					break;

					case D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY:
					{
						SRVDesc.Texture2DMSArray.ArraySize = ResourceDesc.DepthOrArraySize;
					}

					break;
				}
			}

			Resource = pResource;

			DEVICE->CreateShaderResourceView
			(
				Resource.GetRef(),
				&SRVDesc,
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
	struct ShaderResourceViewGroup
	{
		SharedPointer<RDescriptorHeap> DescriptorHeap;
		SharedPointer<RShaderResourceView> ShaderResourceViews[Count];

		RResource * ResourceObjects[Count];

		ErrorCode Initialize
		(
			const SharedPointer<RResource>			ppResources[Count],
			const D3D12_DESCRIPTOR_HEAP_FLAGS		Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
		);

		ErrorCode Initialize
		(
			RResource							*	ppResources[Count],
			const D3D12_DESCRIPTOR_HEAP_FLAGS		Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
		);

		ErrorCode Initialize
		(
			const SharedPointer<RResource>			ppResources[Count],
			const SharedPointer<RDescriptorHeap> &	pDescriptorHeap,
			const UINT								Offset = 0
		);

		ErrorCode Initialize
		(
			RResource							 *	ppResources[Count],
			const SharedPointer<RDescriptorHeap> &	pDescriptorHeap,
			const UINT								Offset = 0
		);

		ErrorCode Initialize
		(
			RResource							*	pResource,
			const D3D12_BUFFER_SRV				*	pBufferDesc = 0,
			const UINT								Offset = 0
		);

		ErrorCode Initialize
		(
			RResource							*	pResource,
			const DXGI_FORMAT						Format,
			const UINT								Offset = 0
		);

		inline const SharedPointer<RShaderResourceView> & operator[]
		(
			const UINT N
		)	const
		{
			return ShaderResourceViews[N];
		}
	};

	template
	<
		size_t Count
	>
	ErrorCode ShaderResourceViewGroup<Count>::Initialize
	(
		const SharedPointer<RResource>		ppResources[Count],
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
				ShaderResourceViews[Index] = new RShaderResourceView();

				if ((Error = ShaderResourceViews[Index]->CreateFromResource(ppResources[Index], DescriptorHeapEntry(DescriptorHeap.Get(), Index))))
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
	inline ErrorCode ShaderResourceViewGroup<Count>::Initialize(RResource * ppResources[Count], const D3D12_DESCRIPTOR_HEAP_FLAGS Flags)
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
				ShaderResourceViews[Index] = new RShaderResourceView();

				if ((Error = ShaderResourceViews[Index]->CreateFromResource(ppResources[Index], DescriptorHeapEntry(DescriptorHeap.Get(), Index))))
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
	ErrorCode ShaderResourceViewGroup<Count>::Initialize
	(
		const SharedPointer<RResource>			ppResources[Count],
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
				ShaderResourceViews[Index] = new RShaderResourceView();

				if ((Error = ShaderResourceViews[Index]->CreateFromResource(ppResources[Index], DescriptorHeapEntry(pDescriptorHeap.Get(), Offset + Index))))
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
	ErrorCode ShaderResourceViewGroup<Count>::Initialize
	(
		RResource							 *	ppResources[Count],
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
				ShaderResourceViews[Index] = new RShaderResourceView();

				if ((Error = ShaderResourceViews[Index]->CreateFromResource(ppResources[Index], DescriptorHeapEntry(pDescriptorHeap.Get(), Offset + Index))))
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
	ErrorCode ShaderResourceViewGroup<Count>::Initialize
	(
		RResource							*	pResource,
		const DXGI_FORMAT						Format,
		const UINT								Offset = 0
	)
	{
		ErrorCode Error;

		if (pResource)
		{
			ShaderResourceViews[Offset] = new RShaderResourceView();

			if ((Error = ShaderResourceViews[Offset]->CreateFromResource(pResource, DescriptorHeapEntry(DescriptorHeap.Get(), Offset), Format)))
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
	ErrorCode ShaderResourceViewGroup<Count>::Initialize
	(
		RResource							*	pResource,
		const D3D12_BUFFER_SRV				*	pBufferDesc = 0,
		const UINT								Offset = 0
	)
	{
		ErrorCode Error;

		if (pResource)
		{
			ShaderResourceViews[Offset] = new RShaderResourceView();

			if ((Error = ShaderResourceViews[Offset]->CreateFromResource(pResource, DescriptorHeapEntry(DescriptorHeap.Get(), Offset), DXGI_FORMAT_UNKNOWN, pBufferDesc)))
			{
				return Error;
			}

			ResourceObjects[Offset] = pResource;
		}

		return S_OK;
	}

	inline void RGrpCommandList::SetShaderResourceView(const UINT RootParameter, const RShaderResourceView & ShaderResourceView) const
	{
		CommandList->SetGraphicsRootShaderResourceView(RootParameter, ShaderResourceView.GetResource()->GetGPUAddress());
	}

	inline void RGrpCommandList::SetShaderResourceViewCompute(const UINT RootParameter, const RShaderResourceView & ShaderResourceView) const
	{
		CommandList->SetComputeRootShaderResourceView(RootParameter, ShaderResourceView.GetResource()->GetGPUAddress());
	}

	inline void RGrpCommandList::SetShaderResourceView(const UINT RootParameter, const D3D12_GPU_VIRTUAL_ADDRESS GPUAddress) const
	{
		CommandList->SetGraphicsRootShaderResourceView(RootParameter, GPUAddress);
	}

	inline void RGrpCommandList::SetShaderResourceViewCompute
	(
		const UINT						RootParameter,
		const D3D12_GPU_VIRTUAL_ADDRESS GPUAddress
	)	const
	{
		CommandList->SetComputeRootShaderResourceView(RootParameter, GPUAddress);
	}
}