#pragma once

#include "TerrainStructure.h"
#include "Resource/Texture/TextureManager.h"
#include "Pipeline/PSOTerrain.h"
#include "Command/CommandList.h"

namespace D3D
{
	class CScene;
	class CSceneRenderer;
	class CSceneLight;

	struct ViewFrustum;

	namespace Terrain
	{
		struct TerrainProperties
		{
			WString Name;
			WString TextureSet;
			WString TextureMap;
			WString HeightMap;
			WString ColorMap;
			WString NormalMap;

			EHeightMapDataType HeightDataType;

			IntPoint	TerrainSize;
			IntPoint	TerrainSizeScaled;
			Int			TerrainID;
			Int			ScaleFactor;
			Int			ScaleFactorHeight;
		};

		class CTerrain
		{
		public:

			class CRenderer
			{
				friend class CTerrain;

			private:

				using IndirectCommand = Pipelines::Terrain::PipelineComputeOcclusion::IndirectCommand;

			private:

				// Shortcuts

				ConstPointer<CScene>					Scene;
				ConstPointer<CSceneRenderer>			SceneRenderer;

				ConstPointer<CTerrain>				Terrain;
				ConstPointer<CStructure>				TerrainStructure;

				ConstPointer<RRenderTargetView>		RenderTarget;
				ConstPointer<RRenderTargetView>		RenderTargetNormals;
				ConstPointer<RDepthStencilView>		DepthStencil;
				ConstPointer<RDepthStencilView>		ShadowMap;

				ConstPointer<ViewFrustum>			ViewFrustum;

				ConstPointer<CCommandListContext>	CmdListCtx;
				ConstPointer<CCommandListContext>	CmdListCtxShadows;

				TVector<ConstPointer<CTerrainTree> >	LastRenderedPatches;

				UniquePointer<CGrpCommandBufferPair> AsyncDispatchCommandBuffer;
				TVector<IndirectCommand>			 AsyncDispatchCommands;

			private:

				void Render
				(
					const QueryResult * pQuery
				);

				void RenderTree
				(
					const CTerrainTree * pTree
				);

			public:

				CRenderer
				(
					const CSceneRenderer * pSceneRenderer
				);

				ErrorCode Create();

			public:

				void Update();
				void RenderGeometry();
				void RenderShadowMap();
				void RenderOcclusionMap();
			};

		protected:

			UniquePointer<CRenderer>			TerrainRenderer;
			UniquePointer<CStructure>			TerrainStructure;
			UniquePointer<CHeight>				TerrainHeight;
			UniquePointer<CTextureMap>			TerrainTextureMap;

			UniquePointer<RDescriptorHeap>		ViewDescriptorHeap;

			DescriptorHeapRange					ViewDescriptorHeapRange;
			DescriptorHeapRange					ViewDescriptorHeapRangeSceneCBV;
			DescriptorHeapRange					ViewDescriptorHeapRangeSceneCBVShadows;
			DescriptorHeapRange					ViewDescriptorHeapRangeShadows;
			DescriptorHeapRange					ViewDescriptorHeapRangeCompute;
			DescriptorHeapRange					ViewDescriptorHeapRangeOcclusion;

			ShaderTextureInfo					TexInfoNormalMap;
			ShaderTextureInfo					TexInfoColorMap;
			ShaderTextureInfo					TexInfoHeightMap;
			ShaderTextureInfo					TexInfoRainDrop;

		private:

			TerrainProperties Properties;

		private:

			ErrorCode CreateDescriptorHeap
			(
				CCommandListContext * CmdListCtx, 
				CCommandListContext * CmdListCtxShadows,
				CCommandListContext * CmdListCtxCompute
			);
			ErrorCode CreateRainDropNormals
			(
				CCommandListContext * CmdListCtx
			);
			ErrorCode CreateHeightMap
			(
				CCommandListContext * CmdListCtx
			);
			ErrorCode CreateTextureMap
			(
				CCommandListContext * CmdListCtx
			);
			ErrorCode CreateColorMap
			(
				CCommandListContext * CmdListCtx,
				CTextureManager::CTextureInitializationStream * InitializationStream
			);
			ErrorCode CreateNormalMap
			(
				CCommandListContext * CmdListCtx,
				CTextureManager::CTextureInitializationStream * InitializationStream
			);

			bool IntersectsRay
			(
				const Vector3f& IV, 
				const Vector3f& RayOrigin, 
				const Vector3f& RayDirection
			);

		public:

			inline const TerrainProperties & GetProperties() const
			{
				return Properties;
			}

			inline const CHeight * GetHeightInfo() const
			{
				return TerrainHeight.Get();
			}

		public:

			ErrorCode Create
			(
						CCommandListContext	* CmdListCtx,
						CCommandListContext	* CmdListCtxShadows,
						CCommandListContext	* CmdListCtxCompute,
				const	TerrainProperties	& Properties
			);

			void ComputeLightmap
			(
				const CSceneLight * pSceneLight
			);
		};
	}
}