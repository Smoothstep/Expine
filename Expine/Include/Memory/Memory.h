#pragma once

#include "Defines.h"
#include <immintrin.h>
#include <xmmintrin.h>
#include <memory>


/************************************************************************************/

#undef Allocate

template
<
	class Type
> 
static FORCEINLINE Type * Allocate
(
	size_t Size
)
{
	return reinterpret_cast<Type*>(malloc(Size));
}

/************************************************************************************/

#undef ReAllocate

template
<
	class Type
> 
static FORCEINLINE Type * ReAllocate
(
	Type *& Memory,
	size_t Size
)
{
	return Memory = reinterpret_cast<Type*>(realloc(
		reinterpret_cast<void*>(Memory), Size));
}

/************************************************************************************/

#undef SafeReleaseArray

template
<
	class Type
> 
static FORCEINLINE void SafeReleaseArray
(
	Type *& Memory
)
{
	delete[] Memory;
	Memory = nullptr;
}

/************************************************************************************/

#undef SafeRelease

template
<
	class Type
> 
static FORCEINLINE void SafeRelease
(
	Type *& Memory
)
{
	delete Memory;
	Memory = nullptr;
}

/************************************************************************************/

#undef CopyMemory

template
<
	class Type
> 
static FORCEINLINE Type * CopyMemory
(
			Type		* Destination,
	const	Type		* Source,
	const	size_t		  Size
)
{
	return reinterpret_cast<Type*>(memcpy(
		reinterpret_cast<void*>			(Destination), 
		reinterpret_cast<const void*>	(Source), 
		sizeof(Type) * Size));
}

/************************************************************************************/

template
<
	class Type
>
static FORCEINLINE Type * CopyMemory
(
			Type		*	Destination,
	const	size_t			OffsetDestination,
	const	Type		*	Source,
	const	size_t			OffsetSource,
	const	size_t			Size
)
{
	return reinterpret_cast<Type*>(memcpy(
		reinterpret_cast<void*>			(reinterpret_cast<uintptr_t>(Destination)	+ OffsetDestination),
		reinterpret_cast<const void*>	(reinterpret_cast<uintptr_t>(Source)		+ OffsetSource),
		Size));
}

/************************************************************************************/

static FORCEINLINE void * CopyMemory
(
			void		*	Destination,
	const	size_t			OffsetDestination,
	const	void		*	Source,
	const	size_t			OffsetSource,
	const	size_t			Size
)
{
	return reinterpret_cast<void*>(memcpy(
		reinterpret_cast<void*>			(reinterpret_cast<uintptr_t>(Destination)	+ OffsetDestination),
		reinterpret_cast<const void*>	(reinterpret_cast<uintptr_t>(Source)		+ OffsetSource),
		Size));
}

static FORCEINLINE void Copy256
(
			void * Destination,
	const	void * Source
)
{
	const	__m256 * Src = reinterpret_cast<const	__m256 *>(Source);
			__m256 * Dst = reinterpret_cast<		__m256 *>(Destination);

	Dst[0] = Src[0];
}

static FORCEINLINE void Copy128
(
	void * Destination,
	const	void * Source
)
{
	const	__m128 * Src = reinterpret_cast<const	__m128 *>(Source);
			__m128 * Dst = reinterpret_cast<		__m128 *>(Destination);

	Dst[0] = Src[0];
}