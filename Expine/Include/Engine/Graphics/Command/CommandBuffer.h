#pragma once

#include "Buffer.h"
#include "CommandList.h"

namespace D3D
{
	class CGrpCommandBuffer : public CGrpBuffer
	{
	public:

		inline const RResource & GetBuffer() const
		{
			return Buffer.GetRef();
		}

		inline RResource * GetBufferResource() const
		{
			return Buffer.Get();
		}

	public:

		CGrpCommandBuffer
		(
			GrpCommandBufferDescriptor * pDescriptor
		);

		ErrorCode Create(const D3D12_RESOURCE_STATES InitialState = D3D12_RESOURCE_STATE_COPY_DEST);

		inline void UploadData
		(
			const void					*	Commands,
			const UINT						Size,
			const CCommandListContext	&	CmdListCtx,
			const UINT						Offset = 0,
			const bool						Transition = true
		)	const;

		template
		<
			typename IndirectCommand
		>
		inline void UploadData
		(
			const IndirectCommand		*	Commands,
			const UINT						NumCommands,
			const CCommandListContext	&	CmdListCtx,
			const UINT						Offset = 0,
			const bool						Transition = true
		)	const;
	};

	class CGrpCommandBufferPair : public CGrpBufferPair
	{
	public:

		inline const RResource & GetBuffer() const
		{
			return BufferUpload.GetRef();
		}

		inline RResource * GetBufferResource() const
		{
			return Buffer.Get();
		}

	public:

		CGrpCommandBufferPair
		(
			GrpCommandBufferDescriptor * pDescriptor
		);

		ErrorCode Create();

		inline void UploadData
		(
			const void					*	Commands,
			const UINT						Size,
			const CCommandListContext	&	CmdList,
			const UINT						Offset = 0,
			const UINT						Count = 1
		)	const;

		template
		<
			typename IndirectCommand
		>
		inline void UploadData
		(
			const IndirectCommand		*	Commands,
			const UINT						NumCommands,
			const CCommandListContext	&	CmdListCtx,
			const UINT						Offset = 0,
			const UINT						Count = 1
		)	const;
	};

	class CCommandBuffer
	{
	private:

		CGrpCommandBuffer Buffer;

	public:

		inline const CGrpCommandBuffer & GetBufferData() const
		{
			return Buffer;
		}

	public:

		CCommandBuffer
		(
			GrpCommandBufferDescriptor * Descriptor
		);

		ErrorCode Create(const D3D12_RESOURCE_STATES InitialState = D3D12_RESOURCE_STATE_COPY_DEST);
	};

	inline void RGrpCommandList::ExecuteIndirect
	(
		const UINT				NumCommands,
		const CCommandBuffer *	pCommandBuffer,
		const UINT				BufferOffset
	)	const
	{
		Ensure(CommandSignature);

		CommandList->ExecuteIndirect
		(
			CommandSignature->GetRef(),
			NumCommands,
			pCommandBuffer->GetBufferData().GetBuffer(),
			BufferOffset,
			NULL,
			0
		);
	}

	inline void RGrpCommandList::ExecuteIndirect
	(
		const UINT				NumCommands,
		const RResource &		CommandBufferResource,
		const UINT				BufferOffset
	)	const
	{
		Ensure(CommandSignature);

		CommandList->ExecuteIndirect
		(
			CommandSignature->GetRef(),
			NumCommands,
			CommandBufferResource,
			BufferOffset,
			NULL,
			0
		);
	}

	template
	<
		typename IndirectCommand
	>
	inline void CGrpCommandBuffer::UploadData
	(
		const IndirectCommand		*	Commands,
		const UINT						NumCommands,
		const CCommandListContext	&	CmdListCtx,
		const UINT						Offset,
		const bool						Transition
	)	const
	{
		Ensure
		(
			Buffer != NULL
		);

		const size_t Size = sizeof(IndirectCommand) * NumCommands;

		Ensure
		(
			Offset + Size < BufferDescriptor.GetRef()
		);

		if (Transition)
		{
			Buffer->SetResourceState(D3D12_RESOURCE_STATE_COPY_DEST, CmdListCtx);
		}

		CmdListCtx.CopyDataToBuffer
		(
			Buffer.Get(),
			Commands,
			Size
		);

		if (Transition)
		{
			Buffer->SetResourceState(D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, CmdListCtx);
		}
	}

	inline void CGrpCommandBuffer::UploadData
	(
		const void					*	Commands,
		const UINT						Size,
		const CCommandListContext	&	CmdListCtx,
		const UINT						Offset,
		const bool						Transition
	)	const
	{
		Ensure
		(
			Buffer != NULL
		);

		Ensure
		(
			Offset + Size < BufferDescriptor.GetRef()
		);

		if (Transition)
		{
			Buffer->SetResourceState(D3D12_RESOURCE_STATE_COPY_DEST, CmdListCtx);
		}

		CmdListCtx.CopyDataToBuffer
		(
			Buffer.Get(),
			Commands,
			Size
		);

		if (Transition)
		{
			Buffer->SetResourceState(D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, CmdListCtx);
		}
	}

	template
	<
		class IndirectCommand
	>
	inline void CGrpCommandBufferPair::UploadData
	(
		const IndirectCommand		*	Commands,
		const UINT						NumCommands,
		const CCommandListContext	&	CmdList,
		const UINT						Offset,
		const UINT						Count
	)	const
	{
		Ensure
		(
			Buffer != NULL
		);

		Ensure
		(
			BufferUpload != NULL
		);

		const size_t Size = sizeof(IndirectCommand) * NumCommands;

		Ensure
		(
			Offset + Size < BufferDescriptor.GetRef()
		);

		Buffer->SetResourceState(D3D12_RESOURCE_STATE_COPY_DEST, CmdList);

		D3D12_SUBRESOURCE_DATA SubResourceData;
		{
			SubResourceData.pData		= Commands;
			SubResourceData.RowPitch	= Size;
			SubResourceData.SlicePitch	= 1;
		}

		UINT64 iRequiredSize;

		for (UINT N = 0; N < Count; ++N)
		{
			if ((iRequiredSize = UpdateSubresources<1>(
				CmdList,
				Buffer.GetRef(),
				BufferUpload.GetRef(),
				Offset + N * Size,
				0,
				1,
				&SubResourceData)) == 0)
			{
				throw(ECException(E_FAIL));
			}
		}

		Buffer->SetResourceState(D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, CmdList);
	}

	inline void CGrpCommandBufferPair::UploadData
	(
		const void					*	Commands,
		const UINT						Size,
		const CCommandListContext	&	CmdList,
		const UINT						Offset,
		const UINT						Count
	)	const
	{
		Ensure
		(
			Buffer != NULL
		);

		Ensure
		(
			BufferUpload != NULL
		);

		Ensure
		(
			Offset + Size < BufferDescriptor.GetRef()
		);

		Buffer->SetResourceState(D3D12_RESOURCE_STATE_COPY_DEST, CmdList);

		D3D12_SUBRESOURCE_DATA SubResourceData;
		{
			SubResourceData.pData		= Commands;
			SubResourceData.RowPitch	= Size;
			SubResourceData.SlicePitch	= 1;
		}

		UINT64 iRequiredSize;

		for (UINT N = 0; N < Count; ++N)
		{
			if ((iRequiredSize = UpdateSubresources(
				CmdList,
				Buffer.GetRef(),
				BufferUpload.GetRef(),
				Offset + N * Size,
				0,
				1,
				&SubResourceData)) == 0)
			{
				throw(ECException(E_FAIL));
			}
		}

		Buffer->SetResourceState(D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, CmdList);
	}
}