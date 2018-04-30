#pragma once

#include "RawShaderResourceView.h"
#include "RawUnorderedAccessView.h"

#include "TextureResource.h"
#include "CommandList.h"

#include <File/File.h>
#include <LexicalCast.h>

#include <XML.h>

namespace D3D
{
	class CTextureMap
	{
	public:

		struct TextureData
		{
			WString Path;
			WString PathNormal;
			WString PathParallax;

			Uint8 Offset;
			Uint8 OffsetNormal;
			Uint8 OffsetParallax;

			inline TextureData() = default;
			inline TextureData
			(
				const WString & C
			) : Path(C)
			{}
			inline TextureData
			(
				const WString & C,
				const WString & N
			) : Path(C), PathNormal(N)
			{}
			inline TextureData
			(
				const WString & C,
				const WString & N,
				const WString & P
			) : Path(C), PathNormal(N), PathParallax(P)
			{}
		};

		typedef TMap<Uint8, TextureData>	TTextureSet;
		typedef TVector<Uint8>				TTextureMap;

	private:

		TTextureSet TextureSet;
		TTextureMap TextureMap;

		Uint8 NumTextures			= 0;
		Uint8 NumTexturesNormal		= 0;
		Uint8 NumTexturesParallax	= 0;

	protected:

		UniquePointer<RShaderResourceView>	TextureAtlasSRV;
		UniquePointer<RShaderResourceView>	TextureAtlasNormalsSRV;
		UniquePointer<RShaderResourceView>	TextureAtlasParallaxSRV;
		UniquePointer<RUnorderedAccessView> TextureAtlasUAV;

		Uint BufferSize = 0;

	public:

		inline RShaderResourceView * GetTextureAtlasSRV() const
		{
			return TextureAtlasSRV.Get();
		}

		inline RShaderResourceView * GetTextureAtlasNormalsSRV() const
		{
			return TextureAtlasSRV.Get();
		}

		inline RShaderResourceView * GetTextureAtlasParallaxSRV() const
		{
			return TextureAtlasSRV.Get();
		}

		inline const TTextureSet & GetTextureSet() const
		{
			return TextureSet;
		}

		inline const TTextureMap & GetTextureMap() const
		{
			return TextureMap;
		}

		inline const TextureData * GetTextureData
		(
			const Uint8 TexIndex
		)	const
		{
			auto Iter = TextureSet.find(TexIndex);

			if (Iter != TextureSet.end())
			{
				return &Iter->second;
			}

			return NULL;
		}

		inline const Uint8 GetTextureIndex
		(
			const Uint Position
		)	const
		{
			return TextureMap[Position];
		}

	private:

		void ReadTextureSet
		(
			const boost::property_tree::basic_ptree<WString, WString> & Tree
		);

		ErrorCode LoadTextures
		(
			const CCommandListContext	& CmdListCtx,
			const DescriptorHeapEntry	* pDescriptorHeapEntry,
			const DescriptorHeapEntry	* pDescriptorHeapEntryNormals,
			const DescriptorHeapEntry	* pDescriptorHeapEntryParallax
		);

		ErrorCode CreateTextureAtlas
		(
			const UINT Width,
			const UINT Height,
			const DescriptorHeapEntry * pDescriptorHeapEntry
		);

	public:

		CTextureMap() = default;

		ErrorCode InitializeResources
		(
			const CCommandListContext	&	CmdListCtx,
			const UINT						Width,
			const UINT						Height,
			const DescriptorHeapEntry	*	pDescriptorHeapEntryUAV,
			const DescriptorHeapEntry	*	pDescriptorHeapEntrySRV,
			const DescriptorHeapEntry	*	pDescriptorHeapEntryNormalsSRV,
			const DescriptorHeapEntry	*	pDescriptorHeapEntryParallaxSRV
		);

		ErrorCode ReadTextureSet
		(
			const WString & FilePath
		);

		ErrorCode ReadTextureMap
		(
			const CCommandListContext	& CmdListCtx,
			const WString				& FilePath
		);
	};
}