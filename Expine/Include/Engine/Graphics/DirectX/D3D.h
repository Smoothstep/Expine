#pragma once
#pragma warning(disable:1563)

#include <DX12Helper.h>

#include "Smart.h"
#include "Singleton.h"
#include "Defines.h"
#include "Hyper.h"
#include "Memory/Memory.h"
#include "Default.h"

#include "Utils/ErrorLog.h"

#undef min
#undef max
#undef CopyMemory

namespace D3D
{
#ifdef EXPINEGRAPHICS_EXPORTS
#define _EX_ __declspec(dllexport)
#else 
#define _EX_ __declspec(dllimport)
#endif
	class CScene;
	class CSceneController;
	class CScreen;
	class CScreenInputHandler;
	class CSceneRenderer;
	class CSceneView;
	class CSceneArea;
	class CSceneIndoor;
	class CSceneOutdoor;
	class CSceneLight;
	class CCamera;
	class CCommandListContext;
	class CCommandList;
	class CCGraphicsCommandList;
	class CVertexBuffer;
	class CIndexBuffer;
	class CCommandBuffer;
	class CCommandQueueDirect;
	class CCommandQueueCopy;
	class CCommandQueueCompute;
	class PipelineObjectBase;
	class RShader;

	using IFence				= ID3D12Fence;
	using IResource				= ID3D12Resource;
	using IGrpCommandList		= ID3D12GraphicsCommandList;
	using IPipelineState		= ID3D12PipelineState;
	using IRootSignature		= ID3D12RootSignature;
	using ICommandSignature		= ID3D12CommandSignature;
	using ICommandAllocator		= ID3D12CommandAllocator;
	using ICommandQueue			= ID3D12CommandQueue;
	using IDevice				= ID3D12Device;
	using IDescriptorHeap		= ID3D12DescriptorHeap;
	using IHeap					= ID3D12Heap;
	using IBlob					= ID3DBlob;
	
	using ShaderMacro			= D3D_SHADER_MACRO;
	using ShaderInclude			= ID3DInclude;

	using LPFence				= IFence*;
	using LPResource			= IResource*;
	using LPDescriptorHeap		= IDescriptorHeap*;
	using LPGrpCommandList		= IGrpCommandList*;
	using LPipelineState		= IPipelineState*;

	using IndexBufferView		= D3D12_INDEX_BUFFER_VIEW;
	using VertexBufferView		= D3D12_VERTEX_BUFFER_VIEW;
	using ConstantBufferView	= D3D12_CONSTANT_BUFFER_VIEW_DESC;

	inline D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveTopologyType
	(
		const D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology
	)
	{
		switch (PrimitiveTopology)
		{
			case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
			case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
			case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ:
			case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ:
			{
				return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			}

			case D3D_PRIMITIVE_TOPOLOGY_LINELIST:
			case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP:
			case D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ:
			case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ:
			{
				return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
			}

			case D3D_PRIMITIVE_TOPOLOGY_POINTLIST:
			{
				return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
			}
		}

		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	}

#define INPUT_LAYOUT(X) { X, _countof(X) }
#define FRAME_COUNT 2
}