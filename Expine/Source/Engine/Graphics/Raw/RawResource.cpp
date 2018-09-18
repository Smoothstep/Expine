#include "Precompiled.h"

#include "Raw/RawResource.h"

namespace D3D
{
	RResource::InitializeOptions::InitializeOptions()
	{
		this->DepthOrArraySize		= 0;
		this->Dimension				= D3D12_RESOURCE_DIMENSION_UNKNOWN;
		this->Width					= 0;
		this->Height				= 0;
		this->Format				= DXGI_FORMAT_UNKNOWN;
		this->MipLevels				= 0;
		this->Flags					= D3D12_RESOURCE_FLAG_NONE;
		this->Layout				= D3D12_TEXTURE_LAYOUT_UNKNOWN;
		this->Alignment				= 0;
		this->SampleDesc.Count		= 1;
		this->SampleDesc.Quality	= 0;
	}

	RResource::InitializeOptions::InitializeOptions
	(
		D3D12_RESOURCE_DESC & ResourceDesc
	) :
		D3D12_RESOURCE_DESC(ResourceDesc)
	{}

	RResource::InitializeOptions::InitializeOptions
	(
		const UINT						Width,
		const UINT						Height,
		const DXGI_FORMAT				Format,
		const UINT						MipLevels,
		const D3D12_CLEAR_VALUE			ClearValue,
		const D3D12_RESOURCE_DIMENSION	Dimension,
		const UINT16					ArraySize,
		const D3D12_RESOURCE_STATES		InitState,
		const D3D12_RESOURCE_FLAGS		Flags,
		const UINT						SampleCount
	)
	{
		this->DepthOrArraySize		= ArraySize;
		this->Dimension				= Dimension;
		this->Width					= Width;
		this->Height				= Height;
		this->Format				= Format;
		this->MipLevels				= MipLevels;
		this->ClearValue			= ClearValue;
		this->InitialState			= InitState;
		this->Flags					= Flags;
		this->Layout				= D3D12_TEXTURE_LAYOUT_UNKNOWN;
		this->Alignment				= 0;
		this->SampleDesc.Count		= SampleCount;
		this->SampleDesc.Quality	= 0;
		this->Clear					= TRUE;
	}

	RResource::InitializeOptions::InitializeOptions
	(
		const UINT						Width,
		const UINT						Height,
		const DXGI_FORMAT				Format,
		const UINT						MipLevels,
		const Float*					ClearValue,
		const D3D12_RESOURCE_DIMENSION	Dimension,
		const UINT16					ArraySize,
		const D3D12_RESOURCE_STATES		InitState,
		const D3D12_RESOURCE_FLAGS		Flags,
		const UINT						SampleCount
	)
	{
		this->DepthOrArraySize		= ArraySize;
		this->Dimension				= Dimension;
		this->Width					= Width;
		this->Height				= Height;
		this->Format				= Format;
		this->MipLevels				= MipLevels;
		this->InitialState			= InitState;
		this->Flags					= Flags;
		this->Layout				= D3D12_TEXTURE_LAYOUT_UNKNOWN;
		this->Alignment				= 0;
		this->SampleDesc.Count		= SampleCount;
		this->SampleDesc.Quality	= 0;

		if (ClearValue)
		{
			this->Clear = TRUE;
			this->ClearValue = CD3DX12_CLEAR_VALUE(Format, ClearValue);
		}
	}

	RResource::InitializeOptions RResource::InitializeOptions::Buffer
	(
		const UINT64					Size,
		const D3D12_HEAP_TYPE			HeapType,
		const D3D12_HEAP_FLAGS			HeapFlags,
		const D3D12_RESOURCE_STATES		ResourceState,
		const D3D12_RESOURCE_FLAGS		ResourceFlags,
		const DXGI_FORMAT				Format
	)
	{
		InitializeOptions Options(CD3DX12_RESOURCE_DESC::Buffer(Size, ResourceFlags));
		{
			Options.HeapFlags		= HeapFlags;
			Options.HeapType		= HeapType;
			Options.InitialState	= ResourceState;
		}

		Options.Format = Format;

		return Options;
	}

	RResource::InitializeOptions RResource::InitializeOptions::DepthStencil2D
	(
		const UINT						Width,
		const UINT						Height,
		const DXGI_FORMAT				Format,
		const FLOAT						DepthClearValue,
		const UINT						StencilClearValue,
		const UINT						MipLevels,
		const D3D12_RESOURCE_STATES		InitState,
		const UINT						SampleCount
	)
	{
		DXGI_FORMAT ClearFormat;

		switch (Format)
		{
			case DXGI_FORMAT_R32G8X24_TYPELESS:
			{
				ClearFormat = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
			}

			break;

			case DXGI_FORMAT_R32_TYPELESS:
			{
				ClearFormat = DXGI_FORMAT_D32_FLOAT;
			}

			break;

			case DXGI_FORMAT_R24G8_TYPELESS:
			{
				ClearFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
			}

			break;

			case DXGI_FORMAT_R16_TYPELESS:
			{
				ClearFormat = DXGI_FORMAT_D16_UNORM;
			}

			break;

			default:
			{
				ClearFormat = Format;
			}
		}

		D3D12_CLEAR_VALUE DepthOptimizedClearValue = {};
		{
			DepthOptimizedClearValue.Format					= ClearFormat;
			DepthOptimizedClearValue.DepthStencil.Depth		= DepthClearValue;
			DepthOptimizedClearValue.DepthStencil.Stencil	= StencilClearValue;
		}

		return InitializeOptions(Width, Height, Format, MipLevels, DepthOptimizedClearValue, D3D12_RESOURCE_DIMENSION_TEXTURE2D, 1, InitState, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, SampleCount);
	}

	RResource::InitializeOptions RResource::InitializeOptions::DepthStencil2D
	(
		const UINT						Width,
		const UINT						Height,
		const DXGI_FORMAT				Format,
		const UINT						MipLevels,
		const D3D12_RESOURCE_STATES		InitState,
		const UINT						SampleCount,
		const FLOAT						ClearColor
	)
	{
		DXGI_FORMAT ClearFormat;

		switch (Format)
		{
			case DXGI_FORMAT_R32G8X24_TYPELESS:
			{
				ClearFormat = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
			}

			break;

			case DXGI_FORMAT_R32_TYPELESS:
			{
				ClearFormat = DXGI_FORMAT_D32_FLOAT;
			}

			break;

			case DXGI_FORMAT_R24G8_TYPELESS:
			{
				ClearFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
			}

			break;

			case DXGI_FORMAT_R16_TYPELESS:
			{
				ClearFormat = DXGI_FORMAT_D16_UNORM;
			}

			break;

			default:
			{
				ClearFormat = Format;
			}
		}

		D3D12_CLEAR_VALUE DepthOptimizedClearValue = {};
		{
			DepthOptimizedClearValue.Format					= ClearFormat;
			DepthOptimizedClearValue.DepthStencil.Depth		= ClearColor;
			DepthOptimizedClearValue.DepthStencil.Stencil	= 0;
		}

		return InitializeOptions(Width, Height, Format, MipLevels, DepthOptimizedClearValue, D3D12_RESOURCE_DIMENSION_TEXTURE2D, 1, InitState, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, SampleCount);
	}

	RResource::InitializeOptions RResource::InitializeOptions::Texture2D
	(
		const UINT						Width,
		const UINT						Height,
		const DXGI_FORMAT				Format,
		const UINT						MipLevels,
		const Float*					ClearValue,
		const D3D12_RESOURCE_STATES		InitState,
		const UINT						SampleCount
	)
	{
		return InitializeOptions(Width, Height, Format, MipLevels, ClearValue, D3D12_RESOURCE_DIMENSION_TEXTURE2D, 1, InitState, InitState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS ?
											D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS	:
			IsRenderTargetFormat(Format) ?	D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET													: D3D12_RESOURCE_FLAG_NONE,
			SampleCount);
	}

	RResource::InitializeOptions RResource::InitializeOptions::Texture2DArray
	(
		const UINT						Width,
		const UINT						Height,
		const DXGI_FORMAT				Format,
		const UINT						Depth,
		const UINT						MipLevels,
		const Float*					ClearValue,
		const D3D12_RESOURCE_STATES		InitState,
		const UINT						SampleCount
	)
	{
		return InitializeOptions(Width, Height, Format, MipLevels, ClearValue, D3D12_RESOURCE_DIMENSION_TEXTURE2D, Depth, InitState, InitState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS ?
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS :
			D3D12_RESOURCE_FLAG_NONE, 
			SampleCount);
	}

	RResource::InitializeOptions RResource::InitializeOptions::Texture3D
	(
		const UINT						Width,
		const UINT						Height,
		const DXGI_FORMAT				Format,
		const UINT						MipLevels,
		const Float*					ClearValue,
		const UINT16					ArraySize,
		const D3D12_RESOURCE_STATES		InitState,
		const UINT						SampleCount
	)
	{
		return InitializeOptions(Width, Height, Format, MipLevels, ClearValue, D3D12_RESOURCE_DIMENSION_TEXTURE3D, ArraySize, InitState, InitState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS ?
			D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS :
			D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
			SampleCount);
	}

	static TAtomic<UINT64> NextResourceId = 0;

	bool RResource::ResourceDescCompatible(const D3D12_RESOURCE_DESC & ResourceDescLhs, const D3D12_RESOURCE_DESC & ResourceDescRhs)
	{
		return		ResourceDescLhs.Format				== ResourceDescLhs.Format				&&
					ResourceDescLhs.Height				== ResourceDescLhs.Height				&&
					ResourceDescLhs.Width				== ResourceDescLhs.Width				&&
					ResourceDescLhs.Dimension			== ResourceDescLhs.Dimension			&&
					ResourceDescLhs.DepthOrArraySize	== ResourceDescLhs.DepthOrArraySize		&&
					ResourceDescLhs.MipLevels			== ResourceDescLhs.MipLevels;
	}

	RResource::RResource()
	{
		ResourceId = NextResourceId++;
	}

	ErrorCode RResource::Create(const InitializeOptions & Options)
	{
		ErrorCode Error = DEVICE->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(Options.HeapType),
			D3D12_HEAP_FLAG_NONE,
			&Options,
			Options.InitialState,
			Options.Clear ? &Options.ClearValue : NULL,
			IID_PPV_ARGS(&Resource)
		);

		if (Error)
		{
			return Error;
		}

		ResourceDesc = Options;
		ResourceState = Options.InitialState;
		DefaultClearValue = Options.ClearValue;

		return S_OK;
	}

	ErrorCode RResource::CreateReserved(const InitializeOptions & Options)
	{
		ErrorCode Error = DEVICE->CreateReservedResource
		(
			&Options,
			Options.InitialState,
			Options.Clear ? &Options.ClearValue : NULL,
			IID_PPV_ARGS(&Resource)
		);

		if (Error)
		{
			return Error;
		}

		ResourceDesc = Options;
		ResourceState = Options.InitialState;
		DefaultClearValue = Options.ClearValue;

		return S_OK;
	}

	ErrorCode RResource::CreatePlaced(const InitializeOptions & Options, const HeapEntry & HeapEntry)
	{
		if (!HeapEntry.Valid())
		{
			return E_INVALIDARG;
		}

		ErrorCode Error = DEVICE->CreatePlacedResource
		(
			*HeapEntry.Heap,
			HeapEntry.HeapOffset,
			&Options,
			Options.InitialState,
			Options.Clear ? &Options.ClearValue : NULL,
			IID_PPV_ARGS(&Resource)
		);

		if (Error)
		{
			return Error;
		}

		ResourceDesc = Options;
		ResourceState = Options.InitialState;
		DefaultClearValue = Options.ClearValue;

		return S_OK;
	}

	ErrorCode RResource::FromSwapChain(IDXGISwapChain * const pSwapChain, UINT Offset)
	{
		CHECK_NULL_ARG(pSwapChain);

		ErrorCode Error = pSwapChain->GetBuffer(Offset, IID_PPV_ARGS(&Resource));

		if (Error)
		{
			return Error;
		}

		ResourceState = D3D12_RESOURCE_STATE_COMMON;

		return S_OK;
	}

	D3D12_RESOURCE_DIMENSION RResource::DetermineDimension()
	{
		Ensure(Resource);

		D3D12_RESOURCE_DESC ResourceDesc = Resource->GetDesc();
		{
			return ResourceDesc.Dimension;
		}
	}

	inline UINT64 RResource::GetFormatByteSize(const DXGI_FORMAT Format)
	{
		switch (Format)
		{
			case DXGI_FORMAT_R32G32B32A32_TYPELESS:
			case DXGI_FORMAT_R32G32B32A32_FLOAT:
			case DXGI_FORMAT_R32G32B32A32_UINT:
			case DXGI_FORMAT_R32G32B32A32_SINT:
			{
				return 16;
			}

			case DXGI_FORMAT_R32G32B32_TYPELESS:
			case DXGI_FORMAT_R32G32B32_FLOAT:
			case DXGI_FORMAT_R32G32B32_UINT:
			case DXGI_FORMAT_R32G32B32_SINT:
			{
				return 12;
			}

			case DXGI_FORMAT_R16G16B16A16_TYPELESS:
			case DXGI_FORMAT_R16G16B16A16_FLOAT:
			case DXGI_FORMAT_R16G16B16A16_UNORM:
			case DXGI_FORMAT_R16G16B16A16_UINT:
			case DXGI_FORMAT_R16G16B16A16_SNORM:
			case DXGI_FORMAT_R16G16B16A16_SINT:
			case DXGI_FORMAT_R32G32_TYPELESS:
			case DXGI_FORMAT_R32G32_FLOAT:
			case DXGI_FORMAT_R32G32_UINT:
			case DXGI_FORMAT_R32G32_SINT:
			case DXGI_FORMAT_R32G8X24_TYPELESS:
			case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
			case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
			case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
			{
				return 8;
			}

			case DXGI_FORMAT_R10G10B10A2_TYPELESS:
			case DXGI_FORMAT_R10G10B10A2_UNORM:
			case DXGI_FORMAT_R10G10B10A2_UINT:
			case DXGI_FORMAT_R11G11B10_FLOAT:
			case DXGI_FORMAT_R8G8B8A8_TYPELESS:
			case DXGI_FORMAT_R8G8B8A8_UNORM:
			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			case DXGI_FORMAT_R8G8B8A8_UINT:
			case DXGI_FORMAT_R8G8B8A8_SNORM:
			case DXGI_FORMAT_R8G8B8A8_SINT:
			case DXGI_FORMAT_R16G16_TYPELESS:
			case DXGI_FORMAT_R16G16_FLOAT:
			case DXGI_FORMAT_R16G16_UNORM:
			case DXGI_FORMAT_R16G16_UINT:
			case DXGI_FORMAT_R16G16_SNORM:
			case DXGI_FORMAT_R16G16_SINT:
			case DXGI_FORMAT_R32_TYPELESS:
			case DXGI_FORMAT_D32_FLOAT:
			case DXGI_FORMAT_R32_FLOAT:
			case DXGI_FORMAT_R32_UINT:
			case DXGI_FORMAT_R32_SINT:
			case DXGI_FORMAT_R24G8_TYPELESS:
			case DXGI_FORMAT_D24_UNORM_S8_UINT:
			case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
			case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
			case DXGI_FORMAT_B8G8R8A8_UNORM:
			case DXGI_FORMAT_B8G8R8X8_UNORM:
			{
				return 4;
			}

			case DXGI_FORMAT_R8G8_TYPELESS:
			case DXGI_FORMAT_R8G8_UNORM:
			case DXGI_FORMAT_R8G8_UINT:
			case DXGI_FORMAT_R8G8_SNORM:
			case DXGI_FORMAT_R8G8_SINT:
			case DXGI_FORMAT_R16_TYPELESS:
			case DXGI_FORMAT_R16_FLOAT:
			case DXGI_FORMAT_D16_UNORM:
			case DXGI_FORMAT_R16_UNORM:
			case DXGI_FORMAT_R16_UINT:
			case DXGI_FORMAT_R16_SNORM:
			case DXGI_FORMAT_R16_SINT:
			case DXGI_FORMAT_B5G6R5_UNORM:
			case DXGI_FORMAT_B5G5R5A1_UNORM:
			{
				return 2;
			}

			case DXGI_FORMAT_R8_TYPELESS:
			case DXGI_FORMAT_R8_UNORM:
			case DXGI_FORMAT_R8_UINT:
			case DXGI_FORMAT_R8_SNORM:
			case DXGI_FORMAT_R8_SINT:
			case DXGI_FORMAT_A8_UNORM:
			{
				return 1;
			}

			case DXGI_FORMAT_BC2_TYPELESS:
			case DXGI_FORMAT_BC2_UNORM:
			case DXGI_FORMAT_BC2_UNORM_SRGB:
			case DXGI_FORMAT_BC3_TYPELESS:
			case DXGI_FORMAT_BC3_UNORM:
			case DXGI_FORMAT_BC3_UNORM_SRGB:
			case DXGI_FORMAT_BC5_TYPELESS:
			case DXGI_FORMAT_BC5_UNORM:
			case DXGI_FORMAT_BC5_SNORM:
			{
				return 16;
			}

			case DXGI_FORMAT_R1_UNORM:
			case DXGI_FORMAT_BC1_TYPELESS:
			case DXGI_FORMAT_BC1_UNORM:
			case DXGI_FORMAT_BC1_UNORM_SRGB:
			case DXGI_FORMAT_BC4_TYPELESS:
			case DXGI_FORMAT_BC4_UNORM:
			case DXGI_FORMAT_BC4_SNORM:
			{
				return 8;
			}

			case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
			{
				return 4;
			}

			case DXGI_FORMAT_R8G8_B8G8_UNORM:
			case DXGI_FORMAT_G8R8_G8B8_UNORM:
			{
				return 4;
			}

			case DXGI_FORMAT_UNKNOWN:
			{
				return 0;
			}
		}

		return -1;
	}

	UINT64 RResource::DetermineSize()
	{
		Ensure(Resource);

		D3D12_RESOURCE_DESC ResourceDesc = Resource->GetDesc();

		if (ResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		{
			return ResourceDesc.Width;
		}

		UINT64 Size =
			ResourceDesc.Width				*
			ResourceDesc.Height				*
			ResourceDesc.DepthOrArraySize	*
			ResourceDesc.MipLevels			*
			GetFormatByteSize(ResourceDesc.Format);

		return Size;
	}

	ErrorCode RResource::Map(const UINT Start, const UINT End, const UINT SubResource, void ** ppData)
	{
		ErrorCode Error;

		Ensure
		(
			End < Start
		);

		Ensure
		(
			Resource
		);

		Ensure
		(
			ppData
		);

		CD3DX12_RANGE Range(Start, End);

		if ((Error = Resource->Map(SubResource, &Range, ppData)))
		{
			return Error;
		}

		return S_OK;
	}

	void RResource::Unmap(const UINT SubResource)
	{
		Resource->Unmap(SubResource, NULL);
	}

	void RResource::Unmap(const UINT SubResource, const D3D12_RANGE * pRange)
	{
		Resource->Unmap(SubResource, pRange);
	}

	ErrorCode RResource::MapData(const size_t Start, const size_t End, const void * Source, const UINT SubResource, const bool UnMap)
	{
		ErrorCode Error;

		Ensure
		(
			End < Start
		);

		Ensure
		(
			Resource
		);

		Ensure
		(
			Source
		);

		void * Destination;

		CD3DX12_RANGE Range(Start, End);

		if ((Error = Resource->Map(SubResource, &Range, &Destination)))
		{
			return Error;
		}

		if (End == 0)
		{
			D3D12_RESOURCE_DESC ResourceDesc = Resource->GetDesc();
			{
				UINT Size = ResourceDesc.Width * ResourceDesc.Height * ResourceDesc.DepthOrArraySize;

				CopyMemory
				(
					Destination,
					0,
					Source,
					0,
					Size
				);
			}
		}
		else
		{
			CopyMemory
			(
				Destination,
				0,
				Source,
				0,
				End - Start
			);
		}

		if (UnMap)
		{
			Resource->Unmap(SubResource, &Range);
		}

		return S_OK;
	}

	void RResource::CopyToResolveTarget(const RGrpCommandList & CmdList, RResource * Source, RResource * Target, UINT OffsetSource, UINT OffsetTarget)
	{
		Source->SetResourceState(D3D12_RESOURCE_STATE_COPY_SOURCE, CmdList, OffsetSource);
		Target->SetResourceState(D3D12_RESOURCE_STATE_COPY_DEST, CmdList, OffsetTarget);
		
		CD3DX12_TEXTURE_COPY_LOCATION TargetCopyLocation(*Target, OffsetTarget);
		CD3DX12_TEXTURE_COPY_LOCATION SourceCopyLocation(*Source, OffsetSource);

		CmdList->CopyTextureRegion(
			&TargetCopyLocation,
			0, 0, 0,
			&SourceCopyLocation,
			NULL);
	}
}
