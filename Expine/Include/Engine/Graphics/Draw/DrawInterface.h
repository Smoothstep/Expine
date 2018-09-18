#pragma once

#include "Raw/RawCommandList.h"
#include "Raw/RawCommandAllocator.h"
#include "Raw/RawCommandSignature.h"
#include "Raw/RawRenderTarget.h"

#include "Utils/State/StateBlend.h"
#include "Utils/State/StateRasterizer.h"

#include "Buffer/BufferConstant.h"
#include "Draw/Geometry.h"

#include "Utils/VertexTypes.h"

namespace D3D
{
	class CSceneView;

	struct IDrawObject
	{
		virtual void Draw
		(
			const CSceneView			&	View,
			const CCommandListContext	&	CmdList
		)	const = 0;

		virtual inline ~IDrawObject() = 0;
	};

	inline IDrawObject::~IDrawObject() {}

	class CGeometryLine : public IDrawObject, public CIGeometryBuffer<SimpleColorVertex, Uint16>
	{
	private:

		D3D_PRIMITIVE_TOPOLOGY PrimitiveTopology;

	public:

		inline D3D_PRIMITIVE_TOPOLOGY GetPrimitiveTopology()
		{
			return PrimitiveTopology;
		}

	public:

		inline CGeometryLine
		(
			const D3D_PRIMITIVE_TOPOLOGY PrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP
		)
		{
			this->PrimitiveTopology = PrimitiveTopology;
		}

		virtual void Draw
		(
			const CSceneView			&	View,
			const CCommandListContext	&	CmdListCtx
		)	const;
	};

	class CGeometryTriangle : public IDrawObject, public CIGeometryBuffer<SimpleColorVertex, Uint16>
	{
	private:

		D3D_PRIMITIVE_TOPOLOGY PrimitiveTopology;

	public:

		inline D3D_PRIMITIVE_TOPOLOGY GetPrimitiveTopology()
		{
			return PrimitiveTopology;
		}

	public:

		inline CGeometryTriangle
		(
			const D3D_PRIMITIVE_TOPOLOGY PrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP
		)
		{
			this->PrimitiveTopology = PrimitiveTopology;
		}

		virtual void Draw
		(
			const CSceneView			&	View,
			const CCommandListContext	&	CmdListCtx
		)	const;

		void Add
		(
			const SimpleColorVertex	& _0,
			const SimpleColorVertex	& _1,
			const SimpleColorVertex	& _2
		);

		void Add
		(
			SimpleColorVertex	* Vertices,
			Uint16				* Indices,
			size_t				NumVertices,
			size_t				NumIndices
		);
	};

	class CDrawInterface
	{
	protected:

		ConstPointer<CCommandListContext> CmdListCtx;

	protected:

		ConstPointer<RRenderTargetView> RenderTarget;

	private:

		TMap<PipelineObjectBase*, TVector<SharedPointer<IDrawObject> > > LineGeometry;
		TMap<PipelineObjectBase*, TVector<SharedPointer<IDrawObject> > > DynamicGeometry;
		TMap<PipelineObjectBase*, TVector<SharedPointer<IDrawObject> > > StaticGeometry;

	private:

		UniquePointer<CConstantBuffer> ConstantBuffer;
		UniquePointer<RDescriptorHeap> ConstantBufferDescriptorHeap;

		struct TransformData
		{
			Matrix4x4 WorldViewProjection;
		};

	public:

		inline void SetRenderTarget
		(
			const RRenderTargetView * pRenderTargetView
		)
		{
			RenderTarget = pRenderTargetView;
		}

		inline RRenderTargetView * GetRenderTarget() const
		{
			return RenderTarget.Get();
		}

	private:

		ErrorCode CreateConstantBuffer();

		void UploadConstantData
		(
			const CSceneView & View
		)	const;

	public:

		CDrawInterface
		(
			const CCommandListContext * pCmdListCtx
		);

		CDrawInterface();

		ErrorCode Create
		(
			const SharedPointer<RCommandAllocator> & Allocator
		);

		ErrorCode Create();

	public:

		void DrawGeometry
		(
			const CSceneView & View
		);

	public:

		inline void AddDynamicGeometry
		(
			IDrawObject			* DrawObject,
			PipelineObjectBase	* PipelineBase
		)
		{
			auto Iter = DynamicGeometry.find(PipelineBase);

			if (Iter != DynamicGeometry.end())
			{
				Iter->second.push_back(DrawObject);
			}
			else
			{
				TVector<SharedPointer<IDrawObject> > DrawObjects =
				{
					DrawObject
				};

				DynamicGeometry.insert(std::make_pair(PipelineBase, DrawObjects));
			}
		}

		inline void AddStaticGeometry
		(
			IDrawObject			* DrawObject,
			PipelineObjectBase	* PipelineBase
		)
		{
			auto Iter = StaticGeometry.find(PipelineBase);

			if (Iter != StaticGeometry.end())
			{
				Iter->second.push_back(DrawObject);
			}
			else
			{
				TVector<SharedPointer<IDrawObject> > DrawObjects =
				{
					DrawObject
				};

				StaticGeometry.insert(std::make_pair(PipelineBase, DrawObjects));
			}
		}

		inline void Flush()
		{
			DynamicGeometry.clear();
		}
	};
}