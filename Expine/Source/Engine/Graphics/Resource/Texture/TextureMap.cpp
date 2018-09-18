#include "Precompiled.h"

#include "Resource/Texture/TextureMap.h"
#include <Utils/File/File.h>
#include <Utils/Parsing/RapidXML.h>
#include <optional>

namespace D3D
{
	template<class Type>
	std::optional<Type> GetXMLValue(const rapidxml::xml_node<char> * Node)
	{
		Type Result;

		if (!Node)
		{
			return {};
		}

		StringValue<StringView> Value(Node->value());

		if (!(Value >> Result))
		{
			return {};
		}

		return Result;
	}

	bool CTextureMap::ReadTextureSet(const rapidxml::xml_node<char> * Tree)
	{
		const auto TexId = GetXMLValue<Uint>(Tree->first_node("Id"));

		if (!TexId || *TexId >= UINT8_MAX - 1)
		{
			CErrorLog::Log() << "Id invalid (Max: 0xFE). " << *TexId << CErrorLog::EndLine;
			return false;
		}

		auto Path = GetXMLValue<WString>(Tree->first_node("Path"));
		auto PathNormal = GetXMLValue<WString>(Tree->first_node("PathNormal"));
		auto PathParallax = GetXMLValue<WString>(Tree->first_node("PathParallax"));

		if (!Path)
		{
			CErrorLog::Log() << "No path node for Id. " << *TexId << CErrorLog::EndLine;
			return false;
		}

		if (TextureSet.find(*TexId) == TextureSet.end())
		{
			TextureData & TexData = TextureSet[*TexId];

			TexData.Offset			= Path->empty()			? -1 : NumTextures++;
			TexData.OffsetNormal	= PathNormal->empty()	? -1 : NumTexturesNormal++;
			TexData.OffsetParallax	= PathParallax->empty()	? -1 : NumTexturesParallax++;
			TexData.Path			= std::move(*Path);
			TexData.PathNormal		= std::move(*PathNormal);
			TexData.PathParallax	= std::move(*PathParallax);
		}
	}

	ErrorCode CTextureMap::ReadTextureMap(const CCommandListContext & CmdListCtx, const WStringView & FilePath)
	{
		ErrorCode Error;

#ifdef TEXTURE_ATLAS_UAV
		if (TextureAtlasUAV == NULL)
		{
			return E_FAIL;
		}
#endif

		File::CFile File(FilePath);

		if (!File.OpenFileRead())
		{
			return ERROR_FILE_INVALID;
		}

#ifdef TEXTURE_ATLAS_UAV
		if (File.GetFileSize() != BufferSize)
		{
			return E_INVALIDARG;
		}

		if (File.ReadFileContentInto(TextureMap) != File::ErrorNone)
		{
			return ERROR_FILE_INVALID;
		}

		TextureAtlasUAV->GetResource()->AsCopyDest(CmdListCtx);
		{
			D3D12_SUBRESOURCE_DATA SubResourceData;
			{
				SubResourceData.pData		= TextureMap.data();
				SubResourceData.RowPitch	= 1;
				SubResourceData.SlicePitch	= TextureMap.size();
			}

			CmdListCtx.CopyDataToTexture(TextureAtlasUAV->GetResource(), &SubResourceData, 0, 1);
		}

		TextureAtlasUAV->GetResource()->AsUnorderedAccessView(CmdListCtx);
#else
		if (File.ReadFileContentInto(TextureMap) != File::ErrorNone)
		{
			return ERROR_FILE_INVALID;
		}
#endif

		return S_OK;
	}

	ErrorCode CTextureMap::LoadTextures(const CCommandListContext & CmdListCtx, const DescriptorHeapEntry * pDescriptorHeapEntry, const DescriptorHeapEntry * pDescriptorHeapEntryNormals, const DescriptorHeapEntry * pDescriptorHeapEntryParallax)
	{
		ErrorCode Error;

		if (!pDescriptorHeapEntry)
		{
			return S_OK;
		}

		SharedPointer<CTextureResource> TextureAtlas = new CTextureResource();

		TVector<WString> TextureFilePathes;
		{
			for (auto Iter = std::rbegin(TextureSet); Iter != std::rend(TextureSet); ++Iter)
			{
				if (!Iter->second.Path.empty())
				{
					TextureFilePathes.push_back(Iter->second.Path);
				}
			}
		}

		if (!TextureFilePathes.empty())
		{
			if ((Error = TextureAtlas->LoadDDSTextureArray(CmdListCtx, TextureFilePathes)))
			{
				return Error;
			}

			TextureAtlasSRV = new RShaderResourceView();

			if ((Error = TextureAtlasSRV->CreateFromResource(TextureAtlas.As<RResource>(), *pDescriptorHeapEntry)))
			{
				return Error;
			}
		}

		if (pDescriptorHeapEntryNormals)
		{
			SharedPointer<CTextureResource> TextureAtlasNormals = new CTextureResource();

			TextureFilePathes.clear();
			{
				for (auto Iter = std::rbegin(TextureSet); Iter != std::rend(TextureSet); ++Iter)
				{
					if (!Iter->second.Path.empty())
					{
						TextureFilePathes.push_back(Iter->second.PathNormal);
					}
				}
			}

			if (!TextureFilePathes.empty())
			{
				if ((Error = TextureAtlasNormals->LoadDDSTextureArray(CmdListCtx, TextureFilePathes)))
				{
					return Error;
				}

				TextureAtlasNormalsSRV = new RShaderResourceView();

				if ((Error = TextureAtlasNormalsSRV->CreateFromResource(TextureAtlasNormals.As<RResource>(), *pDescriptorHeapEntryNormals)))
				{
					return Error;
				}
			}
		}

		if (pDescriptorHeapEntryParallax)
		{
			SharedPointer<CTextureResource> TextureAtlasParallax = new CTextureResource();

			TextureFilePathes.clear();
			{
				for (auto Iter = std::rbegin(TextureSet); Iter != std::rend(TextureSet); ++Iter)
				{
					if (!Iter->second.Path.empty())
					{
						TextureFilePathes.push_back(Iter->second.PathParallax);
					}
				}
			}

			if (!TextureFilePathes.empty())
			{
				if ((Error = TextureAtlasParallax->LoadDDSTextureArray(CmdListCtx, TextureFilePathes)))
				{
					return Error;
				}

				TextureAtlasParallaxSRV = new RShaderResourceView();

				if ((Error = TextureAtlasParallaxSRV->CreateFromResource(TextureAtlasParallax.As<RResource>(), *pDescriptorHeapEntryParallax)))
				{
					return Error;
				}
			}
		}

		return S_OK;
	}

	ErrorCode CTextureMap::CreateTextureAtlas(const UINT Width, const UINT Height, const DescriptorHeapEntry * pDescriptorHeapEntry)
	{
		ErrorCode Error;

		SharedPointer<RResource> ResourceUAV = new RResource();

		if ((Error = ResourceUAV->Create(RResource::InitializeOptions::Texture2D(
			Width,
			Height,
			DXGI_FORMAT_R8_UINT,
			1,
			NULL,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS))))
		{
			return Error;
		}

		TextureAtlasUAV = new RUnorderedAccessView();

		if ((Error = TextureAtlasUAV->CreateFromResource(ResourceUAV, *pDescriptorHeapEntry)))
		{
			return Error;
		}

		BufferSize = Width * Height;

		return S_OK;
	}

	ErrorCode CTextureMap::InitializeResources(const CCommandListContext & CmdListCtx, const UINT Width, const UINT Height, const DescriptorHeapEntry * pDescriptorHeapEntryUAV, const DescriptorHeapEntry * pDescriptorHeapEntrySRV, const DescriptorHeapEntry * pDescriptorHeapEntryNormalsSRV, const DescriptorHeapEntry * pDescriptorHeapEntryParallaxSRV)
	{
		ErrorCode Error;

		if ((Error = LoadTextures(CmdListCtx, 
			pDescriptorHeapEntrySRV, 
			pDescriptorHeapEntryNormalsSRV, 
			pDescriptorHeapEntryParallaxSRV)))
		{
			return Error;
		}

		if (pDescriptorHeapEntryUAV)
		{
			if ((Error = CreateTextureAtlas(Width, Height, pDescriptorHeapEntryUAV)))
			{
				return Error;
			}
		}

		return S_OK;
	}

	ErrorCode CTextureMap::ReadTextureSet(const WStringView & FilePath)
	{
		File::CFile File(FilePath);

		if (!File.OpenFileRead())
		{
			CErrorLog::Log() << "Unable to open file: " << FilePath << CErrorLog::EndLine;
			return ERROR_FILE_INVALID;
		}

		rapidxml::xml_document<char> Document;

		try
		{
			Document.parse<rapidxml::parse_fastest>(File.GetContentRef<char>().data());
		}
		catch (const rapidxml::parse_error& Error)
		{
			CErrorLog::Log() << "Error while parsing file: " << Error.what() << CErrorLog::EndLine;
			return E_FAIL;
		}

		rapidxml::xml_node<char> * Root = Document.parent();
		
		if (!Root)
		{
			return E_FAIL;
		}
		
		rapidxml::xml_node<char> * Textures = Root->first_node("Textures");

		if (!Textures)
		{
			return E_FAIL;
		}

		if (Textures == NULL)
		{
			return E_FAIL;
		}

		for (rapidxml::xml_node<char> * Texture = Textures->first_node(); Texture; Texture = Texture->next_sibling())
		{
			if (!ReadTextureSet(Texture))
			{
				return E_FAIL;
			}
		}

		return S_OK;
	}
}
