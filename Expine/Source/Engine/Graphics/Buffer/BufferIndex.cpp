#include "Precompiled.h"

#include "Buffer/BufferIndex.h"

namespace D3D
{
	ErrorCode CGrpIndexBufferPair::Create(const D3D12_RESOURCE_STATES InitialState)
	{
		if (!BufferDescriptor)
		{
			return E_FAIL;
		}

		GrpIndexBufferDescriptor * Descriptor = static_cast<GrpIndexBufferDescriptor*>(BufferDescriptor.Get());
		
		if (Descriptor->Format == DXGI_FORMAT_UNKNOWN)
		{
			return E_FAIL;
		}

		ErrorCode Error = CreateBuffer(InitialState);

		if (Error)
		{
			return Error;
		}

		IBV.Format = Descriptor->Format;
		IBV.SizeInBytes = BufferDescriptor.GetRef();
		IBV.BufferLocation = BufferUpload.GetRef()->GetGPUVirtualAddress();

		return S_OK;
	}

	ErrorCode CIndexBuffer::Create(const D3D12_RESOURCE_STATES InitialState)
	{
		ErrorCode Error = Buffer.Create(InitialState);

		if (Error)
		{
			return Error;
		}

		return S_OK;
	}

	CIndexBuffer::CIndexBuffer(GrpIndexBufferDescriptor * Descriptor) :
		Buffer(Descriptor)
	{}
}
