#pragma once

#include "RawResource.h"
#include "CommandList.h"

#include <DXTK\DDSTextureLoader.h>

namespace D3D
{
	class CTextureResource : public RResource
	{
	private:

		TVector<D3D12_SUBRESOURCE_DATA> SubResourceData;

	public:

		CTextureResource();
		~CTextureResource();

		ErrorCode LoadTextureType
		(
			const CCommandListContext	& CmdListCtx,
			const WString				& FilePath,
			const D3D12_RESOURCE_STATES	  InitialState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		);

		ErrorCode LoadDDSTexture
		(
			const CCommandListContext	& CmdListCtx,
			const WString				& FilePath,
			const D3D12_RESOURCE_STATES	  InitialState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		);

		ErrorCode LoadDDSTextureArray
		(
			const CCommandListContext	& CmdListCtx,
			const TVector<WString>		& TextureFilePathes,
			const D3D12_RESOURCE_STATES	  InitialState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		);
	};

	class CTextureResourcePair : public RResource
	{
	private:

		SharedPointer<RResource> ResourceUpload;

	private:

		TVector<D3D12_SUBRESOURCE_DATA> SubResourceData;

	public:

		ErrorCode LoadTextureType
		(
			const RGrpCommandList		& CmdList,
			const WString				& FilePath,
			const D3D12_RESOURCE_STATES	  InitialState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		)
		{
			return LoadDDSTexture(CmdList, FilePath, InitialState);
		}

		ErrorCode LoadDDSTexture
		(
			const RGrpCommandList		& CmdList,
			const WString				& FilePath,
			const D3D12_RESOURCE_STATES	  InitialState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		)
		{
			ErrorCode Error;

			TUniquePtr<uint8_t[]> TextureMemory;

			if ((Error = DirectX::LoadDDSTextureFromFile(
				DEVICE,
				FilePath.c_str(),
				&Resource,
				TextureMemory,
				SubResourceData)))
			{
				return Error;
			}

			const UINT64 BufferSize = GetRequiredIntermediateSize
			(
				Resource.Get(),
				0,
				SubResourceData.size()
			);

			ResourceUpload = new RResource();

			if ((Error = ResourceUpload->Create(RResource::InitializeOptions::Buffer(
				BufferSize,
				D3D12_HEAP_TYPE_UPLOAD,
				D3D12_HEAP_FLAG_NONE,
				D3D12_RESOURCE_STATE_GENERIC_READ))))
			{
				return Error;
			}

			UpdateSubresources
			(
				CmdList,
				Resource.Get(),
				ResourceUpload.GetRef(),
				0,
				0,
				SubResourceData.size(),
				SubResourceData.data()
			);

			CmdList->ResourceBarrier(1,
				&CD3DX12_RESOURCE_BARRIER::Transition(
					Resource.Get(),
					D3D12_RESOURCE_STATE_COPY_DEST,
					InitialState)
			);

			ResourceState = InitialState;

			return S_OK;
		}

		ErrorCode LoadDDSTextureArray
		(
			const RGrpCommandList		& CmdList,
			const TVector<WString>		& TextureFilePathes,
			const D3D12_RESOURCE_STATES	  InitialState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		)
		{
			ErrorCode Error;

			if (TextureFilePathes.empty())
			{
				return E_INVALIDARG;
			}

			D3D12_RESOURCE_DESC	ResourceDesc;
			D3D12_RESOURCE_DESC ResourceDescLast;

			TVector<TUniquePtr<Uint8[]> > TextureMemory(TextureFilePathes.size());

			for (UINT N = TextureFilePathes.size() - 1; N != -1; --N)
			{
				ID3D12Resource * TempTexture;

				if (TextureFilePathes[N].empty())
				{
					continue;
				}

				if ((Error = DirectX::LoadDDSTextureFromFile(
					DEVICE,
					TextureFilePathes[N].c_str(),
					&TempTexture,
					TextureMemory[N],
					SubResourceData)))
				{
					return Error;
				}

				ResourceDesc = TempTexture->GetDesc();

				if (N != TextureFilePathes.size() - 1)
				{
					if (ResourceDesc.Format != ResourceDescLast.Format ||
						ResourceDesc.Height != ResourceDescLast.Height ||
						ResourceDesc.Width != ResourceDescLast.Width ||
						ResourceDesc.Dimension != ResourceDescLast.Dimension ||
						ResourceDesc.DepthOrArraySize != ResourceDescLast.DepthOrArraySize ||
						ResourceDesc.MipLevels != ResourceDescLast.MipLevels)
					{
						return E_FAIL;
					}
				}
				else
				{
					ResourceDescLast = ResourceDesc;
				}
			}

			if ((Error = Create(InitializeOptions::Texture2DArray(
				ResourceDesc.Width,
				ResourceDesc.Height,
				ResourceDesc.Format,
				TextureFilePathes.size(),
				ResourceDesc.MipLevels,
				NULL,
				D3D12_RESOURCE_STATE_COPY_DEST))))
			{
				return Error;
			}

			const UINT64 BufferSize = GetRequiredIntermediateSize
			(
				Resource.Get(),
				0,
				SubResourceData.size()
			);

			if (BufferSize == -1)
			{
				return E_FAIL;
			}

			ResourceUpload = new RResource();

			if ((Error = ResourceUpload->Create(RResource::InitializeOptions::Buffer(
				BufferSize,
				D3D12_HEAP_TYPE_UPLOAD,
				D3D12_HEAP_FLAG_NONE,
				D3D12_RESOURCE_STATE_GENERIC_READ))))
			{
				return Error;
			}

			UpdateSubresources
			(
				CmdList,
				Resource.Get(),
				ResourceUpload.GetRef(),
				0,
				0,
				SubResourceData.size(),
				SubResourceData.data()
			);

			CmdList->ResourceBarrier(1,
				&CD3DX12_RESOURCE_BARRIER::Transition(
					Resource.Get(),
					D3D12_RESOURCE_STATE_COPY_DEST,
					InitialState)
			);

			ResourceState = InitialState;

			return S_OK;
		}
	};
}