#include "Precompiled.h"
#include "Buffer/BufferIndex.h"
#include "Buffer/BufferVertex.h"
#include "Buffer/BufferCommand.h"
#include "Buffer/BufferCache.h"

namespace D3D
{
	struct VBKey
	{
		const size_t Size;
		const size_t Stride;

		inline constexpr VBKey
		(
			const size_t Size,
			const size_t Stride
		) :
			Size(Size), Stride(Stride)
		{}

		inline constexpr bool operator==
		(
			const VBKey & Other
		)	const
		{
			return Size == Other.Size && Stride == Other.Stride;
		}
	};

	using VBuffers = TVector<SharedPointer<CVertexBuffer> >;

	struct IBKey
	{
		const size_t		Size;
		const DXGI_FORMAT	Format;

		inline constexpr IBKey
		(
			const size_t		Size,
			const DXGI_FORMAT	Format
		) :
			Size(Size), Format(Format)
		{}

		inline constexpr bool operator==
		(
			const IBKey & Other
		)	const
		{
			return Size == Other.Size && Format == Other.Format;
		}
	};

	using IBuffers = TVector<SharedPointer<CIndexBuffer> >;

	struct CBKey
	{
		const size_t Size;

		inline constexpr CBKey
		(
			const size_t Size
		) :
			Size(Size)
		{}

		inline constexpr bool operator==
		(
			const CBKey & Other
		)	const
		{
			return Size == Other.Size;
		}
	};

	using CBuffers = TVector<SharedPointer<CCommandBuffer> >;

	struct Hash
	{
		inline constexpr size_t operator()
		(
			const IBKey & Key
		)	const
		{
			return 
				(std::hash<size_t>()(Key.Size)) ^ 
				(std::hash<size_t>()(Key.Format) << 1);
		}
		inline constexpr size_t operator()
		(
			const VBKey & Key
		)	const
		{
			return 
				(std::hash<size_t>()(Key.Size)) ^ 
				(std::hash<size_t>()(Key.Stride) << 1);
		}
		inline constexpr size_t operator()
		(
			const CBKey & Key
		)	const
		{
			return
				std::hash<size_t>()(Key.Size);
		}
	};

	class Cache
	{
	private:

		typedef THashMap<IBKey, IBuffers, Hash> TIB;
		typedef THashMap<VBKey, VBuffers, Hash> TVB;
		typedef THashMap<CBKey, CBuffers, Hash> TCB;

		TIB IBufferMap;
		TVB VBufferMap;
		TCB CBufferMap;

	public:

		auto GetVertexBuffer(const VBKey & K)
		{
			auto Iter = VBufferMap.find(K);

			if (Iter == VBufferMap.end())
			{
				return (VBuffers*)(nullptr);
			}

			return std::addressof(Iter.value());
		}

		auto GetIndexBuffer(const IBKey & K)
		{
			auto Iter = IBufferMap.find(K);

			if (Iter == IBufferMap.end())
			{
				return (IBuffers*)(nullptr);
			}

			return std::addressof(Iter.value());
		}

		auto GetCommandBuffer(const CBKey & K)
		{
			auto Iter = CBufferMap.find(K);

			if (Iter == CBufferMap.end())
			{
				return (CBuffers*)(nullptr);
			}

			return std::addressof(Iter.value());
		}

		template<class Key>
		auto GetBuffer(const Key & K)
		{
			if constexpr (std::is_same<Key, VBKey>::value)
			{
				return GetVertexBuffer(K);
			}
			else if constexpr (std::is_same<Key, IBKey>::value)
			{
				return GetIndexBuffer(K);
			}
			else
			{
				return GetCommandBuffer(K);
			}
		}

		template<EBufferType Type, class... Args>
		void AddBuffer(Args&&... Arguments)
		{
			if constexpr (Type == BufferTypeVertex)
			{
				VBufferMap.emplace(Arguments...);
			}
			else if constexpr (Type == BufferTypeIndex)
			{
				IBufferMap.emplace(Arguments...);
			}
			else
			{
				CBufferMap.emplace(Arguments...);
			}
		}
	};

	static Cache GCache;

	bool CBufferCache::GetBuffer(size_t BufferSize, size_t StrideSize, SharedPointer<CVertexBuffer>& Buffer)
	{
		auto Buffers = GCache.GetBuffer(VBKey(BufferSize, StrideSize));

		if (Buffers)
		{
			Buffers->FindVoid([&](const SharedPointer<CVertexBuffer> & B)
			{
				if (B.GetReferenceCount() == 1)
				{
					Buffer = B;
					return true;
				}

				return false;
			});
		}

		return !Buffer.Nil();
	}
	
	bool CBufferCache::GetBuffer(size_t BufferSize, DXGI_FORMAT Format, SharedPointer<CIndexBuffer>& Buffer)
	{
		auto Buffers = GCache.GetBuffer(IBKey(BufferSize, Format));

		if (Buffers)
		{
			Buffers->FindVoid([&](const SharedPointer<CIndexBuffer> & B)
			{
				if (B.GetReferenceCount() == 1)
				{
					Buffer = B;
					return true;
				}

				return false;
			});
		}

		return !Buffer.Nil();
	}

	bool CBufferCache::GetBuffer(size_t BufferSize, SharedPointer<CCommandBuffer>& Buffer)
	{
		auto Buffers = GCache.GetBuffer(CBKey(BufferSize));

		if (Buffers)
		{
			Buffers->FindVoid([&](const SharedPointer<CIndexBuffer> & B)
			{
				if (B.GetReferenceCount() == 1)
				{
					Buffer = B;
					return true;
				}

				return false;
			});
		}

		return !Buffer.Nil();
	}

	ErrorCode CBufferCache::CreateBuffer(size_t StrideCount, size_t StrideSize, SharedPointer<CVertexBuffer>& Buffer)
	{
		ErrorCode Error;

		auto BufferKey = VBKey(StrideCount, StrideSize);
		auto Buffers = GCache.GetBuffer(BufferKey);

		if (Buffers)
		{
			for (const auto & ValueIter : *Buffers)
			{
				if (ValueIter.GetReferenceCount() == 1)
				{
					Buffer = ValueIter;
					return S_OK;
				}
			}

			Buffer = new CVertexBuffer(new GrpVertexBufferDescriptor(StrideSize, StrideCount));
			{
				if ((Error = Buffer->Create(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)))
				{
					return Error;
				}
			}

			Buffers->push_back(Buffer);
		}
		else
		{
			Buffer = new CVertexBuffer(new GrpVertexBufferDescriptor(StrideSize, StrideCount));
			{
				if ((Error = Buffer->Create(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)))
				{
					return Error;
				}
			}

			VBuffers BufferVector =
			{
				Buffer
			};

			GCache.AddBuffer<BufferTypeVertex>(BufferKey, BufferVector);
		}

		return S_OK;
	}
	
	ErrorCode CBufferCache::CreateBuffer(size_t NumIndices, DXGI_FORMAT Format, SharedPointer<CIndexBuffer>& Buffer)
	{
		ErrorCode Error;

		auto BufferKey = IBKey(NumIndices, Format);
		auto Buffers = GCache.GetBuffer(BufferKey);

		if (Buffers)
		{
			for (const auto & ValueIter : *Buffers)
			{
				if (ValueIter.GetReferenceCount() == 1)
				{
					Buffer = ValueIter;
					return S_OK;
				}
			}

			Buffer = new CIndexBuffer(new GrpIndexBufferDescriptor(NumIndices, Format));
			{
				if ((Error = Buffer->Create(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)))
				{
					return Error;
				}
			}

			Buffers->push_back(Buffer);
		}
		else
		{
			Buffer = new CIndexBuffer(new GrpIndexBufferDescriptor(Format, NumIndices));
			{
				if ((Error = Buffer->Create(D3D12_RESOURCE_STATE_INDEX_BUFFER)))
				{
					return Error;
				}
			}

			IBuffers BufferVector =
			{
				Buffer
			};

			GCache.AddBuffer<BufferTypeIndex>(BufferKey, BufferVector);
		}

		return S_OK;
	}
	
	ErrorCode CBufferCache::CreateBuffer(size_t CommandSizePerFrame, SharedPointer<CCommandBuffer>& Buffer)
	{
		ErrorCode Error;

		auto BufferKey = CBKey(CommandSizePerFrame);
		auto Buffers = GCache.GetBuffer(BufferKey);

		if (Buffers)
		{
			for (const auto & ValueIter : *Buffers)
			{
				if (ValueIter.GetReferenceCount() == 1)
				{
					Buffer = ValueIter;
					return S_OK;
				}
			}

			Buffer = new CCommandBuffer(new GrpCommandBufferDescriptor(CommandSizePerFrame));
			{
				if ((Error = Buffer->Create(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)))
				{
					return Error;
				}
			}

			Buffers->push_back(Buffer);
		}
		else
		{
			Buffer = new CCommandBuffer(new GrpCommandBufferDescriptor(CommandSizePerFrame));
			{
				if ((Error = Buffer->Create(D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT)))
				{
					return Error;
				}
			}

			CBuffers BufferVector =
			{
				Buffer
			};

			GCache.AddBuffer<BufferTypeCommand>(BufferKey, BufferVector);
		}

		return S_OK;
	}
}