#pragma once

#include "RawPipelineState.h"

#include "IndexBuffer.h"
#include "VertexBuffer.h"

namespace D3D
{
	template
	<
		class Vertex
	>
	class CQuadBuffer
	{
	protected:

		SharedPointer<CIndexBuffer> IndexBuffer;

	protected:

		SharedPointer<CVertexBuffer> VertexBuffer;

	public:

		static const D3D12_INPUT_LAYOUT_DESC InputLayoutDesc;

	public:

		virtual ErrorCode Create
		(
			const RGrpCommandList & CmdList
		);

	public:

		inline const CVertexBuffer & GetVertexBuffer() const
		{
			return VertexBuffer.GetRef();
		}

		inline const CIndexBuffer & GetIndexBuffer() const
		{
			return IndexBuffer.GetRef();
		}
	};

	struct TextureVertex
	{
		Vector3f Position;
		Vector2f Texture;
	};

	class CTexturedQuadBuffer : public CQuadBuffer<TextureVertex>
	{
	public:

		virtual ErrorCode Create
		(
			const RGrpCommandList & CmdList
		);
	};

	template
	<
		class Vertex
	>
	inline ErrorCode CQuadBuffer<Vertex>::Create
	(
		const RGrpCommandList & CmdList
	)
	{
		ErrorCode Error;

		IndexBuffer = new CIndexBuffer(new GrpIndexBufferDescriptor(sizeof(Uint16), 6));
		{
			if ((Error = IndexBuffer->Create()))
			{
				return Error;
			}

			constexpr Uint16 Indices[6] =
			{
				0, 1, 2,
				2, 3, 0
			};

			if ((Error = IndexBuffer->GetBufferData().UploadData(Indices, _countof(Indices), CmdList)))
			{
				return Error;
			}
		}

		VertexBuffer = new CVertexBuffer(new GrpVertexBufferDescriptor(sizeof(Vertex), 4));
		{
			if ((Error = VertexBuffer->Create()))
			{
				return Error;
			}
		}

		return S_OK;
	}
}