#pragma once

#include "Raw/RawResource.h"

namespace D3D
{
	union VirtualCPUAddress
	{
		size_t	Address;
		void *	Pointer;
	};

	union VirtualGPUAddress
	{
		UINT64 Address;
	};

	struct VirtualResourceAddress
	{
		VirtualGPUAddress GPUAddress;
		VirtualCPUAddress CPUAddress;

		inline VirtualResourceAddress & operator+=
		(
			const UINT64 Offset
		)
		{
			GPUAddress.Address += Offset;
			CPUAddress.Address += Offset;
			return *this;
		}

		inline VirtualResourceAddress operator+
		(
			const UINT64 Offset
		)	const
		{
			VirtualResourceAddress VA(*this);
			VA.GPUAddress.Address += Offset;
			VA.CPUAddress.Address += Offset;
			return VA;
		}

		inline VirtualResourceAddress & operator-=
		(
			const UINT64 Offset
		)
		{
			GPUAddress.Address -= Offset;
			CPUAddress.Address -= Offset;
			return *this;
		}

		inline VirtualResourceAddress operator-
		(
			const UINT64 Offset
		)	const
		{
			VirtualResourceAddress VA(*this);
			VA.GPUAddress.Address -= Offset;
			VA.CPUAddress.Address -= Offset;
			return VA;
		}
	};

	struct ResourceEntry
	{
		inline explicit ResourceEntry
		(
			const SharedPointer<RResource>	&	pResource,
			const UINT64						Offset,
			const VirtualResourceAddress	&	VA
		)
		{
			Resource		= pResource;
			ResourceOffset	= Offset;
			ResourceVA		= VA;
		}

		SharedPointer<RResource>	Resource;
		UINT64						ResourceOffset;
		VirtualResourceAddress		ResourceVA;
	};
}