#include "Precompiled.h"

#include "Raw/RawUnorderedAccessView.h"

namespace D3D
{
	ErrorCode RUnorderedAccessView::CreateFromResource(const SharedPointer<RResource>& pResource, const DescriptorHeapEntry & HeapEntry, const DXGI_FORMAT Format, const D3D12_BUFFER_UAV * pBufferDesc)
	{
		CHECK_NULL_ARG(pResource);

		if (!HeapEntry.Valid())
		{
			return E_INVALIDARG;
		}

		D3D12_RESOURCE_DESC ResourceDesc = pResource.GetRef()->GetDesc();

		if (ResourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER		&&
			ResourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE1D	&&
			ResourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D	&&
			ResourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE3D)
		{
			return E_INVALIDARG;
		}

		D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
		{
			if (Format != DXGI_FORMAT_UNKNOWN)
			{
				UAVDesc.Format = Format;
			}
			else
			{
				UAVDesc.Format = ResourceDesc.Format;
			}

			UAVDesc.ViewDimension =
				ResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D ? (ResourceDesc.DepthOrArraySize > 1 ? D3D12_UAV_DIMENSION_TEXTURE1DARRAY : D3D12_UAV_DIMENSION_TEXTURE1D) :
				ResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D ? (ResourceDesc.DepthOrArraySize > 1 ? D3D12_UAV_DIMENSION_TEXTURE2DARRAY : D3D12_UAV_DIMENSION_TEXTURE2D) :
				ResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D ? D3D12_UAV_DIMENSION_TEXTURE3D :
				ResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER ? D3D12_UAV_DIMENSION_BUFFER : D3D12_UAV_DIMENSION_UNKNOWN;

			UINT MipLevels = ResourceDesc.MipLevels - 1;

			switch (UAVDesc.ViewDimension)
			{
			case D3D12_UAV_DIMENSION_TEXTURE1D:
			{
				UAVDesc.Texture1D.MipSlice = MipLevels;
			}

			break;

			case D3D12_UAV_DIMENSION_TEXTURE2D:
			{
				UAVDesc.Texture2D.MipSlice = MipLevels;
			}

			break;

			case D3D12_UAV_DIMENSION_TEXTURE3D:
			{
				UAVDesc.Texture3D.MipSlice = MipLevels;
			}

			break;

			case D3D12_UAV_DIMENSION_BUFFER:
			{
				if (pBufferDesc)
				{
					UAVDesc.Buffer = *pBufferDesc;
				}
				else
				{
					UAVDesc.Buffer.NumElements = ResourceDesc.Width * ResourceDesc.Height * ResourceDesc.MipLevels;
				}
			}

			break;

			case D3D12_UAV_DIMENSION_TEXTURE1DARRAY:
			{
				UAVDesc.Texture1DArray.ArraySize = ResourceDesc.DepthOrArraySize;
				UAVDesc.Texture1DArray.MipSlice = MipLevels;
			}

			break;

			case D3D12_UAV_DIMENSION_TEXTURE2DARRAY:
			{
				UAVDesc.Texture2DArray.ArraySize = ResourceDesc.DepthOrArraySize;
				UAVDesc.Texture2DArray.MipSlice = MipLevels;
			}

			break;
			}
		}

		Resource = pResource;

		DEVICE->CreateUnorderedAccessView
		(
			Resource.GetRef(),
			NULL,
			&UAVDesc,
			HeapEntry
		);

		DescriptorHeap = HeapEntry;

		return S_OK;
	}
}
