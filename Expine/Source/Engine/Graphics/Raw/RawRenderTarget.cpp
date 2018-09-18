#include "Precompiled.h"

#include "Raw/RawRenderTarget.h"

namespace D3D
{
	ErrorCode RRenderTargetView::CreateFromResource(const SharedPointer<RResource> & pResource, const DescriptorHeapEntry & HeapEntry)
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

		D3D12_RENDER_TARGET_VIEW_DESC RTVDesc = {};
		{
			RTVDesc.Format = ResourceDesc.Format;
			RTVDesc.ViewDimension =
				ResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D	? D3D12_RTV_DIMENSION_TEXTURE1D :
				ResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D	? D3D12_RTV_DIMENSION_TEXTURE2D :
				ResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D	? D3D12_RTV_DIMENSION_TEXTURE3D :
				ResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER		? D3D12_RTV_DIMENSION_BUFFER	: D3D12_RTV_DIMENSION_UNKNOWN;

			UINT MipLevels = ResourceDesc.MipLevels - 1;

			switch (RTVDesc.ViewDimension)
			{
				case D3D12_RTV_DIMENSION_TEXTURE1D:
				{
					RTVDesc.Texture1D.MipSlice = MipLevels;
				}

				break;

				case D3D12_RTV_DIMENSION_TEXTURE2D:
				{
					RTVDesc.Texture2D.MipSlice = MipLevels;
				}

				break;

				case D3D12_RTV_DIMENSION_TEXTURE3D:
				{
					RTVDesc.Texture3D.MipSlice		= MipLevels;
					RTVDesc.Texture3D.FirstWSlice	=  0;
					RTVDesc.Texture3D.WSize			= -1;
				}

				break;

				case D3D12_RTV_DIMENSION_BUFFER:
				{
					RTVDesc.Buffer.NumElements = ResourceDesc.Width * ResourceDesc.Height * ResourceDesc.MipLevels;
				}

				break;

				case D3D12_RTV_DIMENSION_TEXTURE1DARRAY:
				{
					RTVDesc.Texture1DArray.ArraySize	= ResourceDesc.DepthOrArraySize;
					RTVDesc.Texture1DArray.MipSlice		= MipLevels;
				}

				break;

				case D3D12_RTV_DIMENSION_TEXTURE2DARRAY:
				{
					RTVDesc.Texture2DArray.ArraySize	= ResourceDesc.DepthOrArraySize;
					RTVDesc.Texture2DArray.MipSlice		= MipLevels;
				}

				break;

				case D3D12_RTV_DIMENSION_TEXTURE2DMS:
				{
					break;
				}

				case D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY:
				{
					RTVDesc.Texture2DMSArray.ArraySize = ResourceDesc.DepthOrArraySize;
				}

				break;
			}
		}

		Resource = pResource;

		DEVICE->CreateRenderTargetView(
			Resource.GetRef(),
			&RTVDesc,
			HeapEntry);

		DescriptorHeap = HeapEntry;

		return S_OK;
	}
}
