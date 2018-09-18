#include "Precompiled.h"

#include "Buffer/BufferConstant.h"

namespace D3D
{
	ErrorCode CGrpConstantBuffer::Create
	(
		const DescriptorHeapRange & DescriptorRange
	)
	{
		if (!DescriptorRange.Valid())
		{
			return E_INVALIDARG;
		}

		GrpConstantBufferDescriptor * CBVDescriptor = dynamic_cast<GrpConstantBufferDescriptor*>(BufferDescriptor.Get());

		if (!CBVDescriptor)
		{
			return E_INVALIDARG;
		}

		ErrorCode Error;

		if ((Error = CreateUploadBuffer()))
		{
			return Error;
		}

		UINT Offset = 0;

		CBV.resize(CBVDescriptor->size());
		
		for (UINT N = 0; N < CBVDescriptor->size(); ++N)
		{
			CBV[N].BufferLocation = VirtualAddresses.GPUAddress.Address + Offset;
			{
				Offset += CBV[N].SizeInBytes = CBVDescriptor->at(N);
			}

			DEVICE->CreateConstantBufferView(&CBV[N], DescriptorRange[N]);
		}

		return S_OK;
	}

	ErrorCode CGrpConstantBuffer::Create(const VirtualResourceAddress & UploadBufferVA, const DescriptorHeapRange & DescriptorRange)
	{
		if (!DescriptorRange.Valid())
		{
			return E_INVALIDARG;
		}

		GrpConstantBufferDescriptor * CBVDescriptor = dynamic_cast<GrpConstantBufferDescriptor*>(BufferDescriptor.Get());

		if (!CBVDescriptor)
		{
			return E_INVALIDARG;
		}
		
		ErrorCode Error;

		if ((Error = CreateUploadBuffer(UploadBufferVA)))
		{
			return Error;
		}

		UINT Offset = 0;

		CBV.resize(CBVDescriptor->size());

		for (UINT N = 0; N < CBVDescriptor->size(); ++N)
		{
			CBV[N].BufferLocation = VirtualAddresses.GPUAddress.Address + Offset;
			{
				Offset += CBV[N].SizeInBytes = CBVDescriptor->at(N);
			}

			DEVICE->CreateConstantBufferView(&CBV[N], DescriptorRange[N]);
		}

		return S_OK;
	}

	ErrorCode CConstantBuffer::Create(const DescriptorHeapRange & DescriptorRange)
	{
		ErrorCode Error;

		if ((Error = Buffer.Create(DescriptorRange)))
		{
			return Error;
		}

		return S_OK;
	}

	ErrorCode CConstantBuffer::Create(const VirtualResourceAddress & UploadBufferVA, const DescriptorHeapRange & DescriptorRange)
	{
		ErrorCode Error;

		if ((Error = Buffer.Create(UploadBufferVA, DescriptorRange)))
		{
			return Error;
		}

		return S_OK;
	}
}
