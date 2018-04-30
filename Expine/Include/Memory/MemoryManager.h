#pragma once

#include "Memory.h"

#include <Types.h>
#include <Container/HopscotchMap.h>

class AllocationManager
{
private:

	struct Allocation
	{
		void * Data;
		size_t References;
	};

	size_t AllocatedSize;

	tsl::hopscotch_map<size_t, TVector<Allocation> > StaticAllocationMap;
	tsl::hopscotch_map<size_t, TVector<Allocation> > DynamicAllocationMap;

public:

	~AllocationManager()
	{
		for (auto Iter : StaticAllocationMap)
		{
			for (auto ValueIter : Iter.second)
			{
				delete[] ValueIter.Data;
			}
		}

		for (auto Iter : DynamicAllocationMap)
		{
			for (auto ValueIter : Iter.second)
			{
				delete[] ValueIter.Data;
			}
		}
	}

	void * AllocateRoutine(size_t Size)
	{
		auto Iter = StaticAllocationMap.find(Size);

		if(Iter != StaticAllocationMap.end())
		{
			
		}
	}

	void * DeAllocateRoutine(void * Data)
	{

	}
};