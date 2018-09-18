#include "Precompiled.h"
#include "Buffer/BufferCommand.h"

namespace D3D
{
	CCommandBuffer::CCommandBuffer(GrpCommandBufferDescriptor * Descriptor) :
		Buffer(Descriptor)
	{}

	ErrorCode CCommandBuffer::Create(const D3D12_RESOURCE_STATES InitialState)
	{
		ErrorCode Error;

		if ((Error = Buffer.Create(InitialState)))
		{
			return Error;
		}

		return S_OK;
	}

	CGrpCommandBufferPair::CGrpCommandBufferPair(GrpCommandBufferDescriptor * pDescriptor) :
		CGrpBufferPair(pDescriptor)
	{}

	ErrorCode CGrpCommandBufferPair::Create()
	{
		if (!BufferDescriptor)
		{
			return E_FAIL;
		}

		ErrorCode Error = CreateBuffer(D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

		if (Error)
		{
			return Error;
		}

		return S_OK;
	}

	CGrpCommandBuffer::CGrpCommandBuffer(GrpCommandBufferDescriptor * pDescriptor) :
		CGrpBuffer(pDescriptor)
	{}

	ErrorCode CGrpCommandBuffer::Create(const D3D12_RESOURCE_STATES InitialState)
	{
		if (!BufferDescriptor)
		{
			return E_FAIL;
		}

		ErrorCode Error = CreateBuffer(InitialState);

		if (Error)
		{
			return Error;
		}

		return S_OK;
	}
}
