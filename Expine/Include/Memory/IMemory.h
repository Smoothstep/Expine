#pragma once

#include "Memory.h"

template<class Type> class MemoryObject
{
public:
	static volatile size_t AllocatedMemorySize;

public:
	void * operator new
	(
		size_t Size
	)
	{
		AllocatedMemorySize += Size;

		return ::operator new(Size);
	}

	void operator delete
	(
		void * Memory,
		size_t Size
	)
	{
		if (Memory == NULL)
		{
			return;
		}

		if (Size != sizeof(Type))
		{
			AllocatedMemorySize -= sizeof(IMemoryObject);
			::operator delete(Memory);
			return;
		}

		AllocatedMemorySize -= Size;

		return;
	}

	template<class T>
	FORCEINLINE static Type FromMemory
	(
		const T * Memory
	)
	{
#pragma message("Unsafe Function: FromMemory");
		return *reinterpret_cast<const Type*>(Memory);
	}

	template<class T>
	FORCEINLINE void CopyFromMemory
	(
		const T * Memory
	)
	{
#pragma message("Unsafe Function: CopyFromMemory");
		COPY(
			reinterpret_cast<void*>(this),
			reinterpret_cast<const void*>(Memory),
			sizeof(Type)
		);
	}

	template<class T>
	FORCEINLINE void ToMemory
	(
		T * Memory
	)
	{
#pragma message("Unsafe Function: ToMemory");
		COPY(
			reinterpret_cast<void*>(Memory),
			reinterpret_cast<const void*>(this),
			sizeof(Type)
		);
	}

	template<class T>
	FORCEINLINE void ToSource
	(
		T & Source
	)
	{
		Source = *this;
	}

	template<class T>
	FORCEINLINE static Type & FromSource
	(
		IN const T & Source
	)
	{
		static_assert(sizeof(T) == sizeof(Type), "Incorrect size of source type")
		return *reinterpret_cast<const T*>(&Source);
	}

	template<class T>
	FORCEINLINE void CopyFromSource
	(
		IN const T & Source
	)
	{
		static_assert(sizeof(T) == sizeof(Type), "Incorrect size of source type");

		COPY(
			reinterpret_cast<void*>(this),
			reinterpret_cast<const void*>(&Source),
			sizeof(Type)
		);
	}
};