#pragma once

#include "Raw/RawDevice.h"
#include "Raw/RawCommandList.h"
#include "RawHeap.h"

#include "Utils/DXGIHelper.h"

namespace D3D
{
	static inline constexpr size_t MakeBufferSizeAlign
	(
		const size_t Size,
		const size_t Align
	)
	{
		return (Size + (Align - 1)) & ~(Align - 1);
	}

	template
	<
		UINT NumResources
	>
	class ResourceBarrierBatch
	{
		friend class RResource;

	private:

		ConstPointer<RGrpCommandList>	CommandList;
		ConstPointer<RResource>			Resources[NumResources];

	private:

		UINT Current = 0;

	protected:

		inline ResourceBarrierBatch
		(
			const RGrpCommandList * pCommandList
		)
		{
			CommandList = pCommandList;
		}

		inline void AddBarrier
		(
			const RResource * Resource
		)
		{
			Resources[Current++] = Resource;
		}

		inline void Dispatch()
		{}
	};

	class RResource
	{
		friend class RDepthStencilView;
		friend class RRenderTargetView;

	public:

		static bool ResourceDescCompatible
		(
			const D3D12_RESOURCE_DESC & ResourceDescLhs,
			const D3D12_RESOURCE_DESC & ResourceDescRhs
		);

	public:

		struct InitializeOptions : public D3D12_RESOURCE_DESC
		{
			BOOL Clear = FALSE;

			D3D12_CLEAR_VALUE 		ClearValue;
			D3D12_HEAP_TYPE			HeapType		= D3D12_HEAP_TYPE_DEFAULT;
			D3D12_HEAP_FLAGS		HeapFlags		= D3D12_HEAP_FLAG_NONE;
			D3D12_RESOURCE_STATES	InitialState	= D3D12_RESOURCE_STATE_COMMON;

			InitializeOptions();
			InitializeOptions
			(
				D3D12_RESOURCE_DESC & ResourceDesc
			);
			InitializeOptions
			(
				const UINT						Width,
				const UINT						Height,
				const DXGI_FORMAT				Format,
				const UINT						MipLevels,
				const D3D12_CLEAR_VALUE			ClearValue,
				const D3D12_RESOURCE_DIMENSION	Dimension		= D3D12_RESOURCE_DIMENSION_TEXTURE2D,
				const UINT16					ArraySize		= 1,
				const D3D12_RESOURCE_STATES		InitState		= D3D12_RESOURCE_STATE_COMMON,
				const D3D12_RESOURCE_FLAGS		Flags			= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
				const UINT						SampleCount		= 1
			);
			InitializeOptions
			(
				const UINT						Width,
				const UINT						Height,
				const DXGI_FORMAT				Format,
				const UINT						MipLevels		= 1,
				const Float*					ClearValue		= NULL,
				const D3D12_RESOURCE_DIMENSION	Dimension		= D3D12_RESOURCE_DIMENSION_TEXTURE2D,
				const UINT16					ArraySize		= 1,
				const D3D12_RESOURCE_STATES		InitState		= D3D12_RESOURCE_STATE_COMMON,
				const D3D12_RESOURCE_FLAGS		Flags			= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
				const UINT						SampleCount		= 1
			);
			static InitializeOptions Buffer
			(
				const UINT64					Size,
				const D3D12_HEAP_TYPE			HeapType,
				const D3D12_HEAP_FLAGS			HeapFlags,
				const D3D12_RESOURCE_STATES		ResourceState,
				const D3D12_RESOURCE_FLAGS		ResourceFlags	= D3D12_RESOURCE_FLAG_NONE,
				const DXGI_FORMAT				Format			= DXGI_FORMAT_UNKNOWN
			);
			static InitializeOptions DepthStencil2D
			(
				const UINT						Width,
				const UINT						Height,
				const DXGI_FORMAT				Format,
				const FLOAT						DepthClearValue,
				const UINT						StencilClearValue,
				const UINT						MipLevels	= 1,
				const D3D12_RESOURCE_STATES		InitState	= D3D12_RESOURCE_STATE_DEPTH_WRITE,
				const UINT						SampleCount = 1
			);
			static InitializeOptions DepthStencil2D
			(
				const UINT						Width,
				const UINT						Height,
				const DXGI_FORMAT				Format,
				const UINT						MipLevels	= 1,
				const D3D12_RESOURCE_STATES		InitState	= D3D12_RESOURCE_STATE_DEPTH_WRITE,
				const UINT						SampleCount = 1,
				const FLOAT						ClearColor	= 1.0f
			);

			static constexpr inline bool IsRenderTargetFormat
			(
				const DXGI_FORMAT Format
			)
			{
				return Format != DXGI_FORMAT_BC1_UNORM;
			}

			static InitializeOptions Texture2D
			(
				const UINT						Width,
				const UINT						Height,
				const DXGI_FORMAT				Format,
				const UINT						MipLevels	= 1,
				const Float*					ClearValue	= NULL,
				const D3D12_RESOURCE_STATES		InitState	= D3D12_RESOURCE_STATE_COMMON,
				const UINT						SampleCount	= 1
			);
			static InitializeOptions Texture2DArray
			(
				const UINT						Width,
				const UINT						Height,
				const DXGI_FORMAT				Format,
				const UINT						Depth,
				const UINT						MipLevels	= 1,
				const Float*					ClearValue	= NULL,
				const D3D12_RESOURCE_STATES		InitState	= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				const UINT						SampleCount = 1
			);
			static InitializeOptions Texture3D
			(
				const UINT						Width,
				const UINT						Height,
				const DXGI_FORMAT				Format,
				const UINT						MipLevels	= 1,
				const Float*					ClearValue	= NULL,
				const UINT16					ArraySize	= 1,
				const D3D12_RESOURCE_STATES		InitState	= D3D12_RESOURCE_STATE_COMMON,
				const UINT						SampleCount = 1
			);
		};

	protected:

		UINT64					ResourceId;
		D3D12_RESOURCE_STATES	ResourceState;
		D3D12_RESOURCE_DESC		ResourceDesc;
		D3D12_CLEAR_VALUE		DefaultClearValue;


	protected:

		ComPointer<IResource> Resource;

	public:

		RResource();

		inline IResource * Get() const
		{
			return Resource.Get();
		}

		inline operator IResource *() const
		{
			return Resource.Get();
		}

		inline UINT64 GetId() const
		{
			return ResourceId;
		}

		inline IResource * operator->() const
		{
			return Resource.Get();
		}

		inline D3D12_RESOURCE_STATES GetResourceState() const
		{
			return ResourceState;
		}

		inline const D3D12_CLEAR_VALUE & GetDefaultClearValue() const
		{
			return DefaultClearValue;
		}

		inline const D3D12_RESOURCE_DESC & GetResourceDesc() const
		{
			return ResourceDesc;
		}

		inline void SetResourceState
		(
			const D3D12_RESOURCE_STATES		State,
			const RGrpCommandList		&	GrpCmdList
		);

		inline void SetResourceState
		(
			const D3D12_RESOURCE_STATES		State,
			const RGrpCommandList		&	GrpCmdList,
			const UINT						SubResource
		);

		template
		<
			size_t NumResources
		>
		static inline void SetResourceStates
		(
					RResource			*	ppResources[NumResources],
			const	D3D12_RESOURCE_STATES	State,
			const	RGrpCommandList		&	GrpCmdList,
			const	UINT					SubResource
		);

		template
		<
			size_t NumResources
		>
		static inline void SetResourceStates
		(
					RResource			*	ppResources[NumResources],
			const	D3D12_RESOURCE_STATES	States[NumResources],
			const	RGrpCommandList		&	GrpCmdList,
			const	UINT					SubResource
		);

		template
		<
			size_t NumResources
		>
		static inline void SetResourceStates
		(
					RResource			**	ppResources,
			const	UINT				*	pIndices,
			const	D3D12_RESOURCE_STATES	States[NumResources],
			const	RGrpCommandList		&	GrpCmdList,
			const	UINT					SubResource
		);

		template
		<
			size_t NumResources
		>
		static inline void SetResourceStates
		(
					RResource			**	ppResources,
			const	UINT				*	pIndices,
			const	D3D12_RESOURCE_STATES	State,
			const	RGrpCommandList		&	GrpCmdList,
			const	UINT					SubResource
		);

		template
		<
			size_t NumResources
		>
		static inline void SetResourceStates
		(
					RResource			*	ppResources[NumResources],
			const	D3D12_RESOURCE_STATES	States[NumResources],
			const	RGrpCommandList		&	GrpCmdList,
			const	UINT					SubResources[NumResources]
		);

		inline void AsPresent
		(
			const RGrpCommandList & GrpCmdList
		)
		{
			SetResourceState(D3D12_RESOURCE_STATE_PRESENT, GrpCmdList);
		}

		inline void AsRenderTarget
		(
			const RGrpCommandList & GrpCmdList
		)
		{
			SetResourceState(D3D12_RESOURCE_STATE_RENDER_TARGET, GrpCmdList);
		}

		inline void AsDepthRead
		(
			const RGrpCommandList & GrpCmdList
		)
		{
			SetResourceState(D3D12_RESOURCE_STATE_DEPTH_READ, GrpCmdList);
		}

		inline void AsDepthWrite
		(
			const RGrpCommandList & GrpCmdList
		)
		{
			SetResourceState(D3D12_RESOURCE_STATE_DEPTH_WRITE, GrpCmdList);
		}

		inline void AsPixelShaderResource
		(
			const RGrpCommandList & GrpCmdList
		)
		{
			SetResourceState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, GrpCmdList);
		}

		inline void AsNonPixelShaderResource
		(
			const RGrpCommandList & GrpCmdList
		)
		{
			SetResourceState(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, GrpCmdList);
		}

		inline void AsUnorderedAccessView
		(
			const RGrpCommandList & GrpCmdList
		)
		{
			SetResourceState(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, GrpCmdList);
		}

		inline void AsCopyDest
		(
			const RGrpCommandList & GrpCmdList
		)
		{
			SetResourceState(D3D12_RESOURCE_STATE_COPY_DEST, GrpCmdList);
		}

		inline D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress
		(
			const UINT64 Offset = 0
		)	const
		{
			return Resource->GetGPUVirtualAddress() + Offset;
		}

	public:

		ErrorCode Create
		(
			const InitializeOptions & Options
		);

		ErrorCode CreateReserved
		(
			const InitializeOptions & Options
		);

		ErrorCode CreatePlaced
		(
			const InitializeOptions & Options,
			const HeapEntry			& HeapEntry
		);

		ErrorCode FromSwapChain
		(
			IDXGISwapChain *  const	pSwapChain,
			UINT					Offset
		);

		D3D12_RESOURCE_DIMENSION DetermineDimension();

		static UINT64 GetFormatByteSize
		(
			const DXGI_FORMAT Format
		);

		UINT64 DetermineSize();

		ErrorCode Map
		(
			const	UINT	Start,
			const	UINT	End,
			const	UINT	SubResource,
					void **	ppData
		);

		void Unmap
		(
			const UINT SubResource
		);

		void Unmap
		(
			const UINT			SubResource,
			const D3D12_RANGE *	pRange
		);

		ErrorCode MapData
		(
			const size_t Start,
			const size_t End,
			const void * Source,
			const UINT	 SubResource,
			const bool	 UnMap = true
		);

		static void CopyToResolveTarget
		(
			const	RGrpCommandList	&	CmdList,
					RResource		*	Source,
					RResource		*	Target,
					UINT				OffsetSource,
					UINT				OffsetTarget
		);

		template
		<
			typename PixelType
		>
		ErrorCode CopyToTexture2D
		(
			const	RGrpCommandList &	CmdList,
					RResource		*	ResourceTexture,
			const	PixelType		*	Data,
			const	UINT				SubResource = 0
		)
		{
			ErrorCode Error;

			if (Resource->GetDesc().Dimension != D3D12_RESOURCE_DIMENSION_BUFFER)
			{
				return E_FAIL;
			}

			D3D12_RESOURCE_DESC ResourceDesc = ResourceTexture->Get()->GetDesc();

			D3D12_SUBRESOURCE_FOOTPRINT PitchedDesc = { };
			{
				PitchedDesc.Format	= ResourceDesc.Format;
				PitchedDesc.Width	= ResourceDesc.Width;
				PitchedDesc.Height	= ResourceDesc.Height;
				PitchedDesc.Depth	= ResourceDesc.DepthOrArraySize;

				PitchedDesc.RowPitch = MakeBufferSizeAlign(ResourceDesc.Width * sizeof(PixelType), D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
			}

			void * MappedData;
			{
				CD3DX12_RANGE Range(0, 0);

				if ((Error = Resource->Map(0, &Range, &MappedData)))
				{
					return Error;
				}
			}

			const UINT64 DataOffset = reinterpret_cast<UINT64>(MappedData);

			D3D12_PLACED_SUBRESOURCE_FOOTPRINT PlacedTexture2D = {};
			{
				PlacedTexture2D.Offset = MakeBufferSizeAlign(DataOffset, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) - DataOffset;
				PlacedTexture2D.Footprint = PitchedDesc;
			}

			for (UINT y = 0; y < ResourceDesc.Height; y++)
			{
				PixelType * Scan = reinterpret_cast<PixelType*>(DataOffset + PlacedTexture2D.Offset + y * PitchedDesc.RowPitch);
				{
					memcpy(Scan, &Data[y * ResourceDesc.Width], sizeof(PixelType) * ResourceDesc.Width);
				}
			}

			CmdList->CopyTextureRegion
			(
				&CD3DX12_TEXTURE_COPY_LOCATION(ResourceTexture->Get(), SubResource),
				0, 0, 0,
				&CD3DX12_TEXTURE_COPY_LOCATION(Resource.Get(), PlacedTexture2D),
				NULL
			);

			return S_OK;
		}
	};

	inline void RResource::SetResourceState(const D3D12_RESOURCE_STATES State, const RGrpCommandList & GrpCmdList)
	{
		if (ResourceState != State)
		{
			GrpCmdList->ResourceBarrier(1,
				&CD3DX12_RESOURCE_BARRIER::Transition(
					Resource.Get(),
					ResourceState,
					State));

			ResourceState = State;
		}
	}

	inline void RResource::SetResourceState(const D3D12_RESOURCE_STATES State, const RGrpCommandList & GrpCmdList, const UINT SubResource)
	{
		if (ResourceState != State)
		{
			GrpCmdList->ResourceBarrier(1,
				&CD3DX12_RESOURCE_BARRIER::Transition(
					Resource.Get(),
					ResourceState,
					State,
					SubResource));

			ResourceState = State;
		}
	}

	template
	<
		size_t NumResources
	>
	inline void RResource::SetResourceStates(RResource * ppResources[NumResources], const D3D12_RESOURCE_STATES State, const RGrpCommandList & GrpCmdList, const UINT SubResource)
	{
		CD3DX12_RESOURCE_BARRIER Barriers[NumResources];

#pragma unroll(1)
		for (UINT N = 0; N < NumResources; ++N)
		{
			Barriers[N] = CD3DX12_RESOURCE_BARRIER::Transition(
				ppResources[N]->Get(),
				ppResources[N]->ResourceState,
				State,
				SubResource);

			ppResources[N]->ResourceState = State;
		}

		GrpCmdList->ResourceBarrier(NumResources, Barriers);
	}

	template
	<
		size_t NumResources
	>
	inline void RResource::SetResourceStates(RResource * ppResources[NumResources], const D3D12_RESOURCE_STATES States[NumResources], const RGrpCommandList & GrpCmdList, const UINT SubResource)
	{
		CD3DX12_RESOURCE_BARRIER Barriers[NumResources];

#pragma unroll(1)
		for (UINT N = 0; N < NumResources; ++N)
		{
			Barriers[N] = CD3DX12_RESOURCE_BARRIER::Transition(
				ppResources[N]->Get(),
				ppResources[N]->ResourceState,
				States[N],
				SubResource);

			ppResources[N]->ResourceState = States[N];
		}

		GrpCmdList->ResourceBarrier(NumResources, Barriers);
	}

	template
	<
		size_t NumResources
	>
	inline void RResource::SetResourceStates(RResource * ppResources[NumResources], const D3D12_RESOURCE_STATES States[NumResources], const RGrpCommandList & GrpCmdList, const UINT SubResources[NumResources])
	{
		CD3DX12_RESOURCE_BARRIER Barriers[NumResources];

#pragma unroll(1)
		for (UINT N = 0; N < NumResources; ++N)
		{
			Barriers[N] = CD3DX12_RESOURCE_BARRIER::Transition(
				ppResources[N]->Get(),
				ppResources[N]->ResourceState,
				States[N],
				SubResources[N]);

			ppResources[N]->ResourceState = States[N];
		}

		GrpCmdList->ResourceBarrier(NumResources, Barriers);
	}

	template
	<
		size_t NumResources
	>
	inline void RResource::SetResourceStates
	(
				RResource			**	ppResources,
		const	UINT				*	pIndices,
		const	D3D12_RESOURCE_STATES	States[NumResources],
		const	RGrpCommandList		&	GrpCmdList,
		const	UINT					SubResource
	)
	{
		CD3DX12_RESOURCE_BARRIER Barriers[NumResources];

#pragma unroll(1)
		for (UINT N = 0; N < NumResources; ++N)
		{
			Barriers[N] = CD3DX12_RESOURCE_BARRIER::Transition(
				ppResources[pIndices[N]]->Get(),
				ppResources[pIndices[N]]->ResourceState,
				States[N],
				SubResource);

			ppResources[pIndices[N]]->ResourceState = States[N];
		}

		GrpCmdList->ResourceBarrier(NumResources, Barriers);
	}

	template
	<
		size_t NumResources
	>
	inline void RResource::SetResourceStates
	(
				RResource			**	ppResources,
		const	UINT				*	pIndices,
		const	D3D12_RESOURCE_STATES	State,
		const	RGrpCommandList		&	GrpCmdList,
		const	UINT					SubResource
	)
	{
		CD3DX12_RESOURCE_BARRIER Barriers[NumResources];

#pragma unroll(1)
		for (UINT N = 0; N < NumResources; ++N)
		{
			Barriers[N] = CD3DX12_RESOURCE_BARRIER::Transition(
				ppResources[pIndices[N]]->Get(),
				ppResources[pIndices[N]]->ResourceState,
				State,
				SubResource);

			ppResources[pIndices[N]]->ResourceState = State;
		}

		GrpCmdList->ResourceBarrier(NumResources, Barriers);
	}

	inline void RGrpCommandList::CopyBufferRegion
	(
		const RResource *	pBufferDestination,
		const UINT64		OffsetDestination,
		const RResource *	pBufferSource,
		const UINT64		OffsetSource,
		const UINT64		NumBytes
	)	const
	{
		CommandList->CopyBufferRegion
		(
			pBufferDestination->Get(), 
			OffsetDestination, 
			pBufferSource->Get(), 
			OffsetSource, 
			NumBytes
		);
	}

	inline void RGrpCommandList::CopySubresource
	(
		const RResource *	pDestination,
		const UINT			IndexDestination,
		const RResource *	pSource,
		const UINT			IndexSource
	)	const
	{
		D3D12_TEXTURE_COPY_LOCATION DestLocation =
		{
			pDestination->Get(),
			D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
			IndexDestination
		};

		D3D12_TEXTURE_COPY_LOCATION SrcLocation =
		{
			pSource->Get(),
			D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
			IndexSource
		};

		CommandList->CopyTextureRegion(&DestLocation, 0, 0, 0, &SrcLocation, NULL);
	}
}