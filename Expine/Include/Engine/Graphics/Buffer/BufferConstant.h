#pragma once

#include "Buffer/Buffer.h"
#include "Raw/RawDescriptorHeap.h"

namespace D3D
{
	class CGrpConstantBuffer : public CGrpUploadBuffer
	{
	private:

		TVector<ConstantBufferView> CBV;

	private:

		DescriptorHeapRange DescriptorRange;

	public:

		CGrpConstantBuffer
		(
			GrpConstantBufferDescriptor * Descriptor
		) :
			CGrpUploadBuffer(Descriptor)
		{}

		ErrorCode Create
		(
			const DescriptorHeapRange & DescriptorRange
		);

		ErrorCode Create
		(
			const VirtualResourceAddress	& UploadBufferVA,
			const DescriptorHeapRange		& DescriptorRange
		);

	public:

		inline GrpConstantBufferDescriptor * GetBufferDescriptor() const
		{
			return static_cast<GrpConstantBufferDescriptor*>(BufferDescriptor.Get());
		}

		inline const DescriptorHeapRange & GetDescriptorHeapRange() const
		{
			return DescriptorRange;
		}

		inline UINT GetViewCount() const
		{
			return CBV.size();
		}

		inline D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress()	const
		{
			return VirtualAddresses.GPUAddress.Address;
		}

		inline size_t GetCPUVirtualAddress() const
		{
			return VirtualAddresses.CPUAddress.Address;
		}

		inline D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress
		(
			UINT Index
		)	const
		{
			if (Index != 0)
			{
				Index = GetBufferDescriptor()->IndexToOffset(Index);
			}

			return VirtualAddresses.GPUAddress.Address + Index;
		}

		inline size_t GetCPUVirtualAddress
		(
			UINT Index
		)	const
		{
			if (Index != 0)
			{
				Index = GetBufferDescriptor()->IndexToOffset(Index);
			}

			return VirtualAddresses.CPUAddress.Address + Index;
		}

		virtual inline void MapData
		(
					UINT	IndexBegin,
					UINT	Size,
			const	UINT	SubResource,
			const	void *	Data
		)	const	
		override
		{
			if (IndexBegin != 0)
			{
				IndexBegin = GetBufferDescriptor()->IndexToOffset(IndexBegin);
			}

			CGrpUploadBuffer::MapData(IndexBegin, IndexBegin + Size, SubResource, Data);
		}
	};

	class CConstantBuffer
	{
	protected:

		CGrpConstantBuffer Buffer;

	public:

		inline const CGrpConstantBuffer & GetBufferData() const
		{
			return Buffer;
		}

	public:

		ErrorCode Create
		(
			const DescriptorHeapRange & DescriptorRange
		);

		ErrorCode Create
		(
			const VirtualResourceAddress	& UploadBufferVA,
			const DescriptorHeapRange		& DescriptorRange
		);

		CConstantBuffer
		(
			GrpConstantBufferDescriptor * Descriptor
		) :
			Buffer(Descriptor)
		{}

	public:

		inline void Apply
		(
			const UINT				  RootParameterIndex,
			const RGrpCommandList	& CmdList
		)
		{
			CmdList->SetGraphicsRootConstantBufferView(RootParameterIndex, Buffer.GetGPUVirtualAddress(0));
		}

		inline void Apply
		(
			const UINT				ConstantDataIndex,
			const UINT				RootParameterIndex,
			const RGrpCommandList &	CmdList
		)
		{
			CmdList->SetGraphicsRootConstantBufferView(RootParameterIndex, Buffer.GetGPUVirtualAddress(ConstantDataIndex));
		}
	};

	inline void RGrpCommandList::SetConstantBuffer
	(
		const UINT				RootParameterIndex,
		const CConstantBuffer & ConstantBuffer
	)	const
	{
		CommandList->SetGraphicsRootConstantBufferView(RootParameterIndex, ConstantBuffer.GetBufferData().GetGPUVirtualAddress(0));
	}

	inline void RGrpCommandList::SetConstantBuffer
	(
		const UINT						RootParameterIndex,
		const D3D12_GPU_VIRTUAL_ADDRESS GPUAddress
	)	const
	{
		CommandList->SetGraphicsRootConstantBufferView(RootParameterIndex, GPUAddress);
	}

	inline void RGrpCommandList::SetConstantBufferCompute
	(
		const UINT				RootParameterIndex,
		const CConstantBuffer & ConstantBuffer
	)	const
	{
		CommandList->SetComputeRootConstantBufferView(RootParameterIndex, ConstantBuffer.GetBufferData().GetGPUVirtualAddress(0));
	}

	inline void RGrpCommandList::SetConstantBufferCompute
	(
		const UINT						RootParameterIndex,
		const D3D12_GPU_VIRTUAL_ADDRESS GPUAddress
	)	const
	{
		CommandList->SetComputeRootConstantBufferView(RootParameterIndex, GPUAddress);
	}
}