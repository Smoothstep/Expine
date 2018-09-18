#pragma once

#include "Raw/RawDescriptorHeap.h"
#include "Resource/Resource.h"

#include <algorithm>
#include <numeric>

namespace D3D
{
	enum EBufferType
	{
		BufferTypeInvalid = -1,
		BufferTypeIndex,
		BufferTypeVertex,
		BufferTypeConstant,
		BufferTypeCommand
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	static constexpr unsigned GConstantBufferAlignSize = 256;

	static inline constexpr size_t MakeConstantBufferSizeAlign
	(
		const size_t Size
	)
	{
		return MakeBufferSizeAlign(Size, GConstantBufferAlignSize);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	struct GrpBufferDescriptor
	{
		size_t Size;

		inline operator size_t() const
		{
			return Size;
		}

		virtual ~GrpBufferDescriptor() PURE;
	};

	inline GrpBufferDescriptor::~GrpBufferDescriptor()
	{}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	struct GrpIndexBufferDescriptor : public GrpBufferDescriptor
	{
		size_t StrideSize;
		size_t StrideCount;

		DXGI_FORMAT Format;

		explicit inline GrpIndexBufferDescriptor()
		{
			this->Size = this->StrideSize = this->StrideCount = 0;
		}

		explicit inline GrpIndexBufferDescriptor
		(
			const size_t StrideSize,
			const size_t StrideCount
		)
		{
			this->StrideSize	= StrideSize;
			this->StrideCount	= StrideCount;
			this->Size			= StrideSize * StrideCount;
			this->Format		= StrideSize ==
				sizeof(Uint8)  ? DXGI_FORMAT_R8_UINT  :
				sizeof(Uint16) ? DXGI_FORMAT_R16_UINT :
				sizeof(Uint32) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_UNKNOWN;
		}

		explicit inline GrpIndexBufferDescriptor
		(
			const DXGI_FORMAT	Format,
			const size_t		IndexCount
		)
		{
			this->StrideSize = 
				Format == DXGI_FORMAT_R8_UINT  ? sizeof(Uint8)  : 
				Format == DXGI_FORMAT_R16_UINT ? sizeof(Uint16) :
				Format == DXGI_FORMAT_R32_UINT ? sizeof(Uint32) : DXGI_FORMAT_UNKNOWN;

			this->StrideCount	= IndexCount;
			this->Format		= Format;
			this->Size			= StrideSize * IndexCount;
		}
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	struct GrpCommandBufferDescriptor : public GrpBufferDescriptor
	{
		UINT CommandSizePerFrame;

		explicit inline GrpCommandBufferDescriptor
		(
			UINT CommandSizePerFrame
		)
		{
			this->CommandSizePerFrame	= CommandSizePerFrame;
			this->Size					= CommandSizePerFrame;
		}

		explicit inline GrpCommandBufferDescriptor
		(
			UINT CommandStrideSize,
			UINT CommandSize
		)
		{
			this->CommandSizePerFrame	= CommandStrideSize * CommandSize;
			this->Size					= CommandSizePerFrame;
		}
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	struct GrpVertexBufferDescriptor : public GrpBufferDescriptor
	{
		size_t StrideSize;
		size_t StrideCount;

		explicit inline GrpVertexBufferDescriptor()
		{
			Size = StrideSize = StrideCount = 0;
		}

		explicit inline GrpVertexBufferDescriptor
		(
			const size_t StrideSize,
			const size_t StrideCount
		)
		{
			this->StrideSize		= StrideSize;
			this->StrideCount	= StrideCount;
			this->Size			= StrideSize * StrideCount;
		}
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	struct GrpConstantBufferDescriptor : public GrpBufferDescriptor, public TVector<size_t>
	{
	private:
		template<class Class, class... Classes>
		static void CreateDescriptor(GrpConstantBufferDescriptor * Descriptor)
		{
			Descriptor->push_back(sizeof(Class));

			if constexpr (sizeof...(Classes) > 1)
			{
				CreateDescriptor<Classes...>(Descriptor);
			}
		}
		
	public:

		explicit inline GrpConstantBufferDescriptor()
		{
			Size = 0;
		}

		explicit inline GrpConstantBufferDescriptor
		(
			const size_t Size
		)
		{
			push_back(this->Size = MakeConstantBufferSizeAlign(Size));
		}

		explicit inline GrpConstantBufferDescriptor
		(
			const TVector<size_t> & Size
		)
		{
			static_cast<TVector<size_t> &>(*this) = Size;
			{
				this->Size = IndexToOffset(size() - 1);
			}
		}

		explicit inline GrpConstantBufferDescriptor
		(
			const size_t			Alignment,
			const TVector<size_t> & Size
		)
		{
			static_cast<TVector<size_t> &>(*this) = Size;
			{
				TransformAligned(Alignment);
			}
		}

		template<const size_t Alignment, class... Classes>
		static GrpConstantBufferDescriptor * Create()
		{
			GrpConstantBufferDescriptor * Descriptor = new GrpConstantBufferDescriptor();
			{
				CreateDescriptor<Classes...>(Descriptor);
			}

			Descriptor->TransformAligned(Alignment);

			return Descriptor;
		}

		inline void TransformAligned
		(
			const size_t Alignment
		)
		{
			std::transform(begin(), end(), begin(), [Alignment](size_t Size)
			{
				return MakeBufferSizeAlign(Size, Alignment);
			});

			Size = IndexToOffset(size());
		}

		inline size_t IndexToOffset
		(
			const UINT Index
		)
		{
			return std::accumulate(begin(), begin() + Index, 0, std::plus<size_t>());
		}
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	class IBuffer
	{
	public:

		virtual inline const RResource & GetBuffer() const PURE;

	public:

		virtual inline bool UploadData
		(
			const void					*	Data,
			const UINT						Size,
			const CCommandListContext	&	CmdListCtx,
			const UINT						Offset = 0
		)	const PURE;
	};

	class CGrpUploadBuffer
	{
	protected:

		SharedPointer<RResource>			BufferUpload;
		SharedPointer<GrpBufferDescriptor>	BufferDescriptor;

		VirtualResourceAddress VirtualAddresses;

	public:

		explicit inline CGrpUploadBuffer() {}
		explicit inline CGrpUploadBuffer
		(
			GrpBufferDescriptor * Descriptor
		)
		{
			BufferDescriptor = Descriptor;
		}

		ErrorCode CreateUploadBuffer();
		ErrorCode CreateUploadBuffer
		(
			const VirtualResourceAddress & VA
		);

	public:

		inline const VirtualResourceAddress & GetResourceVA() const
		{
			return VirtualAddresses;
		}

		virtual inline void MapData
		(
			const	UINT	Begin,
					UINT	End,
			const	UINT	SubResource,
			const	void *	Data
		)	const
		{
			Ensure
			(
				BufferDescriptor
			);

			Ensure
			(
				Begin < BufferDescriptor.GetRef()
			);

			Ensure
			(
				VirtualAddresses.CPUAddress.Pointer
			);

			if (End == 0)
			{
				End = BufferDescriptor.GetRef();
			}

			CopyMemory
			(
				VirtualAddresses.CPUAddress.Pointer,
				Begin,
				Data,
				0,
				End - Begin
			);
		}
	};

	class CGrpBuffer
	{
		friend class CCommandListBase;

	protected:

		SharedPointer<RResource>			Buffer;
		SharedPointer<GrpBufferDescriptor>	BufferDescriptor;

	protected:

		ErrorCode CreateBuffer
		(
			const D3D12_RESOURCE_STATES InitialState = D3D12_RESOURCE_STATE_COPY_DEST
		);

	public:

		explicit inline CGrpBuffer() {}
		explicit inline CGrpBuffer
		(
			GrpBufferDescriptor * Descriptor
		)
		{
			
			BufferDescriptor = Descriptor;
		}
	};

	class CGrpBufferPair : public CGrpUploadBuffer
	{
	protected:

		SharedPointer<RResource> Buffer;

	protected:

		ErrorCode CreateBuffer
		(
			const D3D12_RESOURCE_STATES InitialState = D3D12_RESOURCE_STATE_COPY_DEST
		);

	public:

		using CGrpUploadBuffer::CGrpUploadBuffer;

	public:

		inline RResource * GetBufferResource() const
		{
			return Buffer.Get();
		}
	};
}