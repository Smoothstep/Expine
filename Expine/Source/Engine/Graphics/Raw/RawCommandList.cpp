#include "Precompiled.h"

#include "Raw/RawCommandList.h"

namespace D3D
{
	ErrorCode RGrpCommandList::CreateRecording(const SharedPointer<RCommandAllocator>& pCommandAllocator)
	{
		CHECK_NULL_ARG(pCommandAllocator);

		ErrorCode Error = DEVICE->CreateCommandList
		(
			0,
			pCommandAllocator->GetType(),
			pCommandAllocator.GetRef(),
			0,
			IID_PPV_ARGS(&CommandList)
		);

		Type = pCommandAllocator->GetType();

		if (Error)
		{
			return Error;
		}

		CommandAllocator = &pCommandAllocator;

		IsRecording = true;
		IsCompute = Type == D3D12_COMMAND_LIST_TYPE_COMPUTE;

		return S_OK;
	}

	ErrorCode RGrpCommandList::Create(const SharedPointer<RCommandAllocator>& pCommandAllocator)
	{
		CHECK_NULL_ARG(pCommandAllocator);

		ErrorCode Error = DEVICE->CreateCommandList
		(
			0,
			pCommandAllocator->GetType(),
			pCommandAllocator.GetRef(),
			0,
			IID_PPV_ARGS(&CommandList)
		);

		Type = pCommandAllocator->GetType();

		if (Error)
		{
			return Error;
		}

		CommandAllocator = &pCommandAllocator;

		if ((Error = Close()))
		{
			return Error;
		}

		IsRecording = false;
		IsCompute = Type == D3D12_COMMAND_LIST_TYPE_COMPUTE;

		return S_OK;
	}

	ErrorCode RGrpCommandList::Create(const SharedPointer<RCommandAllocator>& pCommandAllocator, const SharedPointer<RPipelineState>& pPipelineState)
	{
		CHECK_NULL_ARG(pCommandAllocator);
		CHECK_NULL_ARG(pPipelineState);

		ErrorCode Error = DEVICE->CreateCommandList
		(
			0,
			pCommandAllocator->GetType(),
			pCommandAllocator.GetRef(),
			pPipelineState.GetRef(),
			IID_PPV_ARGS(&CommandList)
		);

		Type = pCommandAllocator->GetType();

		if (Error)
		{
			return Error;
		}

		CommandAllocator	= &pCommandAllocator;
		PipelineState		= &pPipelineState;

		if ((Error = Close()))
		{
			return Error;
		}

		IsRecording = false;
		IsCompute = Type == D3D12_COMMAND_LIST_TYPE_COMPUTE;

		return S_OK;
	}
}
