#include "Precompiled.h"

#include "Pipeline/PSOComposite.h"

#include "Scene/Scene.h"
#include "Scene/SceneRenderer.h"
#include "Scene/SceneComposite.h"

#include "Utils/State/StateBlend.h"
#include "Utils/State/StateRasterizer.h"
#include "Utils/State/StateSampler.h"

#include "Resource/Texture/TextureManager.h"

namespace D3D
{
	CSceneComposite::CRenderer::CRenderer(const CSceneRenderer * pSceneRenderer)
	{
		Scene			= pSceneRenderer->GetScene();
		SceneRenderer	= pSceneRenderer;

		CmdListContext	= SceneRenderer->GetCompositeCommandContext();
	}
	
	ErrorCode CSceneComposite::CRenderer::Create()
	{
		try
		{
			ViewDescriptorHeapRange = CmdListContext->OccupyViewDescriptorRange(Pipelines::PipelineComposite::NumDescriptors);
		}
		catch (ECException & Exception)
		{
			return Exception.GetErrorCode();
		}

		ErrorCode Error;

		if ((Error = Textures.LoadTextures(CmdListContext.GetRef())))
		{
			return Error;
		}

		Textures.NoiseShaderTextureInfo.Resource->AsPixelShaderResource(CmdListContext.GetRef());
		Textures.LightingGGXTextureInfo.Resource->AsPixelShaderResource(CmdListContext.GetRef());

		DescriptorHeapEntry Entries[Pipelines::PipelineComposite::NumDescriptors];
		{
			Entries[Pipelines::PipelineComposite::Noise]		= Textures.NoiseShaderTextureInfo.HeapEntry;
			Entries[Pipelines::PipelineComposite::LightingGGX]	= Textures.LightingGGXTextureInfo.HeapEntry;
			Entries[Pipelines::PipelineComposite::BloomOverlay] = Textures.OverlayTextureInfo.HeapEntry;
		}

		ViewDescriptorHeapRange.DescriptorHeap->CopyDescriptorHeapEntries(ViewDescriptorHeapRange, Entries);

		return S_OK;
	}

	void CSceneComposite::CRenderer::Render()
	{
		const RGrpCommandList & CmdList = CmdListContext.GetRef();

		RenderTarget = Scene->GetCompositeRTV().Get();

		Pipelines::PipelineComposite::Instance().Apply(CmdListContext);
		{
			CmdListContext->SetGraphicsRootDescriptorTable(Pipelines::PipelineComposite::SceneConstants, DIDSceneCBV);
			CmdListContext->SetGraphicsRootDescriptorTable(Pipelines::PipelineComposite::SceneResources, DIDSceneSRV);
			CmdListContext->SetGraphicsRootDescriptorTable(Pipelines::PipelineComposite::SceneResources, ViewDescriptorHeapRange);
		}

		CmdList.SetViewport(Scene->GetOutputScreen()->GetViewport());
		CmdList.SetScissorRect(Scene->GetOutputScreen()->GetViewport().ScissorRect);

		CmdList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		
		RenderTarget->SetAsRenderTarget(CmdList);
		RenderTarget->Clear(CmdList);

		CmdList.DrawInstanced(1, 3);
	}

	inline Int32 RandomSeed(Int32 Seed)
	{
		Seed ^= Seed << 13;
		Seed ^= Seed >> 17;
		Seed ^= Seed << 5;
		return Seed;
	}

	Byte Random(Int32 x, Int32 y, Int32 z)
	{
		Uint Seed = (
			RandomSeed(x) + 
			RandomSeed(y * 19)) * 
			RandomSeed(z * 23) - z;

		return RandomSeed(Seed) % 256;
	}

	ErrorCode CSceneComposite::CRenderer::STextures::GenerateNoiseTexture(const CCommandListContext & CmdListCtx)
	{
		static constexpr size_t NoiseTextureSize = 128;

		RGBAColor * ResourceMemory = new RGBAColor[NoiseTextureSize * NoiseTextureSize];
		
		for (int32_t Y = 0; Y < NoiseTextureSize; ++Y)
		{
			for (int32_t X = 0; X < NoiseTextureSize; ++X)
			{
				ResourceMemory[X + Y * NoiseTextureSize].R = static_cast<float>(Random(X, Y, 1)) / 256.0;
				ResourceMemory[X + Y * NoiseTextureSize].B = Math::GenerateNoise2(Vector3f(X, Y, NoiseTextureSize * Math::RandomNoise()));
			}
		}

		for (int32_t Y = 0; Y < NoiseTextureSize; ++Y)
		{
			for (int32_t X = 0; X < NoiseTextureSize; ++X)
			{
				int32_t X2 = (X - 37) & (NoiseTextureSize - 1);
				int32_t Y2 = (Y - 17) & (NoiseTextureSize - 1);
				ResourceMemory[X + Y * NoiseTextureSize].G = ResourceMemory[X2 + Y2 * NoiseTextureSize].R;
				ResourceMemory[X + Y * NoiseTextureSize].A = ResourceMemory[X2 + Y2 * NoiseTextureSize].B;
			}
		}

		D3D12_SUBRESOURCE_DATA SubResourceData;
		{
			SubResourceData.pData		= ResourceMemory;
			SubResourceData.RowPitch	= NoiseTextureSize * sizeof(RGBAColor);
			SubResourceData.SlicePitch	= NoiseTextureSize * sizeof(RGBAColor);
		}

		return CTextureManager::Instance().CreateTexture(
			L"NoiseTexture", 
			L"Vibe_NoiseTexture", 
			RResource::InitializeOptions(
				NoiseTextureSize,
				NoiseTextureSize,
				DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0,
				D3D12_RESOURCE_DIMENSION_TEXTURE2D, 1,
				D3D12_RESOURCE_STATE_COPY_DEST,
				D3D12_RESOURCE_FLAG_NONE, 1),
			0, 1, &SubResourceData, 
			NoiseShaderTextureInfo);
	}

	inline Vector2f LightingFuncGGX_FV(float DotLH, float Roughness)
	{
		float Alpha = Roughness * Roughness;

		float K = Alpha / 2.0f;
		float K2 = K * K;
		float K2Inv = 1.0f - K2;
		float Visibillity = Math::Rcp(DotLH * DotLH * K2Inv + K2);
		
		return Vector2f(1.0f, Math::Pow(1.0f - DotLH, 5.0f)) * Visibillity;
	}

	inline float LightingFuncGGX_D(float DotNH, float Roughness)
	{
		float Alpha = Roughness * Roughness;
		float AlphaSquared = Alpha * Alpha;
		float Denominator = DotNH * DotNH * (AlphaSquared - 1.0f) + 1.0f;

		return AlphaSquared / (PI * Denominator * Denominator);
	}

	ErrorCode CSceneComposite::CRenderer::STextures::GenerateGGX(const CCommandListContext & CmdListCtx)
	{
		static constexpr size_t TextureHeight = 128;
		static constexpr size_t TextureWidth  = 512;

		RGBColor * TextureMemory = new RGBColor[TextureHeight * TextureWidth];

		int32_t O = 0;

		for (int32_t Y = 0; Y < TextureHeight; ++Y)
		{
			for (int32_t X = 0; X < TextureWidth; ++X)
			{
				float DotLH		= static_cast<float>(X) / TextureWidth;
				float Roughness = static_cast<float>(Y) / TextureHeight;

				Vector2f FV = LightingFuncGGX_FV(DotLH, Roughness);

				TextureMemory[O].R = LightingFuncGGX_D(DotLH, Roughness);
				TextureMemory[O].G = FV.X;
				TextureMemory[O].B = FV.Y;

				O++;
			}
		}

		D3D12_SUBRESOURCE_DATA SubResourceData;
		{
			SubResourceData.pData		= TextureMemory;
			SubResourceData.RowPitch	= TextureWidth  * sizeof(RGBColor);
			SubResourceData.SlicePitch	= TextureHeight * sizeof(RGBColor);
		}

		return CTextureManager::Instance().CreateTexture(
			L"GGXLightingTexture",
			L"Vibe_GGXLightingTexture",
			RResource::InitializeOptions(
				TextureWidth,
				TextureHeight,
				DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, 
				D3D12_RESOURCE_DIMENSION_TEXTURE2D, 1, 
				D3D12_RESOURCE_STATE_COPY_DEST, 
				D3D12_RESOURCE_FLAG_NONE, 1),
			0, 1, &SubResourceData,
			LightingGGXTextureInfo,
			&InitializationStream);
	}

	ErrorCode CSceneComposite::CRenderer::STextures::LoadTextures(const CCommandListContext & CmdListCtx)
	{
		ErrorCode Error;
		
		if ((Error = GenerateNoiseTexture(CmdListCtx)))
		{
			return Error;
		}
		
		if ((Error = GenerateGGX(CmdListCtx)))
		{
			return Error;
		}

		if ((Error = CTextureManager::Instance().GetOrAddTexture(L"BloomOverlay.dds", L"BloomOverlay", OverlayTextureInfo, &InitializationStream)))
		{
			return Error;
		}

		InitializationStream.Dispatch();

		return S_OK;
	}
}
