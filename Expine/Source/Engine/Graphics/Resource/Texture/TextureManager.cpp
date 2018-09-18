#include "Precompiled.h"

#include "Resource/Texture/TextureManager.h"
#include "Raw/RawShaderResourceView.h"
#include "Command/CommandQueue.h"
#include <locale.h>
#include <Utils/XML.h>

namespace D3D
{
	ShaderTextureInfo::ShaderTextureInfo
	(
		const WString				& Name,
		const WString				& Path,
		const DescriptorHeapEntry	& Entry,
		CTextureResource			* pResource
	)
	{
		TextureName = Name;
		TexturePath = Path;
		HeapEntry = Entry;
		Resource = pResource;
	}

	const ShaderTextureInfo * CTextureManager::GetShaderTextureInfo(const WStringView & Name) const
	{
		const auto Iter = Textures.find(Name);

		if (Iter == Textures.end())
		{
			return NULL;
		}

		return &Iter->second;
	}

	CTextureManager::CTextureManager()
	{
		SRVDescriptorHeaps	= new CCPUDescriptorHeapPool(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		CommandContext		= new CCommandListContext();

		ThrowOnError(CommandContext->Create(true, D3D12_COMMAND_LIST_TYPE_COPY));

		CommandContext->Get()->SetName(L"CMDList Texture Manager");
	}

	CTextureManager::~CTextureManager()
	{
	}

	ErrorCode CTextureManager::AddTexture(const WStringView & TexturePath, const WStringView & TextureName)
	{
		ErrorCode Error;

		if (Textures.find(TexturePath) != Textures.end())
		{
			return ERROR_ALREADY_EXISTS;
		}

		UniquePointer<CTextureResource> Texture = new CTextureResource();

		if ((Error = Texture->LoadTextureType(CommandContext.GetRef(), TexturePath)))
		{
			return Error;
		}

		Textures.emplace(std::piecewise_construct, std::forward_as_tuple<>(TexturePath), std::forward_as_tuple<>(
			TextureName, 
			TexturePath,
			SRVDescriptorHeaps->RequestDescriptorHeapRange(),
			Texture.Detach()
		));

		return S_OK;
	}

	ErrorCode CTextureManager::AddLookupPath(const WStringView & Path)
	{
		return ErrorCode();
	}

	ErrorCode CTextureManager::GetOrAddTexture(const WStringView & TexturePath, const WStringView & TextureName, ShaderTextureInfo & Info, CTextureInitializationStream * Stream)
	{
		ErrorCode Error;

		TTextures::iterator Item = Textures.find(TexturePath);

		if (Item != Textures.end())
		{
			Info = Item->second;
			return S_OK;
		}

		UniquePointer<CTextureResource> Texture = new CTextureResource();

		if (Stream)
		{
			if ((Error = Texture->LoadTextureType(Stream->CommandContext.GetRef(), TexturePath)))
			{
				return Error;
			}
		}
		else
		{
			if ((Error = Texture->LoadTextureType(CommandContext.GetRef(), TexturePath)))
			{
				return Error;
			}
		}

		DescriptorHeapRange DHRange = SRVDescriptorHeaps->RequestDescriptorHeapRange();

		Info = Textures.emplace(std::piecewise_construct, std::forward_as_tuple<>(TexturePath), std::forward_as_tuple<>(
			TextureName,
			TexturePath,
			DHRange,
			Texture.Detach()
			)).first.value();

		Info.ShaderResource = new RShaderResourceView();

		if ((Error = Info.ShaderResource->CreateFromResource(Info.Resource.As<RResource>(), Info.HeapEntry)))
		{
			return Error;
		}
	
		if (!Stream)
		{
			RGrpCommandList * CmdList = CommandContext.Get();

			if ((Error = CmdList->Close()))
			{
				return Error;
			}

			CCommandQueueCopy::Instance().ExecuteCommandList(CommandContext.GetRef());

			if ((Error = CmdList->Reset()))
			{
				return Error;
			}
		}

		return S_OK;
	}

	void CTextureManager::LoadTexturesFromDirectory(const WString & DirectoryPath)
	{

	}

	ErrorCode CTextureManager::ReadTextureList(const WString & XMLPath)
	{
		ErrorCode Error;

		XML::XMLDocumentWide Document;

		try
		{
			Document.ParseXML(XMLPath);
		}
		catch (const XML::XMLParseError & Exception)
		{
			CErrorLog::Log() << Exception.what() << CErrorLog::EndLine;
			return ERROR_FILE_INVALID;
		}

		for (const auto & Iter : Document.GetRoot())
		{
			const auto NameChild = Iter.second.get_child_optional(L"Name").get_ptr();

			if (!NameChild)
			{
				CErrorLog::Log<LogError>() << "Unable to get Name field of texture." << CErrorLog::EndLine;
				continue;
			}

			const auto PathChild = Iter.second.get_child_optional(L"Path").get_ptr();

			if (!PathChild)
			{
				CErrorLog::Log<LogError>() << "Unable to get Path field of texture." << CErrorLog::EndLine;
				continue;
			}

			const auto NameValue = NameChild->get_value_optional<WString>();

			if (!NameValue)
			{
				CErrorLog::Log<LogError>() << "Unable to get Name field of texture." << CErrorLog::EndLine;
				continue;
			}

			const auto PathValue = PathChild->get_value_optional<WString>();

			if (!PathValue)
			{
				CErrorLog::Log<LogError>() << "Unable to get Path field of texture." << CErrorLog::EndLine;
				continue;
			}

			if ((Error = AddTexture(*PathValue, *NameValue)))
			{
				CErrorLog::Log<LogError>() << "Unable to add texture." << Error << CErrorLog::EndLine;
				return Error;
			}
		}

		return S_OK;
	}

	ErrorCode CTextureManager::CreateTexture(const WString & TexturePath, const WString & TextureName, const RResource::InitializeOptions& Options, const size_t StartSubResource, const size_t NumSubResources, D3D12_SUBRESOURCE_DATA * SubResourceData, ShaderTextureInfo & Info, CTextureInitializationStream * Stream)
	{
		ErrorCode Error;

		UniquePointer<CTextureResource> Texture = new CTextureResource();

		if ((Error = Texture->Create(Options)))
		{
			return Error;
		}

		if (!Stream)
		{
			CommandContext->CopyDataToTexture
			(
				Texture.Get(),
				SubResourceData,
				StartSubResource,
				NumSubResources
			);
		}
		else
		{
			Stream->CommandContext->CopyDataToTexture
			(
				Texture.Get(),
				SubResourceData,
				StartSubResource,
				NumSubResources
			);

			Stream->NeedsDispatch = true;
		}

		auto Iter = Textures.find(TexturePath);

		if (Iter != Textures.end())
		{
			Info				= Iter.value();
			Info.TextureName	= TextureName;
			Info.Resource		= Texture.Detach();
		}
		else
		{
			Info = Textures.emplace(std::piecewise_construct, std::forward_as_tuple<>(TexturePath), std::forward_as_tuple<>(
				TextureName,
				TexturePath,
				SRVDescriptorHeaps->RequestDescriptorHeapRange(),
				Texture.Detach())).first->second;
		}

		Info.ShaderResource = new RShaderResourceView();

		if ((Error = Info.ShaderResource->CreateFromResource(Info.Resource.As<RResource>(), Info.HeapEntry)))
		{
			return Error;
		}

		if (!Stream)
		{
			RGrpCommandList * CmdList = CommandContext.Get();

			if ((Error = CmdList->Close()))
			{
				return Error;
			}

			CCommandQueueCopy::Instance().ExecuteCommandList(CommandContext.GetRef());

			if ((Error = CmdList->Reset()))
			{
				return Error;
			}
		}

		Info.Resource->Get()->SetName(TextureName.c_str());

		return S_OK;
	}

	ErrorCode CTextureManager::CreateTexture(const WStringView & TexturePath, const WStringView & TextureName, const RResource::InitializeOptions & Options, ShaderTextureInfo & Info, CTextureInitializationStream * Stream)
	{
		ErrorCode Error;

		UniquePointer<CTextureResource> Texture = new CTextureResource();

		if ((Error = Texture->Create(Options)))
		{
			return Error;
		}

		auto Iter = Textures.find(TexturePath);

		if (Iter != Textures.end())
		{
			Info				= Iter.value();
			Info.TextureName	= TextureName;
			Info.Resource		= Texture.Detach();
		}
		else
		{
			Info = Textures.emplace(std::piecewise_construct, std::forward_as_tuple<>(TexturePath), std::forward_as_tuple<>(
				TextureName,
				TexturePath,
				SRVDescriptorHeaps->RequestDescriptorHeapRange(),
				Texture.Detach())).first->second;
		}

		Info.ShaderResource = new RShaderResourceView();

		if ((Error = Info.ShaderResource->CreateFromResource(Info.Resource.As<RResource>(), Info.HeapEntry)))
		{
			return Error;
		}

		Info.Resource->Get()->SetName(TextureName.data());

		return S_OK;
	}

	void CTextureManager::CopyDataToResource(ShaderTextureInfo & Info, const size_t StartSubResource, const size_t NumSubResources, D3D12_SUBRESOURCE_DATA * SubResourceData)
	{
		CommandContext->CopyDataToTexture
		(
			Info.Resource.Get(),
			SubResourceData,
			StartSubResource,
			NumSubResources
		);
	}

	void CTextureManager::FinishExecution()
	{
		CommandContext->WaitForCompletion();
	}

	CTextureManager::CTextureInitializationStream::CTextureInitializationStream()
	{
		CommandContext = new CCommandListContext();
		{
			ThrowOnError(CommandContext->Create(true, D3D12_COMMAND_LIST_TYPE_COPY));
		}
	}

	CTextureManager::CTextureInitializationStream::CTextureInitializationStream(SharedPointer<CCommandListContext>& CmdListCtx) :
		CommandContext(CmdListCtx)
	{
	}

	CTextureManager::CTextureInitializationStream::~CTextureInitializationStream()
	{
		Dispatch();
	}

	void CTextureManager::CTextureInitializationStream::Dispatch()
	{
		if (NeedsDispatch)
		{
			NeedsDispatch = false;

			RGrpCommandList * CmdList = CommandContext.Get();

			ThrowOnError(CmdList->Close());
			CommandContext->Finish(0);
			ThrowOnError(CmdList->Reset());
		}
	}
}
