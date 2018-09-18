#pragma once

#include "Resource/Texture/TextureResource.h"
#include "Command/CommandList.h"

namespace D3D
{
	struct ShaderTextureInfo
	{
		WString								TextureName;
		WString								TexturePath;
		DescriptorHeapEntry					HeapEntry;
		SharedPointer<CTextureResource>		Resource;
		SharedPointer<RShaderResourceView>	ShaderResource;

		ShaderTextureInfo() = default;
		ShaderTextureInfo
		(
			const WString				& Name,
			const WString				& Path,
			const DescriptorHeapEntry	& Entry,
			CTextureResource			* pResource
		);
	};

	class CTextureManager : public CSingleton<CTextureManager>
	{
	public:

		typedef THashMap<WStringView, ShaderTextureInfo> TTextures;

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
			const WStringView & Name
		)	const;

	public:

		 CTextureManager();
		~CTextureManager();

		ErrorCode AddTexture
		(
			const WStringView & TexturePath,
			const WStringView & TextureName
		);

		ErrorCode GetOrAddTexture
		(
			const WStringView & TexturePath,
			const WStringView & TextureName
		)
		{
			return AddTexture(TexturePath, TextureName);
		}

		ErrorCode AddLookupPath
		(
			const WStringView & Path
		);

		ErrorCode GetOrAddTexture
		(
			const WStringView & TexturePath,
			const WStringView & TextureName,
			ShaderTextureInfo & pInfo,
			CTextureInitializationStream * Stream = NULL
		);

		ErrorCode GetOrAddTexture
		(
			const StringView & TexturePath,
			const StringView & TextureName,
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
			const StringView & TexturePath,
			const StringView & TextureName,
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
			const WStringView & TexturePath,
			const WStringView & TextureName,
			const RResource::InitializeOptions & Options,
			ShaderTextureInfo & Info,
			CTextureInitializationStream * Stream = NULL
		);

		inline ErrorCode CreateTexture
		(
			const StringView & TexturePath,
			const StringView & TextureName,
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