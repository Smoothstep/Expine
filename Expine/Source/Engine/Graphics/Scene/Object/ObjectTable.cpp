#include "Precompiled.h"

#include "ObjectTable.h"
#include "SpeedTree.h"

static constexpr size_t GObjectTableCacheCapacity = 256U;

namespace D3D
{
	class ObjectTypeInfo
	{
	public:

		static constexpr EObjectType Type = ObjectTypeNone;

	private:

		Uint32 Id;

	public:

		inline ObjectTypeInfo(const Uint32 Id) :
			Id(Id)
		{}

	public:

		virtual inline EObjectType GetSubType() const = 0;
	};

	class SpeedTreeTypeInfo : public ObjectTypeInfo
	{
	public:

		static constexpr EObjectType Type = ObjectTypeSpeedTree;

	private:

		CSpeedTree * Spt;

		inline void Initialize(const String& FilePath)
		{

		}

	public:

		inline SpeedTreeTypeInfo(const Uint32 Id) :
			ObjectTypeInfo(Id)
		{}

		template<class FileString>
		inline SpeedTreeTypeInfo(const Uint32 Id, const FileString & FilePath) :
			ObjectTypeInfo(Id)
		{
			Initialize(FilePath);
		}

		virtual inline EObjectType GetSubType() const
		{
			return Type;
		}

		template<class Allocator = std::allocator<CSpeedTreeObject> >
		CSpeedTreeObject * Create(const ObjectTableEntry * Entry, Allocator & Allocator = std::allocator<CSpeedTreeObject>())
		{

		}
	};

	class ExObjectTypeCreate : public Exception
	{

	};

	class ExObjectTypeAdd : public Exception
	{
	public:
		
		enum EReason
		{
			AlreadyExists,
			Invalid
		};

	private:

		EReason Reason;

		static inline String GetReasonMessage(const EReason Reason, const ObjectTypeInfo * Info)
		{
			switch (Reason)
			{
				case AlreadyExists:
				{
					return String::MakeString<String::Whitespace>("Object already exists:", Info->GetSubType());
				}

				case Invalid:
				{
					return String::MakeString<String::Whitespace>("Object type invalid:", Info->GetSubType());
				}
			}

			return String();
		}

		ObjectTypeInfo * Info;

	public:

		inline ExObjectTypeAdd(EReason Reason, ObjectTypeInfo * Info) :
			Reason(Reason), Info(Info),
			Exception(GetReasonMessage(Reason, Info))
		{}

		inline EReason GetReason() const
		{
			return Reason;
		}

		inline ObjectTypeInfo * GetInfo() const
		{
			return Info;
		}
	};

	class ObjectTypeStorage
	{
	private:

		TArray<THashMap<Uint32, ObjectTypeInfo*>, ObjectTypeCount> ObjectTypeMap;

	public:

		template <class Class, typename... Args>
		inline Class * AddObjectType(const Uint32 ObjectTypeId, Args&&... Arguments)
		{
			auto Object = ObjectTypeMap[Class::Type].Find(ObjectTypeId);

			if (Object)
			{
				return static_cast<Class*>(*Object);
			}

			return static_cast<Class*>(ObjectTypeMap[Class::Type].emplace(
				ObjectTypeId, new Class(ObjectTypeId, std::forward<Args>(Arguments)...)).second);
		}

		template <class Class>
		inline void RemoveObjectType(const Uint32 ObjectTypeId)
		{
			auto Iter = ObjectTypeMap[Class::Type].find(ObjectTypeId);

			if (Iter != ObjectTypeMap[Class::Type].end())
			{
				ObjectTypeMap[Class::Type].erase(Iter);
			}
		}

		template <class Class>
		inline const ObjectTypeInfo * GetObjectTypeInfo(const Uint32 ObjectTypeId) const
		{
			auto Iter = ObjectTypeMap[Class::Type].find(ObjectTypeId);

			if (Iter != ObjectTypeMap[Class::Type].end())
			{
				return Iter.value();
			}

			return NULL;
		}
	};

	static ObjectTypeStorage GStorage;

	class ObjectFactory
	{
	private:

	public:

		template <class Object, class ObjectInfo>
		static Object * Create(ObjectInfo * Info, ObjectTableEntry * Entry)
		{
			return Info->Create(Entry);
		}

		template <class ObjectInfo, class Iterator, class Result>
		static void CreateMultiple(ObjectInfo * Info, Iterator & Iter, const Iterator & End, Result & Container)
		{
			const Uint32 Type = Iter->Type;

			for (Iter; Iter != End && Iter->Type == Type; ++Iter)
			{
				Container.Add(static_cast<ObjectInfo*>(Info)->Create(&*Iter));
			}
		}
	};

	ObjectTableCache::ObjectTableCache() :
		Cache(GObjectTableCacheCapacity)
	{
	}

	class ObjectContainer
	{
		template<class C, size_t ElementCount> class SimpleAllocator
		{
		public:

			constexpr static size_t NodeHeapSize = ElementCount * sizeof(C);

		private:

			HANDLE Heap;

		public:

			inline SimpleAllocator() 
			{
				if (!(Heap = HeapCreate(HEAP_CREATE_ALIGN_16, 0, 0))) 
				{
					throw ERROR_INVALID_HANDLE;
				}
			}

			inline ~SimpleAllocator() noexcept 
			{
				if (Heap) 
				{
					HeapDestroy(Heap);
				}
			}

			inline C * Allocate() const noexcept 
			{
				return reinterpret_cast<C*>(HeapAlloc(Heap, 0, NodeHeapSize));
			}

			inline C * Allocate(size_t NumElements) const noexcept 
			{
				return reinterpret_cast<C*>(HeapAlloc(Heap, 0, NumElements * sizeof(C)));
			}

			inline C * ReAllocate(C * Memory, size_t Size) const noexcept 
			{
				return reinterpret_cast<C*>(HeapReAlloc(Heap, HEAP_REALLOC_IN_PLACE_ONLY, Memory, Size));
			}

			inline void Deallocate(C * N) const noexcept 
			{
				HeapFree(Heap, 0, N);
			}
		};

		class Allocator : public SimpleAllocator<CSceneObject, GObjectTableCacheCapacity>
		{
		private:

			size_t Space = 0;
			size_t SpaceAvailable = 0;

			CSceneObject * Memory;

			TVector<TPair<CSceneObject*, size_t> > Allocations;

		public:

			Allocator() = default;

			inline void * allocate(const size_t Size) { return Allocate(Size); };
			inline void * Allocate(const size_t Size)
			{
				if (SpaceAvailable < Size)
				{
					if (Memory)
					{
						CSceneObject * M = Memory;
						
						if (!(Memory = ReAllocate(Memory, Space + Size * GObjectTableCacheCapacity)))
						{
							Allocations.push_back(std::make_pair(Memory, Space));

							if (!(Memory = SimpleAllocator::Allocate()))
							{
								throw Exception("Out of Memory.");
							}

							Space = SpaceAvailable = SimpleAllocator::NodeHeapSize - Size;
						}
						else
						{
							Space += 
								Size * GObjectTableCacheCapacity;
							SpaceAvailable += 
								Size * GObjectTableCacheCapacity - Size;
						}
					}
					else
					{
						if (!(Memory = SimpleAllocator::Allocate()))
						{
							throw Exception("Out of Memory.");
						}

						Space = SimpleAllocator::NodeHeapSize - Size;
					}
				}

				SpaceAvailable -= Size;
			}
		};

	private:

		TVector<CSceneObject*> ObjectPointers;
		
	protected:

		Allocator ObjectHeap;

	public:

		void Add(CSceneObject * Object)
		{

		}
	};

	void ObjectTableLoader::LoadTable(ObjectTable & Table)
	{
		ObjectContainer Container;
		ObjectTypeStorage Storage;

		const auto & Entries = Table.GetEntries();

		try
		{
			ObjectTypeInfo * Info;

			for (auto Iter = Entries.begin(); Iter != Entries.end(); ++Iter)
			{
				switch (Iter->Type)
				{
					case ObjectTypeSpeedTree:
					{
						ObjectFactory::CreateMultiple(Storage.AddObjectType<SpeedTreeTypeInfo>(Iter->Id), Iter, Entries.end(), Container);
					}

					break;
				}
			}
		}
		catch (const Exception & Exception)
		{
			CErrorLog::Log<LogException>(Exception.what(), CErrorLog::EndLine);
		}
	}

	inline void ObjectTable::Sort()
	{
		std::sort(
			std::begin(Entries),
			std::end(Entries),
			[this](
				const ObjectTableEntry & Lhs,
				const ObjectTableEntry & Rhs)
			{
				return Lhs.Type < Rhs.Type;
			}
		);
	}
}
