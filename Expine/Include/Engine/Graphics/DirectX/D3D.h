#pragma once
#pragma warning(disable:1563)

#include <DX12Helper.h>

#include <d3dcommon.h>

#include "Smart.h"
#include "Singleton.h"
#include "Defines.h"
#include "Types.h"
#include "Hyper.h"
#include "ErrorLog.h"
#include "Memory.h"
#include "Default.h"

#define CHECK_NULL_ARG(X)		\
	if((X) == NULL)				\
	{							\
		return E_INVALIDARG;	\
	}							\

namespace D3D
{
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