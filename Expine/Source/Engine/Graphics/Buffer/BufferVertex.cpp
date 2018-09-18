#include "Precompiled.h"

#include "Buffer/BufferVertex.h"

namespace D3D
{
	ErrorCode CGrpVertexBufferPair::Create(const D3D12_RESOURCE_STATES InitialState)
	{
		ErrorCode Error;

		if ((Error = CreateBuffer(InitialState)))
		{
			return Error;
		}

		GrpVertexBufferDescriptor * Descriptor = dynamic_cast<GrpVertexBufferDescriptor*>(BufferDescriptor.Get());

		if (!Descriptor)
		{
			return E_FAIL;
		}

		VBV.SizeInBytes		= Descriptor->Size;
		VBV.StrideInBytes	= Descriptor->StrideSize;
		VBV.BufferLocation	= BufferUpload.GetRef()->GetGPUVirtualAddress();

		return S_OK;
	}

	ErrorCode CGrpVertexBufferPair::Create(const VirtualResourceAddress & UploadBufferVA)
	{
		return ErrorCode();
	}

	ErrorCode CVertexBuffer::Create(const D3D12_RESOURCE_STATES InitialState)
	{
		ErrorCode Error = Buffer.Create(InitialState);

		if (Error)
		{
			return Error;
		}

		return S_OK;
	}

	CVertexBuffer::CVertexBuffer(GrpVertexBufferDescriptor * BufferDescriptor) :
		Buffer(BufferDescriptor)
	{}
}
