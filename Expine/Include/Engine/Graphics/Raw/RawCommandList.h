#pragma once

#include "RawCommandAllocator.h"
#include "RawCommandSignature.h"
#include "RawPipelineState.h"

#include "BufferTypes.h"

namespace D3D
{
	class RDescriptorHeap;
	class RRenderTargetView;
	class RDepthStencilView;
	class RShaderResourceView;
	class RUnorderedAccessView;
	class RResource;

	struct DescriptorHeapEntry;
	struct DescriptorHeapRange;

	class RGrpCommandList
	{
	private:

		ComPointer<IGrpCommandList>	CommandList;

		bool IsRecording;
		bool IsCompute;

	protected:

		ConstPointer<SharedPointer<RCommandAllocator> >	CommandAllocator;
		ConstPointer<SharedPointer<RCommandSignature> >	CommandSignature;

		ConstPointer<SharedPointer<RRootSignature> >		RootSignature;
		ConstPointer<SharedPointer<RRootSignature> >		RootSignatureCompute;

		ConstPointer<SharedPointer<RPipelineState> >		PipelineState;
		ConstPointer<SharedPointer<RPipelineState> >		PipelineStateCompute;

		D3D12_COMMAND_LIST_TYPE Type;
		
	public:

		inline operator IGrpCommandList*() const
		{
			return CommandList.Get();
		}

		inline IGrpCommandList * operator->() const
		{
			return CommandList.Get();
		}

		inline IGrpCommandList * Get() const
		{
			return CommandList.Get();
		}

		inline IGrpCommandList * GetCommandList() const
		{
			return CommandList.Get();
		}

		inline RCommandAllocator * GetCommandAllocator() const
		{
			return CommandAllocator.Get()->Get();
		}

		inline RPipelineState * GetPipelineState() const
		{
			return PipelineState.Get()->Get();
		}

		inline D3D12_COMMAND_LIST_TYPE GetType() const
		{
			return Type;
		}

		inline bool IsOpened() const
		{
			return IsRecording;
		}

		inline bool IsComputeCommandList() const
		{
			return IsCompute;
		}

	public:

		ErrorCode CreateRecording
		(
			const SharedPointer<RCommandAllocator> & pCommandAllocator
		);

		ErrorCode Create
		(
			const SharedPointer<RCommandAllocator> & pCommandAllocator
		);

		ErrorCode Create
		(
			const SharedPointer<RCommandAllocator>	& pCommandAllocator,
			const SharedPointer<RPipelineState>		& pPipelineState
		);

		/****************************************************************************
		*
		*	Applies a pipeline. Can only be called on a recording commandlist.
		*
		****************************************************************************/

		inline void ApplyPipelineState
		(
			const SharedPointer<RPipelineState> & pPipelineState
		)
		{
			PipelineState = &pPipelineState;
			{
				CommandList->SetGraphicsRootSignature(pPipelineState->GetRootSignature());
				CommandList->SetPipelineState(pPipelineState.GetRef());
			}
		}

		inline void ApplyPipelineStateCompute
		(
			const SharedPointer<RPipelineState> & pPipelineState
		)
		{
			PipelineState = &pPipelineState;
			{
				CommandList->SetComputeRootSignature(pPipelineState->GetRootSignature());
				CommandList->SetPipelineState(pPipelineState.GetRef());
			}
		}

		/****************************************************************************
		*
		*	Only changes the pipeline which is set on the next reset call.
		*
		****************************************************************************/

		inline void SetPipelineState
		(
			const SharedPointer<RPipelineState> & pPipelineState
		)
		{
			PipelineState = &pPipelineState;
		}

		inline void SetCommandAllocator
		(
			const SharedPointer<RCommandAllocator> & pAllocator
		)
		{
			CommandAllocator = &pAllocator;
		}

		inline void SetCommandSignature
		(
			const SharedPointer<RCommandSignature> & pSignature
		)
		{
			CommandSignature = &pSignature;
		}

		inline ErrorCode Reset()
		{
			ErrorCode Error;

			if (PipelineState)
			{
				Error = CommandList->Reset(CommandAllocator->GetRef(), PipelineState->GetRef());

				if (!Error)
				{
					if (Type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
					{
						CommandList->SetComputeRootSignature(PipelineState->Get()->GetRootSignature());
					}
					else
					{
						CommandList->SetGraphicsRootSignature(PipelineState->Get()->GetRootSignature());
					}
				}
			}
			else
			{
				Error = CommandList->Reset(CommandAllocator->GetRef(), NULL);
			}

			IsRecording = true;

			return S_OK;
		}

		inline ErrorCode Reset
		(
			const SharedPointer<RPipelineState> & pPipelineState
		)
		{
			ErrorCode Error;

			if (pPipelineState)
			{
				Error = CommandList->Reset(CommandAllocator->GetRef(), pPipelineState.GetRef());

				if (!Error)
				{
					if (Type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
					{
						CommandList->SetComputeRootSignature(pPipelineState->GetRootSignature());
					}
					else
					{
						CommandList->SetGraphicsRootSignature(pPipelineState->GetRootSignature());
					}
				}
			}
			else
			{
				Error = CommandList->Reset(CommandAllocator->GetRef(), NULL);
			}

			PipelineState = &pPipelineState;

			IsRecording = true;

			return S_OK;
		}

		inline ErrorCode Reset
		(
			const SharedPointer<RPipelineState>		& pPipelineState,
			const SharedPointer<RCommandAllocator>	& pCommandAllocator
		)
		{
			ErrorCode Error;

			if (pPipelineState)
			{
				Error = CommandList->Reset(pCommandAllocator.GetRef(), pPipelineState.GetRef());
				
				if (!Error)
				{
					if (Type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
					{
						CommandList->SetComputeRootSignature(pPipelineState->GetRootSignature());
					}
					else
					{
						CommandList->SetGraphicsRootSignature(pPipelineState->GetRootSignature());
					}
				}
			}
			else
			{
				Error = CommandList->Reset(pCommandAllocator.GetRef(), NULL);
			}

			PipelineState		= &pPipelineState;
			CommandAllocator	= &pCommandAllocator;

			IsRecording = true;
			
			return S_OK;
		}

		inline ErrorCode Reset
		(
			const SharedPointer<RCommandAllocator> & pCommandAllocator
		)
		{
			ErrorCode Error;

			if (PipelineState)
			{
				Error = CommandList->Reset(pCommandAllocator.GetRef(), PipelineState.Get()->GetRef());

				if (!Error)
				{
					if (Type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
					{
						CommandList->SetComputeRootSignature(PipelineState->Get()->GetRootSignature());
					}
					else
					{
						CommandList->SetGraphicsRootSignature(PipelineState->Get()->GetRootSignature());
					}
				}
			}
			else
			{
				Error = CommandList->Reset(pCommandAllocator.GetRef(), NULL);
			}

			CommandAllocator = &pCommandAllocator;

			IsRecording = true;

			return S_OK;
		}

		inline ErrorCode Close();

		inline void Dispatch
		(
			const UINT ThreadGroupCountX,
			const UINT ThreadGroupCountY,
			const UINT ThreadGroupCountZ
		)	const;

		inline void SetIndexBuffer
		(
			const CIndexBuffer & IndexBuffer
		)	const;

		inline void SetNullIndexBuffer() const;

		inline void SetVertexBuffer
		(
			const CVertexBuffer &	VertexBuffer,
			const UINT				Slot = 0
		)	const;

		inline void SetNullVertexBuffer() const;

		inline void SetVertexBuffers
		(
			const CVertexBuffers & VertexBuffers
		)	const;

		inline void SetConstantBuffer
		(
			const UINT				RootParameterIndex,
			const CConstantBuffer & ConstantBuffer
		)	const;

		inline void SetConstantBuffer
		(
			const UINT						RootParameterIndex,
			const D3D12_GPU_VIRTUAL_ADDRESS GPUAddress
		)	const;

		inline void SetConstantBufferCompute
		(
			const UINT				RootParameterIndex,
			const CConstantBuffer & ConstantBuffer
		)	const;

		inline void SetConstantBufferCompute
		(
			const UINT						RootParameterIndex,
			const D3D12_GPU_VIRTUAL_ADDRESS GPUAddress
		)	const;

		inline void SetViewport
		(
			const D3D12_VIEWPORT & Viewport
		)	const;

		inline void SetScissorRect
		(
			const D3D12_RECT & ScissorRect
		)	const;

		inline void SetPrimitiveTopology
		(
			const D3D12_PRIMITIVE_TOPOLOGY Topology
		)	const;

		inline void SetDescriptorHeap
		(
			const RDescriptorHeap * pDescriptorHeap
		)	const;

		inline void SetDescriptorHeaps
		(
			const UINT						NumDescriptorHeaps,
			const RDescriptorHeap * const *	ppDescriptorHeaps
		)	const;

		template
		<
			size_t NumDescriptorHeaps
		>
		inline void SetDescriptorHeaps
		(
			const RDescriptorHeap * const * ppDescriptorHeaps
		)	const;

		inline void SetGraphicsRootDescriptorTable
		(
			const UINT				RootParameter,
			const RDescriptorHeap * pDescriptorHeap
		)	const;

		inline void SetComputeRootDescriptorTable
		(
			const UINT				RootParameter,
			const RDescriptorHeap * pDescriptorHeap
		)	const;

		inline void RGrpCommandList::SetGraphicsRootDescriptorTable
		(
			const UINT				RootParameter,
			const UINT				Offset,
			const RDescriptorHeap * pDescriptorHeap
		)	const;

		inline void RGrpCommandList::SetComputeRootDescriptorTable
		(
			const UINT				RootParameter,
			const UINT				Offset,
			const RDescriptorHeap * pDescriptorHeap
		)	const;

		inline void SetGraphicsRootDescriptorTable
		(
			const UINT					RootParameter,
			const DescriptorHeapEntry & DescriptorHeapEntry
		)	const;

		inline void SetComputeRootDescriptorTable
		(
			const UINT					RootParameter,
			const DescriptorHeapEntry & DescriptorHeapEntry
		)	const;

		inline void SetGraphicsRootDescriptorTable
		(
			const UINT				RootParameter,
			const RDescriptorHeap * pDescriptorHeap,
			const UINT				Offset
		)	const;

		inline void SetComputeRootDescriptorTable
		(
			const UINT				RootParameter,
			const RDescriptorHeap * pDescriptorHeap,
			const UINT				Offset
		)	const;

		inline void SetGraphicsRoot32BitConstants
		(
			const UINT				RootParameter,
			const UINT				Num32Bits,
			const void			*	Data
		)	const;

		inline void SetComputeRoot32BitConstants
		(
			const UINT				RootParameter,
			const UINT				Num32Bits,
			const void			*	Data
		)	const;

		inline void DrawIndexedInstanced
		(
			const UINT NumInstances,
			const UINT NumIndices
		)	const;

		inline void DrawIndexedInstanced
		(
			const UINT NumInstances,
			const UINT NumIndices,
			const UINT IndexStart
		)	const;

		inline void DrawInstanced
		(
			const UINT NumInstances,
			const UINT NumVertices
		)	const;

		inline void ExecuteIndirect
		(
			const UINT				NumCommands,
			const CCommandBuffer *	pCommandBuffer,
			const UINT				BufferOffset = 0
		)	const;

		inline void ExecuteIndirect
		(
			const UINT				NumCommands,
			const RResource &		CommandBufferResource,
			const UINT				BufferOffset = 0
		)	const;

		template
		<
			size_t NumRenderTargets
		>
		inline void SetRenderTargets
		(
			const RRenderTargetView * RenderTargets[NumRenderTargets],
			const RDepthStencilView & DepthStencil
		)	const;

		template
		<
			size_t NumRenderTargets
		>
		inline void SetRenderTargets
		(
			const RRenderTargetView * RenderTargets[NumRenderTargets]
		)	const;

		inline void SetRenderTarget
		(
			const RRenderTargetView & RenderTarget,
			const RDepthStencilView & DepthStencil
		)	const;

		inline void SetRenderTarget
		(
			const RRenderTargetView & RenderTarget
		)	const;

		inline void SetRenderTarget
		(
			const RDepthStencilView & DepthStencil
		)	const;

		inline void SetShaderResourceView
		(
			const UINT						RootParameter,
			const RShaderResourceView	&	ShaderResourceView
		)	const;

		inline void SetShaderResourceViewCompute
		(
			const UINT						RootParameter,
			const RShaderResourceView	&	ShaderResourceView
		)	const;

		inline void SetUnorderedAccessView
		(
			const UINT						RootParameter,
			const RUnorderedAccessView	&	UnorderedAccessView
		)	const;

		inline void SetUnorderedAccessViewCompute
		(
			const UINT						RootParameter,
			const RUnorderedAccessView	&	UnorderedAccessView
		)	const;

		inline void SetShaderResourceView
		(
			const UINT						RootParameter,
			const D3D12_GPU_VIRTUAL_ADDRESS GPUAddress 
		)	const;

		inline void SetShaderResourceViewCompute
		(
			const UINT						RootParameter,
			const D3D12_GPU_VIRTUAL_ADDRESS GPUAddress
		)	const;

		inline void CopyBufferRegion
		(
			const RResource *	pBufferDestination,
			const UINT64		OffsetDestination,
			const RResource *	pBufferSource,
			const UINT64		OffsetSource,
			const UINT64		NumBytes
		)	const;

		inline void CopySubresource
		(
			const RResource *	pDestination,
			const UINT			IndexDestination,
			const RResource *	pSource,
			const UINT			IndexSource
		)	const;

		inline void Clear
		(
			const RDepthStencilView & DepthStencil
		)	const;

		inline void Clear
		(
			const RRenderTargetView & RenderTarget
		)	const;

		template
		<
			UINT Count
		>
		inline void MakeResourceBarriers
		(
			const D3D12_RESOURCE_BARRIER Barriers[Count]
		);
	};

	template
	<
		UINT Count
	>
	inline void RGrpCommandList::MakeResourceBarriers
	(
		const D3D12_RESOURCE_BARRIER Barriers[Count]
	)
	{
		CommandList->ResourceBarrier(Count, Barriers);
	}

	inline void RGrpCommandList::SetViewport
	(
		const D3D12_VIEWPORT & Viewport
	)	const
	{
		CommandList->RSSetViewports(1, &Viewport);
	}

	inline void RGrpCommandList::SetScissorRect
	(
		const D3D12_RECT & ScissorRect
	)	const
	{
		CommandList->RSSetScissorRects(1, &ScissorRect);
	}

	inline void RGrpCommandList::SetPrimitiveTopology
	(
		const D3D12_PRIMITIVE_TOPOLOGY Topology
	)	const
	{
		CommandList->IASetPrimitiveTopology(Topology);
	}

	inline void RGrpCommandList::DrawIndexedInstanced
	(
		const UINT NumInstances, 
		const UINT NumIndices
	)	const
	{
		CommandList->DrawIndexedInstanced(NumIndices, NumInstances, 0, 0, 0);
	}

	inline void RGrpCommandList::DrawIndexedInstanced(const UINT NumInstances, const UINT NumIndices, const UINT IndexStart) const
	{
		CommandList->DrawIndexedInstanced(NumIndices, NumInstances, IndexStart, 0, 0);
	}

	inline void RGrpCommandList::DrawInstanced
	(
		const UINT NumInstances,
		const UINT NumVertices
	)	const
	{
		CommandList->DrawInstanced(NumVertices, NumInstances, 0, 0);
	}

	inline ErrorCode RGrpCommandList::Close()
	{
		IsRecording = false;
		return CommandList->Close();
	}

	inline void RGrpCommandList::Dispatch
	(
		const UINT ThreadGroupCountX,
		const UINT ThreadGroupCountY,
		const UINT ThreadGroupCountZ
	)	const
	{
		CommandList->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
	}
}