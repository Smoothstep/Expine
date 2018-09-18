#pragma once

#include "Buffer/BufferCache.h"
#include "Buffer/BufferVertex.h"
#include "Buffer/BufferIndex.h"
#include "Buffer/BufferCommand.h"

namespace D3D
{
	namespace Geometries
	{
		static const Vector3f CubeVertices[] =
		{
			{ -1.0, -1.0,  1.0 },
			{  1.0, -1.0,  1.0 },
			{  1.0,  1.0,  1.0 },
			{ -1.0,  1.0,  1.0 },
			{ -1.0, -1.0, -1.0 },
			{  1.0, -1.0, -1.0 },
			{  1.0,  1.0, -1.0 },
			{ -1.0,  1.0, -1.0 },
		};

		static const Uint16 CubeIndices[] =
		{
			0, 1, 2,
			2, 3, 0,
			1, 5, 6,
			6, 2, 1,
			7, 6, 5,
			5, 4, 7,
			4, 0, 3,
			3, 7, 4,
			4, 5, 1,
			1, 0, 4,
			3, 2, 6,
			6, 7, 3,
		};
	}

	template
	<
		typename Vertex,
		typename Index		= Uint16,
		typename Command	= D3D12_DRAW_INDEXED_ARGUMENTS
	>
	class CIGeometryBuffer
	{
	protected:

		TVector<Vertex>		VertexData;
		TVector<Index>		IndexData;
		TVector<Command>	CommandData;

	private:

		SharedPointer<CIndexBuffer> IndexBuffer;
		SharedPointer<CVertexBuffer> VertexBuffer;
		SharedPointer<CCommandBuffer> CommandBuffer;

	private:

		inline ErrorCode CreateIndexBuffer();
		inline ErrorCode CreateVertexBuffer();
		inline ErrorCode CreateCommandBuffer();
		inline ErrorCode CreateIndexBufferDynamic();
		inline ErrorCode CreateVertexBufferDynamic();
		inline ErrorCode CreateCommandBufferDynamic();

	public:

		inline TVector<Vertex> & GetVertices()
		{
			return VertexData;
		}

		inline TVector<Index> & GetIndices()
		{
			return IndexData;
		}

		inline TVector<Command> & GetCommands()
		{
			return CommandData;
		}

	public:

		inline ErrorCode CreateBuffers();
		inline ErrorCode CreateBuffersDynamic();

		void UploadData
		(
			const CCommandListContext & CmdListCtx
		)	const;

		inline void ApplyBuffers
		(
			const RGrpCommandList & CmdList
		)	const
		{
			CmdList.SetVertexBuffer(VertexBuffer.GetRef());
			CmdList.SetIndexBuffer(IndexBuffer.GetRef());
		}

		inline void ExecuteIndirect
		(
			const	RGrpCommandList & CmdList,
					UINT NumCommands	= 0,
					UINT Offset			= 0
		)	const
		{
			if (NumCommands == 0)
			{
				NumCommands = CommandData.size();
			}

			CmdList.ExecuteIndirect(NumCommands, CommandBuffer.Get(), Offset * sizeof(Command));
		}

		inline void AddLine
		(
			const Vertex & Start,
			const Vertex & End
		)
		{
			IndexData.push_back(IndexData.size());
			IndexData.push_back(IndexData.size());

			VertexData.push_back(Start);
			VertexData.push_back(End);
		}

		inline void AddLine
		(
			const Vertex &	Start,
			const Vertex &	End,
			const Index		IndexStart,
			const Index		IndexEnd
		)
		{
			IndexData.push_back(IndexStart);
			IndexData.push_back(IndexEnd);

			VertexData.push_back(Start);
			VertexData.push_back(End);
		}

		inline void AddTri
		(
			const Vertex &	P0,
			const Vertex &	P1,
			const Vertex &	P2,
			const Index		I0,
			const Index		I1,
			const Index		I2
		)
		{
			IndexData.push_back(I0);
			IndexData.push_back(I1);
			IndexData.push_back(I2);

			VertexData.push_back(P0);
			VertexData.push_back(P1);
			VertexData.push_back(P2);
		}

		inline void AddCommand
		(
			const Command & Cmd
		)
		{
			CommandData.push_back(Cmd);
		}

		inline void AddFaces
		(
			const Vertex	* LineVertices,
			const Index		* LineIndices,
			const Index		  NumVertices,
			const Index		  NumIndices
		)
		{
			IndexData.insert
			(
				IndexData.end(),
				LineIndices,
				LineIndices + NumIndices
			);

			VertexData.insert
			(
				VertexData.end(),
				LineVertices,
				LineVertices + NumVertices
			);
		}
	};

	template
	<
		typename Vertex, 
		typename Index, 
		typename Command
	>
	inline ErrorCode CIGeometryBuffer<Vertex, Index, Command>::CreateBuffers()
	{
		ErrorCode Error;

		if ((Error = CreateVertexBuffer()))
		{
			return Error;
		}

		if ((Error = CreateIndexBuffer()))
		{
			return Error;
		}

		if ((Error = CreateCommandBuffer()))
		{
			return Error;
		}

		return S_OK;
	}

	template
	<
		typename Vertex, 
		typename Index, 
		typename Command
	>
	inline ErrorCode CIGeometryBuffer<Vertex, Index, Command>::CreateBuffersDynamic()
	{
		ErrorCode Error;

		if ((Error = CreateVertexBufferDynamic()))
		{
			return Error;
		}

		if ((Error = CreateIndexBufferDynamic()))
		{
			return Error;
		}

		if ((Error = CreateCommandBufferDynamic()))
		{
			return Error;
		}

		return S_OK;
	}

	template
	<
		typename Vertex, 
		typename Index, 
		typename Command
	>
	inline void CIGeometryBuffer<Vertex, Index, Command>::UploadData(const CCommandListContext & CmdListCtx) const
	{
		RResource * Resources[] =
		{
			IndexBuffer->GetBufferData().GetBufferResource(),
			VertexBuffer->GetBufferData().GetBufferResource(),
			CommandBuffer->GetBufferData().GetBufferResource()
		};

		D3D12_RESOURCE_STATES ResourceStatesBegin[] =
		{
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_COPY_DEST
		};

		RResource::SetResourceStates<_countof(Resources)>
		(
			Resources,
			ResourceStatesBegin,
			CmdListCtx,
			D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES
		);

		IndexBuffer->GetBufferData().UploadData
		(
			IndexData.data(),
			IndexData.size(),
			CmdListCtx,
			0,
			false
		);

		VertexBuffer->GetBufferData().UploadData
		(
			VertexData.data(),
			VertexData.size(),
			CmdListCtx,
			0,
			false
		);

		CommandBuffer->GetBufferData().UploadData
		(
			CommandData.data(),
			CommandData.size(),
			CmdListCtx,
			0,
			false
		);

		D3D12_RESOURCE_STATES ResourceStatesEnd[] =
		{
			D3D12_RESOURCE_STATE_INDEX_BUFFER,
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
			D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT
		};

		RResource::SetResourceStates<_countof(Resources)>
		(
			Resources,
			ResourceStatesEnd,
			CmdListCtx,
			D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES
		);
	}

	template
	<
		typename Vertex, 
		typename Index, 
		typename Command
	>
	inline ErrorCode CIGeometryBuffer<Vertex, Index, Command>::CreateIndexBuffer()
	{
		static constexpr unsigned IndexSize = sizeof(Index);

		static constexpr DXGI_FORMAT IndexFormat = 
			IndexSize == sizeof(Uint16) ? DXGI_FORMAT_R16_UINT : 
			IndexSize == sizeof(Uint32) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R8_UINT;

		return CBufferCache::Instance().CreateBuffer(IndexData.size(), IndexFormat, IndexBuffer);
	}

	template
	<
		typename Vertex, 
		typename Index, 
		typename Command
	>
	inline ErrorCode CIGeometryBuffer<Vertex, Index, Command>::CreateVertexBuffer()
	{
		return CBufferCache::Instance().CreateBuffer(VertexData.size(), sizeof(Vertex), VertexBuffer);
	}

	template
	<
		typename Vertex, 
		typename Index, 
		typename Command
	>
	inline ErrorCode CIGeometryBuffer<Vertex, Index, Command>::CreateCommandBuffer()
	{
		return CBufferCache::Instance().CreateBuffer(sizeof(Command), CommandBuffer);
	}

	template
	<
		typename Vertex, 
		typename Index, 
		typename Command
	>
	inline ErrorCode CIGeometryBuffer<Vertex, Index, Command>::CreateIndexBufferDynamic()
	{
		static constexpr unsigned IndexSize = sizeof(Index);

		static constexpr DXGI_FORMAT IndexFormat =
			IndexSize == sizeof(Uint16) ? DXGI_FORMAT_R16_UINT :
			IndexSize == sizeof(Uint32) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R8_UINT;

		ErrorCode Error;

		UniquePointer<CIndexBuffer> Buffer = new CIndexBuffer(new GrpIndexBufferDescriptor(IndexFormat, IndexData.size()));
		{
			if ((Error = Buffer->Create(D3D12_RESOURCE_STATE_INDEX_BUFFER)))
			{
				return Error;
			}
		}

		IndexBuffer = Buffer.Detach();

		return S_OK;
	}

	template
	<
		typename Vertex, 
		typename Index, 
		typename Command
	>
	inline ErrorCode CIGeometryBuffer<Vertex, Index, Command>::CreateVertexBufferDynamic()
	{
		ErrorCode Error;

		UniquePointer<CVertexBuffer> Buffer = new CVertexBuffer(new GrpVertexBufferDescriptor(sizeof(Vertex), VertexData.size()));
		{
			if ((Error = Buffer->Create(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)))
			{
				return Error;
			}
		}

		VertexBuffer = Buffer.Detach();

		return S_OK;
	}

	template
	<
		typename Vertex, 
		typename Index, 
		typename Command
	>
	inline ErrorCode CIGeometryBuffer<Vertex, Index, Command>::CreateCommandBufferDynamic()
	{
		ErrorCode Error;

		UniquePointer<CCommandBuffer> Buffer = new CCommandBuffer(new GrpCommandBufferDescriptor(sizeof(Command), CommandData.size()));
		{
			if ((Error = Buffer->Create(D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT)))
			{
				return Error;
			}
		}

		CommandBuffer = Buffer.Detach();

		return S_OK;
	}
}