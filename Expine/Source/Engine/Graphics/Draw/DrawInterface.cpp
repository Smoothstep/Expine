#include "Precompiled.h"

#include "Draw/DrawInterface.h"
#include "Scene/SceneView.h"
#include "Pipeline/Pipelines.h"

namespace D3D
{
	ErrorCode CDrawInterface::CreateConstantBuffer()
	{
		ErrorCode Error;

		ConstantBuffer = new CConstantBuffer
		(
			new GrpConstantBufferDescriptor(sizeof(TransformData))
		);

		ConstantBufferDescriptorHeap = new RDescriptorHeap();

		if ((Error = ConstantBufferDescriptorHeap->Create_CBV_SRV_UAV()))
		{
			return Error;
		}

		if ((Error = ConstantBuffer->Create(DescriptorHeapRange(ConstantBufferDescriptorHeap.Get()))))
		{
			return Error;
		}

		return S_OK;
	}

	CDrawInterface::CDrawInterface(const CCommandListContext * pCmdListCtx)
	{
		CmdListCtx = pCmdListCtx;
	}

	CDrawInterface::CDrawInterface()
	{
		CmdListCtx = new CCommandListContext();
	}

	ErrorCode CDrawInterface::Create(const SharedPointer<RCommandAllocator>& Allocator)
	{
		ErrorCode Error;

		if ((Error = CmdListCtx->Create(Allocator)))
		{
			return Error;
		}

		if ((Error = CreateConstantBuffer()))
		{
			return Error;
		}

		return S_OK;
	}

	ErrorCode CDrawInterface::Create()
	{
		if (!CmdListCtx)
		{
			return E_FAIL;
		}

		ErrorCode Error;

		if ((Error = CreateConstantBuffer()))
		{
			return Error;
		}

		return S_OK;
	}

	void CDrawInterface::DrawGeometry(const CSceneView & View)
	{
		const RGrpCommandList & CmdList = CmdListCtx.GetRef();

		CmdList->RSSetViewports(1, &View.GetViewport());
		CmdList->RSSetScissorRects(1, &View.GetViewport().ScissorRect);

		bool CustomRenderTarget = false;

		// Rendertarget

		if (CustomRenderTarget)
		{
			RenderTarget->SetAsRenderTarget(CmdList);
			RenderTarget->Clear(CmdList);
		}

		// Constants

		UploadConstantData(View);

		for (const auto MapIter : DynamicGeometry)
		{
			MapIter.first->Apply(CmdListCtx.Get());
			{
				CmdListCtx->SetGraphicsRootDescriptorTable(0, 0);
			}

			for (const auto VectorIter : MapIter.second)
			{
				VectorIter->Draw(View, CmdListCtx.GetRef());
			}
		}

		for (const auto MapIter : StaticGeometry)
		{
			MapIter.first->Apply(CmdListCtx.Get());
			{
				CmdListCtx->SetGraphicsRootDescriptorTable(0, 0);
			}

			CmdListCtx->SetGraphicsRootDescriptorTable(0, 0);

			for (const auto VectorIter : MapIter.second)
			{
				VectorIter->Draw(View, CmdListCtx.GetRef());
			}
		}
	}

	inline void CDrawInterface::UploadConstantData(const CSceneView & View) const
	{
		ConstantBuffer->GetBufferData().MapData(0, 0, 0, &View.GetViewSetup().ViewProjectionMatrix.GetTransposed());
	}

	void CGeometryLine::Draw(const CSceneView & View, const CCommandListContext	& CmdListCtx) const
	{
		CmdListCtx.SetPrimitiveTopology(PrimitiveTopology);

		UploadData(CmdListCtx);
		{
			ApplyBuffers(CmdListCtx);
		}

		for (size_t N = 0; N < IndexData.size(); N += 2)
		{
			CmdListCtx->DrawIndexedInstanced(2, 1, N, 0, 0);
		}
	}

	void CGeometryTriangle::Draw(const CSceneView &	View, const CCommandListContext & CmdListCtx) const
	{
		CmdListCtx.SetPrimitiveTopology(PrimitiveTopology);

		UploadData(CmdListCtx);
		{
			ApplyBuffers(CmdListCtx);
		}

		CmdListCtx->DrawIndexedInstanced(IndexData.size(), 1, 0, 0, 0);
	}

	void CGeometryTriangle::Add(const SimpleColorVertex & _0, const SimpleColorVertex & _1, const SimpleColorVertex & _2)
	{
		IndexData.push_back(IndexData.size());
		IndexData.push_back(IndexData.size());
		IndexData.push_back(IndexData.size());
		VertexData.push_back(_0);
		VertexData.push_back(_1);
		VertexData.push_back(_2);
	}

	void CGeometryTriangle::Add(SimpleColorVertex * Vertices, Uint16 * Indices, size_t NumVertices, size_t NumIndices)
	{
		IndexData.insert(IndexData.end(), 
			Indices, 
			Indices + NumIndices);
		VertexData.insert(VertexData.end(),
			Vertices, 
			Vertices + NumVertices);
	}
}
