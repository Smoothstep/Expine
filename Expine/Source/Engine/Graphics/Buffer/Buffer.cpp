#include "Precompiled.h"
#include "Buffer/Buffer.h"

namespace D3D
{
	ErrorCode CGrpBufferPair::CreateBuffer(const D3D12_RESOURCE_STATES InitialState)
	{
		Ensure(BufferDescriptor);

		ErrorCode Error;

		if ((Error = CreateUploadBuffer()))
		{
			return Error;
		}

		Buffer = new RResource();
		{
			if ((Error = Buffer->Create(RResource::InitializeOptions::Buffer(
				BufferDescriptor.GetRef(),
				D3D12_HEAP_TYPE_DEFAULT,
				D3D12_HEAP_FLAG_NONE,
				InitialState))))
			{
				return Error;
			}

			Buffer.GetRef()->SetName(L"Buffer");
		}

		return S_OK;
	}

	ErrorCode CGrpUploadBuffer::CreateUploadBuffer()
	{
		Ensure(BufferDescriptor);

		ErrorCode Error;

		BufferUpload = new RResource();
		{
			if ((Error = BufferUpload->Create(RResource::InitializeOptions::Buffer(
				BufferDescriptor.GetRef(),
				D3D12_HEAP_TYPE_UPLOAD,
				D3D12_HEAP_FLAG_NONE,
				D3D12_RESOURCE_STATE_GENERIC_READ))))
			{
				return Error;
			}

			BufferUpload.GetRef()->SetName(L"Upload Buffer");
		}

		if ((Error = BufferUpload->Map(0, 0, 0, &VirtualAddresses.CPUAddress.Pointer)))
		{
			return Error;
		}

		VirtualAddresses.GPUAddress.Address = BufferUpload->GetGPUAddress();

		return S_OK;
	}

	ErrorCode CGrpUploadBuffer::CreateUploadBuffer(const VirtualResourceAddress & VA)
	{
		VirtualAddresses = VA;
		return S_OK;
	}

	ErrorCode CGrpBuffer::CreateBuffer(const D3D12_RESOURCE_STATES InitialState)
	{
		Ensure(BufferDescriptor);

		ErrorCode Error;

		Buffer = new RResource();
		{
			if ((Error = Buffer->Create(RResource::InitializeOptions::Buffer(
				BufferDescriptor.GetRef(),
				D3D12_HEAP_TYPE_DEFAULT,
				D3D12_HEAP_FLAG_NONE,
				InitialState))))
			{
				return Error;
			}

			Buffer.GetRef()->SetName(L"Buffer");
		}

		return S_OK;
	}
}