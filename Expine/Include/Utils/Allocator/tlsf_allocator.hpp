#include <exception>
#include <vector>
#include <WindowsH.h>
#include <Defines.h>
#include <algorithm>
#include <stdlib.h>

#undef min
#undef max

extern "C"
{
#include "tlsf.h"
}

template<size_t AllocSize>
class TLSF_AllocationSource
{
private:

	std::vector<std::pair<void *, void*> > MappedPointers;
	static inline const size_t Granularity = GetSysInfo().dwAllocationGranularity;

public:

	TLSF_AllocationSource()
	{
		const size_t RequiredSize = MakeAlign(AllocSize, Granularity);
		const auto LoHi = ExtractLoHi(RequiredSize);
		Mapping = CreateFileMapping(0, 0, PAGE_EXECUTE_READWRITE, LoHi.second, LoHi.first, 0);
		
		if (!Mapping)
		{
			throw std::runtime_error("Unable to create file mapping.");
		}
	}

	~TLSF_AllocationSource()
	{
		for (auto& P : MappedPointers)
		{
			UnmapViewOfFile(P.second);
			CloseHandle(P.first);
		}
	}

	size_t operator()(void *& rpMemory, size_t MinSize)
	{
		const size_t RequiredSize = MakeAlign(std::max(MinSize, AllocSize), Granularity);
		const auto LoHi = ExtractLoHi(RequiredSize);
		std::pair<void*, void*> Pair;
		
		if (!(Pair.second = MapViewOfFile(Pair.first = CreateFileMapping(0, 0, PAGE_EXECUTE_READWRITE, LoHi.second, LoHi.first, 0) RequiredSize, FILE_MAP_ALL_ACCESS, 0, 0)))
		{
			throw std::bad_alloc();
		}

		MappedPointers.push_back(Pair);

		return RequiredSize;
	}
};

template<class Type, class AllocationSource = TLSF_AllocationSource<sizeof(Type)> >
class TLSF_Allocator
{
private:

	void * MemoryPool;
	size_t MemoryPoolSize;

	AllocationSource Alloc;

public:

	explicit TLSF_Allocator(AllocationSource&& Source)
		: Alloc(std::move(Source))
	{
		MemoryPoolSize = Alloc(MemoryPool, sizeof(Type));

		if (MemoryPoolSize == 0)
		{
			throw std::bad_alloc();
		}
	}

	Type * allocate() { return Allocate(); }
	Type * Allocate()
	{
		Type * Result = reinterpret_cast<Type*>(malloc_ex(sizeof(Type), MemoryPool));

		if (!Result)
		{
			void * Area;
			size_t AreaSize = Alloc(Area, sizeof(Type));

			if (MemoryPoolSize == 0)
			{
				throw std::bad_alloc();
			}

			add_new_area(Area, AreaSize, MemoryPool);

			return reinterpret_cast<Type*>(malloc_ex(sizeof(Type), MemoryPool));
		}
	}

	Type * Allocate(size_t Count)
	{
		Type * Result = reinterpret_cast<Type*>(malloc_ex(sizeof(Type) * Count, MemoryPool));

		if (!Result)
		{
			void * Area;
			size_t AreaSize = Alloc(Area, sizeof(Type) * Count);

			if (MemoryPoolSize == 0)
			{
				throw std::bad_alloc();
			}

			add_new_area(Area, AreaSize, MemoryPool);

			return reinterpret_cast<Type*>(malloc_ex(sizeof(Type) * Count, MemoryPool));
		}
	}

	void deallocate(Type * Memory) { return Deallocate(); }
	void Deallocate(Type * Memory)
	{
		free_ex(Memory, MemoryPool);
	}
};