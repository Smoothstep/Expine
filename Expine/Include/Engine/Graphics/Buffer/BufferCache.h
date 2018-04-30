#pragma once

#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "CommandBuffer.h"

#include <Container/HopscotchMap.h>

namespace D3D
{
	class CBufferCache : public CSingleton<CBufferCache>
	{
	public:

		bool GetVertexBuffer
		(
			size_t							BufferSize,
			size_t							StrideSize,
			SharedPointer<CVertexBuffer> &	Buffer 
		);

		bool GetIndexBuffer
		(
			size_t							BufferSize,
			DXGI_FORMAT						Format,
			SharedPointer<CIndexBuffer> &	Buffer
		);

		ErrorCode CreateVertexBuffer
		(
			size_t							StrideCount,
			size_t							StrideSize,
			SharedPointer<CVertexBuffer> &	Buffer
		);

		ErrorCode CreateIndexBuffer
		(
			size_t						  NumIndices,
			DXGI_FORMAT					  Format,
			SharedPointer<CIndexBuffer> & Buffer
		);

		ErrorCode CreateCommandBuffer
		(
			size_t							CommandSizePerFrame,
			SharedPointer<CCommandBuffer> & Buffer
		);
	};
}