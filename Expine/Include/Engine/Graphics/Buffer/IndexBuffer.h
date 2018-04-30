#pragma once

#include "Buffer.h"
#include "RawCommandList.h"

namespace D3D
{
	class CGrpIndexBufferPair : public CGrpBufferPair
	{
	private:

		IndexBufferView IBV;

	public:

		inline const IndexBufferView & GetBufferView() const
		{
			return IBV;
		}

		inline D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress()	const
		{
			return Buffer.GetRef()->GetGPUVirtualAddress();
		}

	public:

		template
		<
			typename Index
		>
		inline bool UploadData
		(
			const Index				* Indices,
			const UINT				  NumIndices,
			const RGrpCommandList	& CmdList,
			const UINT				  Offset = 0,
			const bool				  Transition = true
		)	const
		{
			size_t Size = sizeof(Index) * NumIndices;

			// A few checks. Not all cases are handled.

			Ensure
			(
				Buffer != NULL
			);

			Ensure
			(
				BufferUpload != NULL
			);

			Ensure // Size can't be bigger than the IBV size
			(
				Size < IBV.SizeInBytes
			);

			Ensure // Offset may not go out of bounds
			(
				Offset < IBV.SizeInBytes
			);

			if (Transition)
			{
				Buffer->SetResourceState(D3D12_RESOURCE_STATE_COPY_DEST, CmdList);
			}

			D3D12_SUBRESOURCE_DATA SubResourceData;
			{
				SubResourceData.pData = Indices;
				SubResourceData.RowPitch = Size;
				SubResourceData.SlicePitch = Size;
			}

			UINT64 iRequiredSize;

			if (iRequiredSize = UpdateSubresources(
				CmdList,
				Buffer.GetRef(),
				BufferUpload.GetRef(),
				Offset,
				0,
				1,
				&SubResourceData) == 0)
			{
				return false;
			}

			if (Transition)
			{
				Buffer->SetResourceState(D3D12_RESOURCE_STATE_INDEX_BUFFER, CmdList);
			}

			return true;
		}

	public:

		explicit inline CGrpIndexBufferPair
		(
			GrpIndexBufferDescriptor * Descriptor
		) :
			CGrpBufferPair(Descriptor)
		{}

		ErrorCode Create(const D3D12_RESOURCE_STATES InitialState = D3D12_RESOURCE_STATE_COPY_DEST);
	};


	class CIndexBuffer
	{
	protected:

		CGrpIndexBufferPair Buffer;

	public:

		inline const CGrpIndexBufferPair & GetBufferData() const
		{
			return Buffer;
		}

	public:

		CIndexBuffer
		(
			GrpIndexBufferDescriptor * Descriptor
		);

		ErrorCode Create(const D3D12_RESOURCE_STATES InitialState = D3D12_RESOURCE_STATE_COPY_DEST);
	};

	inline void RGrpCommandList::SetIndexBuffer
	(
		const CIndexBuffer & IndexBuffer
	)	const
	{
		const IndexBufferView View[] =
		{
			IndexBuffer.GetBufferData().GetBufferView()
		};

		CommandList->IASetIndexBuffer(View);
	}

	inline void RGrpCommandList::SetNullIndexBuffer() const
	{
		CommandList->IASetIndexBuffer(NULL);
	}
}