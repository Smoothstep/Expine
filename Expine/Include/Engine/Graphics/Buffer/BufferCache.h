#pragma once

#include "DirectX/D3D.h"

namespace D3D
{
	class CBufferCache : public CSingleton<CBufferCache>
	{
	public:

		bool GetBuffer
		(
			size_t							BufferSize,
			size_t							StrideSize,
			SharedPointer<CVertexBuffer> &	Buffer 
		);

		bool GetBuffer
		(
			size_t							BufferSize,
			DXGI_FORMAT						Format,
			SharedPointer<CIndexBuffer> &	Buffer
		);

		bool GetBuffer
		(
			size_t							CommandSizePerFrame,
			SharedPointer<CCommandBuffer> & Buffer
		);

		ErrorCode CreateBuffer
		(
			size_t							StrideCount,
			size_t							StrideSize,
			SharedPointer<CVertexBuffer> &	Buffer
		);

		ErrorCode CreateBuffer
		(
			size_t						  NumIndices,
			DXGI_FORMAT					  Format,
			SharedPointer<CIndexBuffer> & Buffer
		);

		ErrorCode CreateBuffer
		(
			size_t							CommandSizePerFrame,
			SharedPointer<CCommandBuffer> & Buffer
		);
	};
}