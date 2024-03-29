#pragma once

#include "Raw/RawRenderTarget.h"
#include "Raw/RawShaderResourceView.h"
#include "Raw/RawPipelineState.h"

#include "Buffer/BufferQuad.h"

#include "Resource/Texture/TextureManager.h"

namespace D3D
{
	class CScene;
	class CSceneRenderer;
	class CSceneComposite
	{
	public:

		class CRenderer
		{
		private:
			class STextures
			{
			public:

				CTextureManager::CTextureInitializationStream InitializationStream;

				ShaderTextureInfo NoiseShaderTextureInfo;
				ShaderTextureInfo LightingGGXTextureInfo;
				ShaderTextureInfo OverlayTextureInfo;

			public:

				ErrorCode LoadTextures(const CCommandListContext& CmdListCtx);

			private:

				ErrorCode GenerateNoiseTexture(const CCommandListContext& CmdListCtx);
				ErrorCode GenerateGGX(const CCommandListContext& CmdListCtx);
			};

			STextures Textures;

			ConstPointer<RRenderTargetView>		RenderTarget;
			ConstPointer<RRenderTargetView>		RenderTargetNormals;

			ConstPointer<CScene>				Scene;
			ConstPointer<CSceneRenderer>		SceneRenderer;
			ConstPointer<CSceneComposite>		SceneComposite;

			ConstPointer<CCommandListContext>	CmdListContext;

			DescriptorHeapRange					ViewDescriptorHeapRange;

		public:

			ErrorCode Create();

		public:

			CRenderer
			(
				const CSceneRenderer * pScene
			);

		public:

			void Render();
		};
	};
}