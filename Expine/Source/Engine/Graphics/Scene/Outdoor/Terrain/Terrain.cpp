#include "Precompiled.h"

#include "Scene/Outdoor/Terrain.h"
#include "Scene/Outdoor/TerrainRenderer.h"
#include "Scene/Outdoor/TerrainStructure.h"
#include "Scene/Outdoor/TerrainQuadTree.h"
#include "Resource/Texture/TextureManager.h"
#include "Scene/SceneRenderer.h"
#include "Scene/SceneLight.h"
#include "Scene/SceneView.h"
#include "Pipeline/PSOTerrain.h"

#include <future>

namespace D3D
{
	namespace Terrain
	{
		static const wchar_t * NormalMapPath = L"Vibe_TerrainNormalMap";
		static const wchar_t * HeightMapPath = L"Vibe_TerrainHeightMap";

		CTerrain::CRenderer::CRenderer(const CSceneRenderer * pSceneRenderer)
		{
			SceneRenderer		= pSceneRenderer;				Ensure(SceneRenderer);
			Scene				= pSceneRenderer->GetScene();	Ensure(Scene);

			ViewFrustum			= &Scene->GetViewForInput().GetViewFrustum();

			Update();
		}

		ErrorCode CTerrain::CRenderer::Create()
		{
			ErrorCode Error;

			constexpr int32_t MaxDispatchesX = 100;
			constexpr int32_t MaxDispatchesY = 100;

			const IntPoint& TerrainSize = Terrain->GetProperties().TerrainSize;

			const int32_t DispatchesNeededY = Math::RoundToInt(static_cast<float>(TerrainSize.Y) / MaxDispatchesY);
			const int32_t DispatchesNeededX = Math::RoundToInt(static_cast<float>(TerrainSize.X) / MaxDispatchesX);

			AsyncDispatchCommands.ResizeUninitialized(DispatchesNeededY * DispatchesNeededX);

			int32_t O = 0;

			for (int32_t Y = 0; Y < DispatchesNeededY; ++Y)
			{
				for (int32_t X = 0; X < DispatchesNeededX; ++X)
				{
					AsyncDispatchCommands[O].DispatchArguments.ThreadGroupCountX = 1;
					AsyncDispatchCommands[O].DispatchArguments.ThreadGroupCountY = 1;
					AsyncDispatchCommands[O].DispatchArguments.ThreadGroupCountZ = 1;

					const int32_t StartX = Math::Min(MaxDispatchesX * X, TerrainSize.X);
					const int32_t StartY = Math::Min(MaxDispatchesY * Y, TerrainSize.Y);

					AsyncDispatchCommands[O].TerrainPatch[0] = IntPoint(StartX, StartY);

					const int32_t EndX = Math::Min(StartX + MaxDispatchesX, TerrainSize.X);
					const int32_t EndY = Math::Min(StartY + MaxDispatchesY, TerrainSize.Y);

					AsyncDispatchCommands[O].TerrainPatch[1] = IntPoint(EndX, EndY);

					O++;
				}
			}

			AsyncDispatchCommandBuffer = new CGrpCommandBufferPair(new GrpCommandBufferDescriptor(sizeof(IndirectCommand), AsyncDispatchCommands.size()));

			if ((Error = AsyncDispatchCommandBuffer->Create()))
			{
				return Error;
			}

			CCommandListContext * CmdListCtx = SceneRenderer->GetComputeCommandContext();

			if (!Terrain->ViewDescriptorHeapRangeOcclusion.Valid())
			{
				Terrain->ViewDescriptorHeapRangeOcclusion = CmdListCtx->OccupyViewDescriptorRange(1);
				Terrain->ViewDescriptorHeapRangeOcclusion.DescriptorHeap->CopyDescriptorHeapEntries(Terrain->ViewDescriptorHeapRangeOcclusion, Scene->GetDescriptorHeapUAV());
			}

			if ((Error = CmdListCtx->ResetCommandList(0)))
			{
				return Error;
			}

			AsyncDispatchCommandBuffer->UploadData
			(
				AsyncDispatchCommands.data(),
				AsyncDispatchCommands.size(),
				*CmdListCtx
			);

			if ((Error = CmdListCtx->Close()))
			{
				return Error;
			}

			CmdListCtx->Finish(0);

			return S_OK;
		}

		void CTerrain::CRenderer::Update()
		{
			IntPoint TerrainSizeBefore;

			if (Terrain)
			{
				TerrainSizeBefore = Terrain->GetProperties().TerrainSizeScaled;
			}

			const CSceneOutdoor * Outdoor = dynamic_cast<const CSceneOutdoor*>(Scene->GetArea());

			// Outdoor is needed for terrain.
			Ensure(Outdoor);

			Terrain				= Outdoor->GetTerrain();						Ensure(Terrain);
			TerrainStructure	= Outdoor->GetTerrain()->TerrainStructure;		Ensure(TerrainStructure);

			RenderTarget		= Scene->GetColorRTV();							Ensure(RenderTarget);
			RenderTargetNormals	= Scene->GetNormalRTV();						Ensure(RenderTargetNormals);

			DepthStencil		= Scene->GetDepthDSV();							Ensure(DepthStencil);
			ShadowMap			= Scene->GetShadowDSV();						Ensure(ShadowMap);

			CmdListCtx			= SceneRenderer->GetGeometryCommandContext();	Ensure(CmdListCtx);
			CmdListCtxShadows	= SceneRenderer->GetShadowMapCommandContext();	Ensure(CmdListCtxShadows);

			if (TerrainSizeBefore != Terrain->GetProperties().TerrainSizeScaled)
			{
				ThrowOnError(Create());
			}
		}

		void CTerrain::CRenderer::RenderGeometry()
		{
			const RGrpCommandList & CmdList = CmdListCtx.GetRef();

			LastRenderedPatches.clear();
			LastRenderedPatches.reserve(TerrainStructure->GetPatchCount().Product());

			TerrainStructure->GetMesh().Buffers->ApplyBuffers(CmdList);

			Pipelines::Terrain::PipelineGeometry::Instance().Apply(CmdListCtx);
			{
				CmdListCtx->SetGraphicsRootDescriptorTable(0, Terrain->ViewDescriptorHeapRangeSceneCBV);
				CmdListCtx->SetGraphicsRootDescriptorTable(1, Terrain->ViewDescriptorHeapRange);

				CmdList->SetGraphicsRoot32BitConstant(2, Scene->GetProperties().FrameCount, 0);

				const RRenderTargetView * RenderTargets[] =
				{
					RenderTarget.Get(),
					RenderTargetNormals.Get()
				};

				CmdList.SetRenderTargets<_countof(RenderTargets)>(RenderTargets, DepthStencil.GetRef());
				CmdList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
				
				RenderTree(TerrainStructure->GetRootTree());
			}
		}

		void RenderAllTrees(const RGrpCommandList & CmdList, Terrain::CStructure * TerrainStructure, CTerrainTree * TerrainTree)
		{
			for (int i = 0; i < 4; ++i)
			{
				auto SubTree = TerrainTree->GetSubTree(i);

				if (SubTree)
				{
					RenderAllTrees(CmdList, TerrainStructure, SubTree);
				}

				// Has sub nodes.
				if (!TerrainTree->GetPatch())
				{
					continue;
				}

				CmdList.DrawIndexedInstanced
				(
					1,
					4 * TerrainTree->GetSize().Product(),
					4 * TerrainTree->GetIndexLocation()
				);
			}
		}

		void CTerrain::CRenderer::RenderShadowMap()
		{
			const RGrpCommandList & CmdList = CmdListCtxShadows.GetRef();

			TerrainStructure->GetMesh().Buffers->ApplyBuffers(CmdList);

			Pipelines::Terrain::PipelineShadows::Instance().Apply(CmdListCtxShadows);
			{
				CmdListCtxShadows->SetGraphicsRootDescriptorTable(0, Terrain->ViewDescriptorHeapRangeSceneCBVShadows);
				CmdListCtxShadows->SetGraphicsRootDescriptorTable(1, Terrain->ViewDescriptorHeapRangeShadows);

				CmdList.SetRenderTarget(ShadowMap.GetRef());
				CmdList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

				CmdList->OMSetStencilRef(0xFF);

				RenderAllTrees
				(
					CmdList, 
					TerrainStructure, 
					TerrainStructure->GetRootTree()
				);
			}
		}

		void CTerrain::CRenderer::Render(const QueryResult * pQuery)
		{
			const RGrpCommandList & CmdList = CmdListCtx.GetRef();

			if (pQuery->Tree)
			{
				CTerrainPatch * Patch = pQuery->Tree->GetPatch();

				if (Patch)
				{
					UINT Index = pQuery->Tree->GetIndex();

					CmdList.DrawIndexedInstanced
					(
						1,
						4 * pQuery->Tree->GetSize().Product(),
						4 * pQuery->Tree->GetIndexLocation()
					);

					LastRenderedPatches.push_back(pQuery->Tree);
				}
			}

#pragma unroll(1)
			for (UINT N = 0; N < 4; ++N)
			{
				if (pQuery->SubQuery[N])
				{
					Render(pQuery->SubQuery[N]);
				}
			}
		}

		void CTerrain::CRenderer::RenderTree(const CTerrainTree * pTree)
		{
			if (!pTree)
			{
				return;
			}

			const RGrpCommandList & CmdList = CmdListCtx.GetRef();

			if (!ViewFrustum->Intersects(
				Vector2f(pTree->GetPosition()) * TerrainStructure->GetTileSize(), 
				Vector2f(pTree->GetPosition()  * TerrainStructure->GetTileSize() + pTree->GetSize() * TerrainStructure->GetTileSize())))
			{
				return;
			}
			
			if (!pTree->HasChildren())
			{
				if (pTree->GetPatch())
				{
					UINT Index = pTree->GetIndex();
					
					CmdList.DrawIndexedInstanced
					(
						1,
						4 * pTree->GetSize().Product(),
						4 * pTree->GetIndexLocation()
					);

					LastRenderedPatches.push_back(pTree);
				}
			}

			RenderTree(pTree->GetSubTree(0));
			RenderTree(pTree->GetSubTree(1));
			RenderTree(pTree->GetSubTree(2));
			RenderTree(pTree->GetSubTree(3));
		}

		void CTerrain::CRenderer::RenderOcclusionMap()
		{
			ErrorCode Error;

			if (AsyncDispatchCommands.empty())
			{
				return;
			}

			const CSceneLight	  & SceneLight		= Scene->GetLight();
			const CSceneOcclusion & OcclusionMap	= Scene->GetOcclusionMap();

			CCommandListContext * CmdListCtx = SceneRenderer->GetComputeCommandContext();

			const IntPoint& TerrainSize = Terrain->GetProperties().TerrainSizeScaled;

			Pipelines::Terrain::PipelineComputeOcclusion::Instance().Apply(CmdListCtx);
			{
				CmdListCtx->SetConstantBufferCompute
				(
					Pipelines::Terrain::PipelineComputeOcclusion::LightCBV, Scene->GetConstantBuffer()->GetBufferData().GetGPUVirtualAddress(CBV_LIGHT)
				);

				CmdListCtx->SetComputeRootDescriptorTable
				(
					Pipelines::Terrain::PipelineComputeOcclusion::TerrainSRV, Terrain->ViewDescriptorHeapRangeCompute
				);

				CmdListCtx->SetComputeRootDescriptorTable
				(
					Pipelines::Terrain::PipelineComputeOcclusion::OcclusionMapUAV, Terrain->ViewDescriptorHeapRangeOcclusion
				);

				CmdListCtx->SetComputeRoot32BitConstants
				(
					Pipelines::Terrain::PipelineComputeOcclusion::SizeConstant, 2, &TerrainSize
				);
			}

			CmdListCtx->ExecuteIndirect
			(
				AsyncDispatchCommands.size(),
				AsyncDispatchCommandBuffer->GetBuffer()
			);
		}

		ErrorCode CTerrain::Create(
			CCommandListContext * CmdListCtx, 
			CCommandListContext * CmdListCtxShadows, 
			CCommandListContext * CmdListCtxCompute, 
			const TerrainProperties & TerrainProperties)
		{
			Properties = TerrainProperties;

			ErrorCode Error;

			TerrainStructure = new CStructure();

			static constexpr int PatchSizeX = 16;
			static constexpr int PatchSizeY = 16;

			TerrainHeight = new CHeight(Properties.TerrainSize * Properties.ScaleFactor * Properties.ScaleFactorHeight);

			if ((Error = TerrainHeight->LoadRAWHeightmap(Properties.HeightDataType, Properties.HeightMap)))
			{
				return Error;
			}

			TerrainStructure->SetHeightMap(TerrainHeight.Get());

			if ((Error = CreateDescriptorHeap(CmdListCtx, CmdListCtxShadows, CmdListCtxCompute)))
			{
				return Error;
			}

			if ((Error = CreateTextureMap(CmdListCtx)))
			{
				return Error;
			}

			TerrainStructure->SetTextureMap(TerrainTextureMap.Get());

			if ((Error = TerrainStructure->Create(*CmdListCtx, CStructure::InitializeParameters(Properties.TerrainSize, IntPoint(PatchSizeX, PatchSizeY), Properties.ScaleFactor))))
			{
				return Error;
			}

			if ((Error = CreateRainDropNormals(CmdListCtx)))
			{
				return Error;
			}

			if ((Error = CreateHeightMap(CmdListCtx)))
			{
				return Error;
			}

			CTextureManager::CTextureInitializationStream InitializationStream;

			if ((Error = CreateColorMap(CmdListCtx, &InitializationStream)))
			{
				return Error;
			}

			if ((Error = CreateNormalMap(CmdListCtx, &InitializationStream)))
			{
				return Error;
			}

			InitializationStream.Dispatch();

			ViewDescriptorHeapRange.DescriptorHeap->CopyDescriptorHeapEntries
			(
				ViewDescriptorHeapRange, ViewDescriptorHeap->AsDescriptorHeapRange()
			);

			ViewDescriptorHeapRangeShadows.DescriptorHeap->CopyDescriptorHeapEntries
			(
				ViewDescriptorHeapRangeShadows, ViewDescriptorHeap->AsDescriptorHeapRange()
			);

			ViewDescriptorHeapRangeCompute.DescriptorHeap->CopyDescriptorHeapEntries
			(
				ViewDescriptorHeapRangeCompute, ViewDescriptorHeap->AsDescriptorHeapRange()
			);

			return S_OK;
		}

		ErrorCode CTerrain::CreateDescriptorHeap(
			CCommandListContext * CmdListCtx, 
			CCommandListContext * CmdListCtxShadows, 
			CCommandListContext * CmdListCtxCompute)
		{
			ErrorCode Error;

			ViewDescriptorHeap = new RDescriptorHeap();

			if ((Error = ViewDescriptorHeap->Create(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, Pipelines::Terrain::NumDescriptors)))
			{
				return Error;
			}

			try
			{
				ViewDescriptorHeapRange					= CmdListCtx->OccupyViewDescriptorRange(Pipelines::Terrain::NumDescriptors);
				ViewDescriptorHeapRangeShadows			= CmdListCtxShadows->OccupyViewDescriptorRange(Pipelines::Terrain::NumDescriptors);
				ViewDescriptorHeapRangeCompute			= CmdListCtxCompute->OccupyViewDescriptorRange(Pipelines::Terrain::NumDescriptors);
				ViewDescriptorHeapRangeSceneCBV			= CmdListCtx->GetDescriptorHeapRange(DIDSceneCBV);
				ViewDescriptorHeapRangeSceneCBVShadows	= CmdListCtxShadows->GetDescriptorHeapRange(DIDSceneCBV);
			}
			catch (ErrorCode EC)
			{
				return EC;
			}

			return S_OK;
		}

		ErrorCode CTerrain::CreateTextureMap(CCommandListContext * CmdListCtx)
		{
			ErrorCode Error;

			TerrainTextureMap = new CTextureMap();

			if ((Error = TerrainTextureMap->ReadTextureSet(Properties.TextureSet)))
			{
				return Error;
			}

			if ((Error = TerrainTextureMap->InitializeResources(
				*CmdListCtx,
				Properties.TerrainSize.X * Properties.ScaleFactor,
				Properties.TerrainSize.Y * Properties.ScaleFactor,
				0,
				&DescriptorHeapEntry(ViewDescriptorHeap.Get(), Pipelines::Terrain::SRV_TextureAtlas),
				&DescriptorHeapEntry(ViewDescriptorHeap.Get(), Pipelines::Terrain::SRV_TextureAtlasNormals),
				&DescriptorHeapEntry(ViewDescriptorHeap.Get(), Pipelines::Terrain::SRV_TextureAtlasParallax))))
			{
				return Error;
			}

			if ((Error = TerrainTextureMap->ReadTextureMap(*CmdListCtx, Properties.TextureMap)))
			{
				return Error;
			}

			return S_OK;
		}

		ErrorCode CTerrain::CreateColorMap(CCommandListContext * CmdListCtx, CTextureManager::CTextureInitializationStream * InitializationStream)
		{
			ErrorCode Error;

			if (Properties.ColorMap.empty())
			{
				return S_OK;
			}

			if ((Error = CTextureManager::Instance().GetOrAddTexture(Properties.ColorMap, L"TerrainColorMap", TexInfoColorMap, InitializationStream)))
			{
				CErrorLog::Log() << "Unable to find Terrain color map: " << Properties.ColorMap << CErrorLog::EndLine;
				return Error;
			}

			RDescriptorHeap::CopyInto<1, 1>
			(
				&DescriptorHeapRange(TexInfoColorMap.HeapEntry), &DescriptorHeapRange(ViewDescriptorHeap.Get(), Pipelines::Terrain::SRV_ColorMap)
			);

			return S_OK;
		}

		ErrorCode CTerrain::CreateNormalMap(CCommandListContext * CmdListCtx, CTextureManager::CTextureInitializationStream * InitializationStream)
		{
			ErrorCode Error;

			if (!TerrainHeight->GetNormals())
			{
				if (Properties.NormalMap.empty())
				{
					return S_OK;
				}

				if ((Error = CTextureManager::Instance().GetOrAddTexture(Properties.NormalMap, L"TerrainNormalMap", TexInfoNormalMap)))
				{
					CErrorLog::Log() << "Unable to find Terrain normal map: " << Properties.NormalMap << CErrorLog::EndLine;
					return Error;
				}

				TexInfoNormalMap.Resource->AsPixelShaderResource(*CmdListCtx);
			}
			else
			{
				D3D12_SUBRESOURCE_DATA SubResourceData;
				{
					SubResourceData.pData		= TerrainHeight->GetNormals();
					SubResourceData.RowPitch	= TerrainHeight->GetSizeX() * sizeof(Vector3f);
					SubResourceData.SlicePitch	= TerrainHeight->GetSizeZ() * sizeof(Vector3f);
				}

				if ((Error = CTextureManager::Instance().CreateTexture(L"Vibe_TerrainNormalMap", L"TerrainNormalMap", RResource::InitializeOptions(
						TerrainHeight->GetSizeX(), 
						TerrainHeight->GetSizeZ(), 
						DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, 
						D3D12_RESOURCE_DIMENSION_TEXTURE2D, 1, 
						D3D12_RESOURCE_STATE_COPY_DEST, 
						D3D12_RESOURCE_FLAG_NONE, 1), 
					0, 1, &SubResourceData, TexInfoNormalMap,
					InitializationStream)))
				{
					return Error;
				}

				TexInfoNormalMap.Resource->AsPixelShaderResource(*CmdListCtx);
			}

			RDescriptorHeap::CopyInto<1, 1>
			(
				&DescriptorHeapRange(TexInfoNormalMap.HeapEntry), &DescriptorHeapRange(ViewDescriptorHeap.Get(), Pipelines::Terrain::SRV_NormalMap)
			);

			return S_OK;
		}

		bool CTerrain::IntersectsRay
		(
			const Vector3f & IV,
			const Vector3f & RayOrigin,
			const Vector3f & RayDirection
		)
		{
			const int32_t W = Properties.TerrainSizeScaled.X;
			const int32_t H = Properties.TerrainSizeScaled.Y;

			Vector3f V = IV + RayDirection;
			Vector3f Dir = Vector3f(RayDirection.X, RayDirection.Y, 0).GetSafeNormal();

			while (!((V.X >= W - 1) || (V.X <= 0) || (V.Y >= H - 1) || (V.Y <= 0)))
			{
				Float DeltaLengthRay = (Vector3f(V.X, V.Y, 0) - Vector3f(RayOrigin.X, RayOrigin.Y, 0)).Size();
				Float DeltaLengthIV = (IV - V).Size();
				Float DeltaHeight = IV.Z + (DeltaLengthIV * RayOrigin.Z) / DeltaLengthRay;

				if (TerrainHeight->GetActualHeight(Math::Floor(V.X), Math::Floor(V.Y)) * 255.0f / 255.0f > DeltaHeight)
				{
					return true;
				}

				V += Dir;
			}

			return false;
		}

		void CTerrain::ComputeLightmap(const CSceneLight * pSceneLight)
		{
		}

		ErrorCode CTerrain::CreateRainDropNormals(CCommandListContext * CmdListCtx)
		{
			ErrorCode Error;

			const ShaderTextureInfo * RainDropNormalTextureInfo = CTextureManager::Instance().GetShaderTextureInfo(L"RainDrop_N.dds");

			if (RainDropNormalTextureInfo)
			{
				TexInfoRainDrop = *RainDropNormalTextureInfo;

				RDescriptorHeap::CopyInto<1, 1>
				(
					&DescriptorHeapRange(TexInfoRainDrop.HeapEntry), &DescriptorHeapRange(ViewDescriptorHeap.Get(), Pipelines::Terrain::SRV_Normal)
				);

			}

			return S_OK;
		}

		ErrorCode CTerrain::CreateHeightMap(CCommandListContext * CmdListCtx)
		{
			ErrorCode Error;

			D3D12_SUBRESOURCE_DATA SubResourceData;
			{
				SubResourceData.pData		= TerrainHeight->GetData();
				SubResourceData.RowPitch	= TerrainHeight->GetSizeX() * sizeof(Float);
				SubResourceData.SlicePitch	= TerrainHeight->GetSizeZ() * sizeof(Float);
			}

			if ((Error = CTextureManager::Instance().CreateTexture(L"Vibe_TerrainHeightMap", L"TerrainHeightMap", RResource::InitializeOptions(
				TerrainHeight->GetSizeX(),
				TerrainHeight->GetSizeZ(),
				DXGI_FORMAT_R32_FLOAT, 1, 0,
				D3D12_RESOURCE_DIMENSION_TEXTURE2D, 1,
				D3D12_RESOURCE_STATE_COPY_DEST,
				D3D12_RESOURCE_FLAG_NONE, 1),
				0, 1, &SubResourceData, TexInfoHeightMap)))
			{
				return Error;
			}

			TexInfoHeightMap.Resource->AsPixelShaderResource(*CmdListCtx);

			RDescriptorHeap::CopyInto<1, 1>
			(
				&DescriptorHeapRange(TexInfoHeightMap.HeapEntry), &DescriptorHeapRange(ViewDescriptorHeap.Get(), Pipelines::Terrain::SRV_Height)
			);

			return S_OK;
		}
	}
}
