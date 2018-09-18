#include "Precompiled.h"

#include "Resource/Texture/TextureResource.h"

namespace D3D
{
	CTextureResource::CTextureResource()
	{
	}

	CTextureResource::~CTextureResource()
	{
	}

	ErrorCode CTextureResource::LoadTextureType(const CCommandListContext & CmdListCtx, const WString & FilePath, const D3D12_RESOURCE_STATES InitialState)
	{
		return LoadDDSTexture(CmdListCtx, FilePath, InitialState);
	}

	ErrorCode CTextureResource::LoadDDSTexture(const CCommandListContext & CmdListCtx, const WString & FilePath, const D3D12_RESOURCE_STATES InitialState)
	{
		ErrorCode Error;

		TUniquePtr<uint8_t[]> TextureMemory;

		ComPointer<IResource> TempResource;

		if ((Error = DirectX::LoadDDSTextureFromFile(
			DEVICE,
			FilePath.c_str(),
			&TempResource,
			TextureMemory,
			SubResourceData)))
		{
			return Error;
		}

		D3D12_RESOURCE_DESC ResourceDesc = TempResource->GetDesc();

		if ((Error = Create(InitializeOptions::Texture2D(
			ResourceDesc.Width,
			ResourceDesc.Height,
			ResourceDesc.Format,
			ResourceDesc.MipLevels,
			NULL,
			D3D12_RESOURCE_STATE_COPY_DEST))))
		{
			return Error;
		}

		CmdListCtx.CopyDataToTexture
		(
			this,
			SubResourceData.data(),
			0,
			SubResourceData.size()
		);

		return S_OK;
	}

	ErrorCode CTextureResource::LoadDDSTextureArray(const CCommandListContext & CmdListCtx, const TVector<WString> & TextureFilePathes, const D3D12_RESOURCE_STATES InitialState)
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
				if (!RResource::ResourceDescCompatible(ResourceDesc, ResourceDescLast))
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

		CmdListCtx.CopyDataToTexture
		(
			this,
			SubResourceData.data(),
			0,
			SubResourceData.size()
		);

		return S_OK;
	}
}
