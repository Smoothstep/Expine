#pragma once

#include "TextureResource.h"
#include "CommandList.h"

namespace D3D
{
	struct ShaderTextureInfo
	{
		WString								TextureName;
		WString								TexturePath;
		DescriptorHeapEntry					HeapEntry;
		SharedPointer<CTextureResource>		Resource;
		SharedPointer<RShaderResourceView>	ShaderResource;

		inline ShaderTextureInfo() = default;
		inline ShaderTextureInfo
		(
			const WString				& Name,
			const WString				& Path,
			const DescriptorHeapEntry	& Entry,
			CTextureResource			* pResource
		)
		{
			TextureName		= Name;
			TexturePath		= Path;
			HeapEntry		= Entry;
			Resource		= pResource;
		}
	};

	class CTextureManager : public CSingleton<CTextureManager>
	{
	public:

		typedef THashMap<WString, ShaderTextureInfo> TTextures;

		class CTextureInitializationStream
		{
			friend class CTextureManager;

		private:

			SharedPointer<CCommandListContext> CommandContext;
			TAtomic<bool> NeedsDispatch;

		public:

			 CTextureInitializationStream();
			 CTextureInitializationStream(SharedPointer<CCommandListContext> & CmdListCtx);
			~CTextureInitializationStream();

			void Dispatch();
		};

	private:

		TTextures Textures;

	protected:

		SharedPointer<CCommandListContext> CommandContext;
	
	private:

		UniquePointer<CCPUDescriptorHeapPool> SRVDescriptorHeaps;

	public:

		const ShaderTextureInfo * GetShaderTextureInfo
		(
			const WString & Name
		)	const;

	public:

		 CTextureManager();
		~CTextureManager();

		ErrorCode AddTexture
		(
			const WString & TexturePath,
			const WString & TextureName
		);

		ErrorCode GetOrAddTexture
		(
			const WString & TexturePath,
			const WString & TextureName
		)
		{
			return AddTexture(
				WString(TexturePath.begin(), TexturePath.end()),
				WString(TextureName.begin(), TextureName.end()));
		}

		ErrorCode AddLookupPath
		(
			const WString & Path
		);

		ErrorCode GetOrAddTexture
		(
			const WString & TexturePath,
			const WString & TextureName,
			ShaderTextureInfo & pInfo,
			CTextureInitializationStream * Stream = NULL
		);

		ErrorCode GetOrAddTexture
		(
			const String & TexturePath,
			const String & TextureName,
			ShaderTextureInfo & Info,
			CTextureInitializationStream * Stream = NULL
		)
		{
			return GetOrAddTexture(
				WString(TexturePath.begin(), TexturePath.end()),
				WString(TextureName.begin(), TextureName.end()),
				Info,
				Stream);
		}

		void LoadTexturesFromDirectory
		(
			const WString & DirectoryPath
		);

		ErrorCode ReadTextureList
		(
			const WString & XMLPath
		);

		ErrorCode CreateTexture
		(
			const WString & TexturePath,
			const WString & TextureName,
			const RResource::InitializeOptions & Options,
			const size_t StartSubResource,
			const size_t NumSubResources,
			D3D12_SUBRESOURCE_DATA * SubResourceData,
			ShaderTextureInfo & Info,
			CTextureInitializationStream * Stream = NULL
		);

		inline ErrorCode CreateTexture
		(
			const String & TexturePath,
			const String & TextureName,
			const RResource::InitializeOptions & Options,
			const size_t StartSubResource,
			const size_t NumSubResources,
			D3D12_SUBRESOURCE_DATA * SubResourceData,
			ShaderTextureInfo & Info,
			CTextureInitializationStream * Stream = NULL
		)
		{
			return CreateTexture(
				WString(TexturePath.begin(), TexturePath.end()),
				WString(TextureName.begin(), TextureName.end()),
				Options,
				StartSubResource,
				NumSubResources,
				SubResourceData,
				Info,
				Stream);
		}

		ErrorCode CreateTexture
		(
			const WString & TexturePath,
			const WString & TextureName,
			const RResource::InitializeOptions & Options,
			ShaderTextureInfo & Info,
			CTextureInitializationStream * Stream = NULL
		);

		inline ErrorCode CreateTexture
		(
			const String & TexturePath,
			const String & TextureName,
			const RResource::InitializeOptions & Options,
			ShaderTextureInfo & Info,
			CTextureInitializationStream * Stream = NULL
		)
		{
			return CreateTexture(
				WString(TexturePath.begin(), TexturePath.end()),
				WString(TextureName.begin(), TextureName.end()),
				Options,
				Info,
				Stream);
		}

		void CopyDataToResource
		(
			ShaderTextureInfo & Info,
			const size_t StartSubResource,
			const size_t NumSubResources,
			D3D12_SUBRESOURCE_DATA * SubResourceData
		);

		void FinishExecution();
	};
}