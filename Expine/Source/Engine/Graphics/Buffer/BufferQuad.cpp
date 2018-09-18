#include "Precompiled.h"
#include "Buffer/BufferQuad.h"

#include "Utils/State/StateBlend.h"
#include "Utils/State/StateRasterizer.h"

namespace D3D
{
	const D3D12_INPUT_ELEMENT_DESC TextureInputElementDesc[2] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,		0, 12,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	const D3D12_INPUT_LAYOUT_DESC CTexturedQuadBuffer::InputLayoutDesc =
	{
					TextureInputElementDesc,
		_countof(	TextureInputElementDesc)
	};

	ErrorCode CTexturedQuadBuffer::Create
	(
		const RGrpCommandList & CmdList
	)
	{
		ErrorCode Error = CQuadBuffer<TextureVertex>::Create(CmdList);

		if (Error)
		{
			return Error;
		}

		const TextureVertex Vertices[4] =
		{
			Vector3f(-1.0f,  1.0f,  1.0f), Vector2f(0.0f, 0.0f),
			Vector3f( 1.0f,  1.0f,  1.0f), Vector2f(1.0f, 0.0f),
			Vector3f( 1.0f, -1.0f,  1.0f), Vector2f(1.0f, 1.0f),
			Vector3f(-1.0f, -1.0f,  1.0f), Vector2f(0.0f, 1.0f)
		};

		if ((Error = VertexBuffer->GetBufferData().UploadData(Vertices, _countof(Vertices), CmdList)))
		{
			return Error;
		}

		return S_OK;
	}
}
