#pragma once

#include "Buffer.h"

namespace D3D
{
	class CGrpVertexBuffer : public CGrpBuffer
	{
	private:

		VertexBufferView VBV;

	public:

		inline const VertexBufferView & GetBufferView() const
		{
			return VBV;
		}

	public:

		template
		<
			typename Vertex
		>
		inline bool UploadData
		(
			const Vertex				*	Vertices,
			const UINT						NumVertices,
			const CCommandListContext	&	CmdListCtx,
			const UINT						Offset = 0,
			const bool						Transition = true
		)	const;

	public:

		explicit inline CGrpVertexBuffer
		(
			GrpVertexBufferDescriptor * Descriptor
		) :
			CGrpBuffer(Descriptor)
		{}

		ErrorCode Create();
		ErrorCode Create
		(
			const VirtualResourceAddress & UploadBufferVA
		);
	};

	class CGrpVertexBufferPair : public CGrpBufferPair
	{
	private:

		VertexBufferView VBV;

	public:

		inline const VertexBufferView & GetBufferView() const
		{
			return VBV;
		}

	public:

		template
		<
			typename Vertex
		>
		inline bool UploadData
		(
			const Vertex			*	Vertices,
			const UINT					NumVertices,
			const RGrpCommandList	&	CmdList,
			const UINT					Offset = 0,
			const bool					Transition = true
		)	const;

	public:

		explicit inline CGrpVertexBufferPair
		(
			GrpVertexBufferDescriptor * Descriptor
		) :
			CGrpBufferPair(Descriptor)
		{}

		ErrorCode Create
		(
			const D3D12_RESOURCE_STATES InitialState = D3D12_RESOURCE_STATE_COPY_DEST
		);
		ErrorCode Create
		(
			const VirtualResourceAddress & UploadBufferVA
		);
	};

	class CVertexBuffer
	{
	protected:

		CGrpVertexBufferPair Buffer;

	public:

		inline const CGrpVertexBufferPair & GetBufferData() const
		{
			return Buffer;
		}

	public:

		CVertexBuffer
		(
			GrpVertexBufferDescriptor * BufferDescriptor
		);

		ErrorCode Create
		(
			const D3D12_RESOURCE_STATES InitialState = D3D12_RESOURCE_STATE_COPY_DEST
		);
	};

	class CVertexBuffers
	{
	private:

		TVector<UniquePointer<CVertexBuffer> >		VertexBuffers;
		TVector<VertexBufferView>					VertexBufferViews;

	public:

		inline void AddVertexBuffer
		(
			CVertexBuffer * pBuffer
		)
		{
			VertexBuffers.push_back(pBuffer);
			{
				VertexBufferViews.push_back(pBuffer->GetBufferData().GetBufferView());
			}
		}

		inline UINT GetBufferCount() const
		{
			return VertexBuffers.size();
		}

		inline const VertexBufferView * GetViews() const
		{
			return VertexBufferViews.data();
		}
	};

	template
	<
		typename Vertex
	>
	inline bool CGrpVertexBuffer::UploadData
	(
		const Vertex				*	Vertices,
		const UINT						NumVertices,
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
			BufferUpload != NULL
		);

		const size_t Size = sizeof(Vertex) * NumVertices;

		Ensure
		(
			Offset < VBV.SizeInBytes
		);

		Ensure
		(
			Size < VBV.SizeInBytes
		);

		if (Transition)
		{
			Buffer->SetResourceState(D3D12_RESOURCE_STATE_COPY_DEST, CmdListCtx);
		}

		CmdListCtx.CopyDataToBuffer
		(
			Buffer.Get(),
			Vertices,
			Size
		);

		if (Transition)
		{
			Buffer->SetResourceState(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, CmdListCtx);
		}

		return true;
	}

	template
	<
		typename Vertex
	>
	inline bool CGrpVertexBufferPair::UploadData
	(
		const Vertex			*	Vertices,
		const UINT					NumVertices,
		const RGrpCommandList	&	CmdList,
		const UINT					Offset,
		const bool					Transition
	)	const
	{
		Ensure
		(
			Buffer != NULL
		);

		const size_t Size = sizeof(Vertex) * NumVertices;

		Ensure
		(
			Offset < VBV.SizeInBytes
		);

		Ensure
		(
			Size < VBV.SizeInBytes
		);

		if (Transition)
		{
			Buffer->SetResourceState(D3D12_RESOURCE_STATE_COPY_DEST, CmdList);
		}

		D3D12_SUBRESOURCE_DATA SubResourceData;
		{
			SubResourceData.pData		= Vertices;
			SubResourceData.RowPitch	= Size;
			SubResourceData.SlicePitch	= Size;
		}

		UINT64 iRequiredSize;

		if ((iRequiredSize = UpdateSubresources(
			CmdList,
			Buffer.GetRef(),
			BufferUpload.GetRef(),
			Offset,
			0,
			1,
			&SubResourceData)) == 0)
		{
			return false;
		}

		if (Transition)
		{
			Buffer->SetResourceState(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, CmdList);
		}

		return true;
	}

	inline void RGrpCommandList::SetVertexBuffer
	(
		const CVertexBuffer &	VertexBuffer,
		const UINT				Slot
	)	const
	{
		VertexBufferView View[] =
		{
			VertexBuffer.GetBufferData().GetBufferView()
		};

		CommandList->IASetVertexBuffers(Slot, 1, View);
	}

	inline void RGrpCommandList::SetVertexBuffers
	(
		const CVertexBuffers & VertexBuffers
	)	const
	{
		CommandList->IASetVertexBuffers
		(
			0,
			VertexBuffers.GetBufferCount(),
			VertexBuffers.GetViews()
		);
	}

	inline void RGrpCommandList::SetNullVertexBuffer() const
	{
		CommandList->IASetVertexBuffers(0, 0, NULL);
	}
}