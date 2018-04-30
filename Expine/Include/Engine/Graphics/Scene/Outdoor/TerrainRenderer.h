#pragma once

#include "TerrainStructure.h"

namespace D3D
{
	namespace Terrain
	{
		class CRenderer
		{
		private:

			ConstPointer<CStructure>	TerrainStructure;
			ConstPointer<RGrpCommandList> GrpCmdList;

		private:

			ConstPointer<ViewFrustum> ViewFrustum;

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
				const CStructure		* pStructure,
				const RGrpCommandList	* pCommandList
			);

			void Render
			(
				const RRenderTargetView		& RenderTarget,
				const RDepthStencilView		& DepthStencil,
				const RDescriptorHeap		& DescriptorHeap,
				const CSceneView			& View
			);

			void RenderShadowMap
			(
				const RDepthStencilView		& ShadowMap,
				const RDescriptorHeap		& DescriptorHeap,
				const CSceneView			& View
			);

			ErrorCode Initialize
			(
				const WString & TextureSet,
				const WString & TextureMap
			);
		};
	}
}