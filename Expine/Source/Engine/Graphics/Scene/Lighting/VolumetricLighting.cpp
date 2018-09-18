#include "Precompiled.h"

#include "Scene/Scene.h"
#include "Scene/SceneView.h"
#include "Scene/SceneRenderer.h"
#include "Scene/Lighting/VolumetricLighting.h"

namespace D3D
{
	namespace Light
	{		
		CVolumetricLighting::CVolumetricLighting()
		{
			LUTViewport.Height		= LIGHT_LUT_WDOTV_RESOLUTION;
			LUTViewport.Width		= 1.0f;
			LUTViewport.MaxDepth	= 1.0f;
			LUTViewport.MinDepth	= 0.0f;
			LUTViewport.TopLeftX	= 0.0f;
			LUTViewport.TopLeftY	= 0.0f;

			LUTScissorRect.bottom	= LIGHT_LUT_WDOTV_RESOLUTION;
			LUTScissorRect.right	= 1;
			LUTScissorRect.left		= 0;
			LUTScissorRect.top		= 0;
		}

		CVolumetricLighting::CVolumetricLighting(const CCommandListContext * pCmdListCtx)
		{
			LUTViewport.Height		= LIGHT_LUT_WDOTV_RESOLUTION;
			LUTViewport.Width		= 1.0f;
			LUTViewport.MaxDepth	= 1.0f;
			LUTViewport.MinDepth	= 0.0f;
			LUTViewport.TopLeftX	= 0.0f;
			LUTViewport.TopLeftY	= 0.0f;

			LUTScissorRect.bottom	= LIGHT_LUT_WDOTV_RESOLUTION;
			LUTScissorRect.right	= 1;
			LUTScissorRect.left		= 0;
			LUTScissorRect.top		= 0;

			CmdListCtx = pCmdListCtx;
		}

		ErrorCode CVolumetricLighting::CreateDescriptorHeaps(const CScene * pScene)
		{
			ErrorCode Error;

			DescriptorHeap = new RDescriptorHeap();
			{
				if ((Error = DescriptorHeap->Create_CBV_SRV_UAV(NumShaderResources, D3D12_DESCRIPTOR_HEAP_FLAG_NONE)))
				{
					return Error;
				}
			}

			return S_OK;
		}

		ErrorCode CVolumetricLighting::CreateConstantBuffers()
		{
			ErrorCode Error;

			ConstantBuffer = new CConstantBuffer(GrpConstantBufferDescriptor::Create<256,
				ContextConstants,
				FrameConstants,
				VolumeConstants,
				ApplyConstants>());

			if ((Error = ConstantBuffer->Create(DescriptorHeap->AsDescriptorHeapRange())))
			{
				return Error;
			}

			return S_OK;
		}

		ErrorCode CVolumetricLighting::CreateViews(const CScene * pScene)
		{
			ErrorCode Error;

			SharedPointer<RResource> ResourcesRTV[NumRenderTargets];
			SharedPointer<RResource> ResourceDepth = new RResource();
			{
				if ((Error = ResourceDepth->Create(RResource::InitializeOptions::DepthStencil2D(
					GetInternalBufferSize(pScene->GetView().GetSize().X),
					GetInternalBufferSize(pScene->GetView().GetSize().Y),
					CScene::DepthResourceFormat,
					0.0f, 
					0xFF,
					1,
					D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
					1))))
				{
					return Error;
				}
			}

			ResourceDepth->Get()->SetName(L"Light Depth & Stencil");

			ResourcesRTV[TARGET_DEPTH_RESOLVED] = new RResource();
			{
				if ((Error = ResourcesRTV[TARGET_DEPTH_RESOLVED]->Create(RResource::InitializeOptions::Texture2D(
					GetInternalBufferSize(pScene->GetView().GetSize().X),
					GetInternalBufferSize(pScene->GetView().GetSize().Y),
					DXGI_FORMAT_R16G16_FLOAT,
					1, 
					RGBAColor::Black.ColorArray,
					D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE))))
				{
					return Error;
				}
			}

			ResourcesRTV[TARGET_DEPTH_RESOLVED]->Get()->SetName(L"Light Depth Resolved");

			ResourcesRTV[TARGET_ACCUMULATION] = new RResource();
			{
				if ((Error = ResourcesRTV[TARGET_ACCUMULATION]->Create(RResource::InitializeOptions::Texture2D(
					GetInternalBufferSize(pScene->GetView().GetSize().X),
					GetInternalBufferSize(pScene->GetView().GetSize().Y),
					DXGI_FORMAT_R16G16B16A16_FLOAT,
					1,
					RGBAColor::Transparent.ColorArray,
					D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE))))
				{
					return Error;
				}
			}

			ResourcesRTV[TARGET_ACCUMULATION]->Get()->SetName(L"Light Accumulation");

			ResourcesRTV[TARGET_ACCUMULATION_RESOLVED] = new RResource();
			{
				if ((Error = ResourcesRTV[TARGET_ACCUMULATION_RESOLVED]->Create(RResource::InitializeOptions::Texture2D(
					GetInternalBufferSize(pScene->GetView().GetSize().X),
					GetInternalBufferSize(pScene->GetView().GetSize().Y),
					DXGI_FORMAT_R16G16B16A16_FLOAT,
					1,
					RGBAColor::Black.ColorArray,
					D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE))))
				{
					return Error;
				}
			}

			ResourcesRTV[TARGET_ACCUMULATION_RESOLVED]->Get()->SetName(L"Light Accumulation Resolved");

			for (UINT N = TARGET_LUT; N < TARGET_ACCUMULATION; ++N)
			{
				ResourcesRTV[N] = new RResource();
				{
					if ((Error = ResourcesRTV[N]->Create(RResource::InitializeOptions::Texture2D(
						1,
						LIGHT_LUT_WDOTV_RESOLUTION,
						DXGI_FORMAT_R16G16B16A16_FLOAT,
						1,
						RGBAColor::Transparent.ColorArray,
						D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE))))
					{
						return Error;
					}
				}
			}

			ResourcesRTV[TARGET_LUT]->Get()->SetName(L"Light LUT");
			ResourcesRTV[TARGET_LUT_P_0]->Get()->SetName(L"Light LUT P0");
			ResourcesRTV[TARGET_LUT_P_1]->Get()->SetName(L"Light LUT P1");
			ResourcesRTV[TARGET_LUT_S0_0]->Get()->SetName(L"Light LUT S0_0");
			ResourcesRTV[TARGET_LUT_S0_1]->Get()->SetName(L"Light LUT S0_1");
			ResourcesRTV[TARGET_LUT_S1_0]->Get()->SetName(L"Light LUT S1_0");
			ResourcesRTV[TARGET_LUT_S1_0]->Get()->SetName(L"Light LUT S1_1");
			ResourcesRTV[TARGET_LUT_S2_0]->Get()->SetName(L"Light LUT S2_0");
			ResourcesRTV[TARGET_LUT_S2_1]->Get()->SetName(L"Light LUT S2_1");

			if (FilterMode == FILTER_TEMPORAL)
			{
				for (UINT N = TARGET_ACCUMULATION_FILTERED_0; N < NumRenderTargets; N += 2)
				{
					ResourcesRTV[N + 0] = new RResource();
					{
						if ((Error = ResourcesRTV[N + 0]->Create(RResource::InitializeOptions::Texture2D(
							GetInternalBufferSize(pScene->GetView().GetSize().X),
							GetInternalBufferSize(pScene->GetView().GetSize().Y),
							DXGI_FORMAT_R16G16B16A16_FLOAT,
							1,
							RGBAColor::Black.ColorArray,
							D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE))))
						{
							return Error;
						}
					}

					ResourcesRTV[N + 1] = new RResource();
					{
						if ((Error = ResourcesRTV[N + 1]->Create(RResource::InitializeOptions::Texture2D(
							GetInternalBufferSize(pScene->GetView().GetSize().X),
							GetInternalBufferSize(pScene->GetView().GetSize().Y),
							DXGI_FORMAT_R16G16_FLOAT,
							1,
							RGBAColor::Black.ColorArray,
							D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE))))
						{
							return Error;
						}
					}
				}
			}

			if ((Error = RenderTargets.Initialize(ResourcesRTV)))
			{
				return Error;
			}

			if ((Error = ShaderResources.Initialize(ResourcesRTV, DescriptorHeap, SRV_DEPTH_LOCAL + 1)))
			{
				return Error;
			}

			DescriptorHeapDSV = new RDescriptorHeap();

			if ((Error = DescriptorHeapDSV->Create_DSV(2)))
			{
				return Error;
			}

			DepthStencilSRV = new RShaderResourceView();

			if ((Error = DepthStencilSRV->CreateFromResource(ResourceDepth, DescriptorHeapEntry(DescriptorHeap.Get(), SRV_DEPTH_LOCAL))))
			{
				return Error;
			}

			DepthStencil = new RDepthStencilView();

			if ((Error = DepthStencil->CreateFromResource(ResourceDepth, DescriptorHeapEntry(DescriptorHeapDSV.Get(), 0))))
			{
				return Error;
			}

			DepthStencilReadOnly = new RDepthStencilView();

			if ((Error = DepthStencilReadOnly->CreateFromResource(ResourceDepth, DescriptorHeapEntry(DescriptorHeapDSV.Get(), 1), TRUE)))
			{
				return Error;
			}

			return S_OK;
		}

		void CVolumetricLighting::SetupPerContextConstants
		(
			const CScene * pScene
		)
		{
			ContextConstants * PerContextConstants = reinterpret_cast<ContextConstants *>(ConstantBuffer->GetBufferData().GetCPUVirtualAddress(CBV_PER_CONTEXT));

			const Uint32 Width	= pScene->GetView().GetViewport().Width;
			const Uint32 Height = pScene->GetView().GetViewport().Height;

			PerContextConstants->vOutputSize		= Vector2f(1.0f * Width, 1.0f * Height);
			PerContextConstants->vOutputSize_Inv	= Vector2f(1.0f / Width, 1.0f / Height);

			const Uint32 InternalWidth	= GetInternalBufferSize(Width);
			const Uint32 InternalHeight = GetInternalBufferSize(Height);

			PerContextConstants->vBufferSize		= Vector2f(1.0f * Width, 1.0f * Height);
			PerContextConstants->vBufferSize_Inv	= Vector2f(1.0f / Width, 1.0f / Height);

			PerContextConstants->uSampleCount = GetInternalSampleCount();
			PerContextConstants->fResMultiplier = 1.0f / GetInternalScale();
		}

		void CVolumetricLighting::SetupPerFrameConstants
		(
			const CScene * pScene
		)
		{
			const MediumDesc & Medium = pScene->GetLight().GetMediumDesc();

			FrameConstants * PerFrameConstants = reinterpret_cast<FrameConstants *>(ConstantBuffer->GetBufferData().GetCPUVirtualAddress(CBV_PER_FRAME));

			PerFrameConstants->mProj				= pScene->GetView().GetViewSetup().ProjectionMatrix;
			PerFrameConstants->mViewProj			= pScene->GetView().GetViewSetup().ViewProjectionMatrix;
			PerFrameConstants->mViewProj_Inv		= pScene->GetView().GetViewSetup().ViewProjectionInverseMatrix;
			PerFrameConstants->vEyePosition		= pScene->GetView().GetViewSetup().ViewOrigin;

			PerFrameConstants->fZFar				= pScene->GetProperties().ClipNear;
			PerFrameConstants->fZNear			= pScene->GetProperties().ClipFar;

			JitterIndex							= pScene->GetProperties().JitterIndex;

			PerFrameConstants->vJitterOffset	= GetJitter();

			const Uint32 Width	= pScene->GetView().GetViewport().Width;
			const Uint32 Height = pScene->GetView().GetViewport().Height;

			PerFrameConstants->vOutputViewportSize		= Vector2f(1.0f * Width, 1.0f * Height);
			PerFrameConstants->vOutputViewportSize_Inv	= Vector2f(1.0f / Width, 1.0f / Height);

			const Uint32 InternalWidth	= GetInternalBufferSize(Width);
			const Uint32 InternalHeight = GetInternalBufferSize(Height);

			PerFrameConstants->vViewportSize			= Vector2f(1.0f * InternalWidth, 1.0f * InternalHeight);
			PerFrameConstants->vViewportSize_Inv		= Vector2f(1.0f / InternalWidth, 1.0f /	InternalHeight);

			Vector3f TotalScatter
			(
				0.000001f,
				0.000001f,
				0.000001f
			);

			PerFrameConstants->uNumPhaseTerms = Medium.NumPhaseTerms;

			for (Uint32 T = 0; T < PerFrameConstants->uNumPhaseTerms; ++T)
			{
				PerFrameConstants->uPhaseFunc[T][0] = static_cast<Uint32>(Medium.PhaseTerms[T].ePhaseFunc);

				const Vector3f& Density = Medium.PhaseTerms[T].vDensity;

				PerFrameConstants->vPhaseParams[T] = Vector4f
				(
					Density.X, 
					Density.Y, 
					Density.Z, 
					Medium.PhaseTerms[T].fEccentricity
				);

				TotalScatter += Density;
			}

			PerFrameConstants->vScatterPower.X = 1 - exp(-TotalScatter.X);
			PerFrameConstants->vScatterPower.Y = 1 - exp(-TotalScatter.Y);
			PerFrameConstants->vScatterPower.Z = 1 - exp(-TotalScatter.Z);

			PerFrameConstants->vSigmaExtinction = TotalScatter + Medium.Absorption;
		}

		void CVolumetricLighting::SetupPerVolumeConstants
		(
			const CScene		* pScene,
			const VolumeDesc	& Volume,
			const ShadowMapDesc & ShadowMap
		)
		{
			const LightProperties & Properties = pScene->GetLight().GetProperties();

			VolumeConstants * PerVolumeConstants = reinterpret_cast<VolumeConstants *>(ConstantBuffer->GetBufferData().GetCPUVirtualAddress(CBV_PER_VOLUME));

			PerVolumeConstants->mLightToWorld	= Properties.LightViewProjectionInverseMatrix;

			PerVolumeConstants->vLightIntensity	= Properties.GetLightIntesity();
			PerVolumeConstants->vLightPos		= Properties.Position;

			switch (Properties.Type)
			{
				case LIGHT_DIRECTIONAL:
				{
					PerVolumeConstants->vLightDir = Properties.Directional.Direction;
				}

				break;

				case LIGHT_SPOT:
				{
					PerVolumeConstants->vLightDir				= Properties.Spot.Direction;
					PerVolumeConstants->fLightZNear				= Properties.Spot.ZNear;
					PerVolumeConstants->fLightZFar				= Properties.Spot.ZFar;
					PerVolumeConstants->LightFalloffAngle		= Properties.Spot.FalloffCosTheta;
					PerVolumeConstants->LightFalloffAngle		= Properties.Spot.FalloffPower;
					PerVolumeConstants->vAttenuationFactors[0]	= Properties.Spot.AttenuationFactors[0];
					PerVolumeConstants->vAttenuationFactors[1]	= Properties.Spot.AttenuationFactors[1];
					PerVolumeConstants->vAttenuationFactors[2]	= Properties.Spot.AttenuationFactors[2];
					PerVolumeConstants->vAttenuationFactors[3]	= Properties.Spot.AttenuationFactors[3];
				}

				break;

				case LIGHT_OMNI:
				{
					PerVolumeConstants->vLightPos				= Properties.Position;
					PerVolumeConstants->fLightZNear				= Properties.Omni.ZNear;
					PerVolumeConstants->fLightZFar				= Properties.Omni.ZFar;
					PerVolumeConstants->vAttenuationFactors[0]	= Properties.Omni.AttenuationFactors[0];
					PerVolumeConstants->vAttenuationFactors[1]	= Properties.Omni.AttenuationFactors[1];
					PerVolumeConstants->vAttenuationFactors[2]	= Properties.Omni.AttenuationFactors[2];
					PerVolumeConstants->vAttenuationFactors[3]	= Properties.Omni.AttenuationFactors[3];
				}

				break;
			}

			PerVolumeConstants->fDepthBias		= Volume.DepthBias;
			PerVolumeConstants->uMeshResolution	= Volume.GetCoarseResolution();

			const Vector4f Vw1 = Properties.LightViewProjectionInverseMatrix.TransformVector4f(Vector4f(-1, -1,  1,  1));
			const Vector4f Vw2 = Properties.LightViewProjectionInverseMatrix.TransformVector4f(Vector4f( 1,  1,  1,  1));
			const Vector3f Ray = Vw1.TransformToVector3() - Vw2.TransformToVector3();

			const float fCrossLength = Math::Sqrt(Ray.SizeSquared());
			const float fSideLength  = Math::Sqrt(0.5f * fCrossLength * fCrossLength);

			PerVolumeConstants->fGridSectionSize = fSideLength / static_cast<float>(PerVolumeConstants->uMeshResolution);
			PerVolumeConstants->fTargetRaySize = Volume.TargetRayResolution;

#pragma unroll(1)
			for (UINT N = 0; N < MAX_SHADOWMAP_ELEMENTS; ++N)
			{
				PerVolumeConstants->vElementOffsetAndScale[N].X = static_cast<float>(ShadowMap.Elements[N].uOffsetX) / ShadowMap.uWidth;
				PerVolumeConstants->vElementOffsetAndScale[N].Y = static_cast<float>(ShadowMap.Elements[N].uOffsetY) / ShadowMap.uHeight;
				PerVolumeConstants->vElementOffsetAndScale[N].Z = static_cast<float>(ShadowMap.Elements[N].uWidth)	/ ShadowMap.uWidth;
				PerVolumeConstants->vElementOffsetAndScale[N].W = static_cast<float>(ShadowMap.Elements[N].uHeight)	/ ShadowMap.uHeight;

				PerVolumeConstants->mLightProj[N]		= ShadowMap.Elements[N].mViewProj;
				PerVolumeConstants->mLightProj_Inv[N]	= ShadowMap.Elements[N].mViewProj.GetInverse();

				PerVolumeConstants->uElementIndex[N][0]	= ShadowMap.Elements[N].mArrayIndex;
			}

			PerVolumeConstants->vShadowMapDim.X = static_cast<float>(ShadowMap.uWidth);
			PerVolumeConstants->vShadowMapDim.Y = static_cast<float>(ShadowMap.uHeight);
		}

		void CVolumetricLighting::SetupPerApplyConstants
		(
			const CScene			* pScene,
			const PostprocessDesc	& Postprocess
		)
		{
			ApplyConstants * PerApplyConstants = reinterpret_cast<ApplyConstants *>(ConstantBuffer->GetBufferData().GetCPUVirtualAddress(CBV_PER_APPLY));

			if (FilterMode == FILTER_TEMPORAL)
			{
				PerApplyConstants->fHistoryFactor	= Postprocess.fTemporalFactor;
				PerApplyConstants->fFilterThreshold	= Postprocess.fFilterThreshold;

				if (pScene->GetProperties().IsFirstFrame())
				{
					LastViewProjectionMatrix = Postprocess.mUnjitteredViewProj;
				}
				else
				{
					LastViewProjectionMatrix = NextViewProjectionMatrix;
				}

				NextViewProjectionMatrix = Postprocess.mUnjitteredViewProj;

				PerApplyConstants->mHistoryXform = LastViewProjectionMatrix * Postprocess.mUnjitteredViewProj.GetInverse();
			}
			else
			{
				PerApplyConstants->mHistoryXform		= Matrix4x4::Identity();
				PerApplyConstants->fFilterThreshold	= 0.0f;
				PerApplyConstants->fHistoryFactor	= 0.0f;
			}

			PerApplyConstants->vFogLight			= Postprocess.vFogLight;
			PerApplyConstants->fMultiScattering	= Postprocess.fMultiscatter;
		}

		ErrorCode CVolumetricLighting::Create(const CScene * pScene)
		{
			ErrorCode Error;

			if ((Error = CreateDescriptorHeaps(pScene)))
			{
				return Error;
			}

			if ((Error = CreateConstantBuffers()))
			{
				return Error;
			}

			if ((Error = CreateViews(pScene)))
			{
				return Error;
			}

			SetupPerContextConstants(pScene);

			if (CmdListCtx)
			{
				DescriptorHeapRangeLocal = CmdListCtx->OccupyViewDescriptorRange(DescriptorHeap->GetDescriptorsCount());
				DescriptorHeapRangeLocal.DescriptorHeap->CopyDescriptorHeapEntries(DescriptorHeapRangeLocal, &DescriptorHeap->AsDescriptorHeapRange());
			}

			VolumeSettings.DepthBias			= 0.0f;
			VolumeSettings.TargetRayResolution	= 1.0f;
			VolumeSettings.TessQuality			= TessellationQuality::HIGH;
			
			return S_OK;
		}

		void CVolumetricLighting::Finalize(const CCommandListContext * pCmdListCtx)
		{
			if (DescriptorHeapRangeLocal.Valid())
			{
				CDescriptorHeap * pDescriptorHeap = dynamic_cast<CDescriptorHeap*>(DescriptorHeapRangeLocal.DescriptorHeap.Get());

				if (pDescriptorHeap)
				{
					pDescriptorHeap->UnregisterRange(DescriptorHeapRangeLocal.Offset);
				}
			}

			CmdListCtx = pCmdListCtx;

			Ensure
			(
				CmdListCtx
			);

			DescriptorHeapRangeLocal = CmdListCtx->OccupyViewDescriptorRange(DescriptorHeap->GetDescriptorsCount());
			DescriptorHeapRangeLocal.DescriptorHeap->CopyDescriptorHeapEntries(DescriptorHeapRangeLocal, DescriptorHeap->AsDescriptorHeapRange());

			DescriptorHeapRangeSceneCBV = CmdListCtx->GetDescriptorHeapRange(DIDSceneCBV);
			DescriptorHeapRangeSceneSRV = CmdListCtx->GetDescriptorHeapRange(DIDSceneSRV);
		}

		void CVolumetricLighting::ApplyLighting_Start(const CScene * pScene, const PostprocessDesc & Postprocess)
		{
			const RGrpCommandList & CmdList = CmdListCtx.GetRef();

			SetupPerApplyConstants(pScene, Postprocess);

			CmdList.SetViewport(pScene->GetView().GetViewport());
			CmdList.SetScissorRect(pScene->GetView().GetViewport().ScissorRect);

			RResource * SRVResources[] =
			{
				DepthStencil->GetResource(),
				RenderTargets[TARGET_ACCUMULATION]->GetResource(),
				RenderTargets[TARGET_ACCUMULATION_RESOLVED]->GetResource(),
				RenderTargets[TARGET_DEPTH_RESOLVED]->GetResource()
			};

			static const D3D12_RESOURCE_STATES ResourceStates[] =
			{
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_RENDER_TARGET
			};

			RResource::SetResourceStates<_countof(SRVResources)>(SRVResources, ResourceStates, CmdList, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

			CmdList->OMSetStencilRef(0xFF);
		}

		void CVolumetricLighting::ApplyLighting_Resolve(const CScene * pScene, const PostprocessDesc & Postprocess)
		{
			const RGrpCommandList & CmdList = CmdListCtx.GetRef();

			if (pScene->GetProperties().IsMSAA())
			{
				Pipelines::VolumetricLighting::ResolvePSPipelineMSAA::Instance().Apply(CmdListCtx.Get());
			}
			else
			{
				Pipelines::VolumetricLighting::ResolvePSPipeline::Instance().Apply(CmdListCtx.Get());
			}

			CmdList.SetGraphicsRootDescriptorTable(Pipelines::VolumetricLighting::ResolvePSPipeline::PARAMETER_CBV_SRV_DESCRIPTOR_TABLE, DescriptorHeapRangeLocal);

			CmdList->OMSetStencilRef(0xFF);

			const RRenderTargetView * ppRenderTargets[] =
			{
				RenderTargets[TARGET_ACCUMULATION_RESOLVED].Get(),
				RenderTargets[TARGET_DEPTH_RESOLVED].Get()
			};

			CmdList.SetRenderTargets<_countof(ppRenderTargets)>(ppRenderTargets);

			CmdList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			CmdList.DrawInstanced(1, 4);
		}

		void CVolumetricLighting::ApplyLighting_TemporalFilter(const CScene * pScene, const PostprocessDesc & Postprocess)
		{
			const UINT NextFrameIndex = pScene->GetProperties().FrameIndexCurrent;
			const UINT LastFrameIndex = pScene->GetProperties().FrameIndexLast == NextFrameIndex ? (NextFrameIndex + 1) % FRAME_COUNT : pScene->GetProperties().FrameIndexLast;

			const RGrpCommandList & CmdList = CmdListCtx.GetRef();
			{
				Pipelines::VolumetricLighting::TemporalFilter::Instance().Apply(CmdListCtx.Get());
			}

			const UINT FrameOffsetNext = NextFrameIndex * 2;
			const UINT FrameOffsetLast = LastFrameIndex * 2;

			const RRenderTargetView * ppRenderTargets[] =
			{
				RenderTargets[TARGET_ACCUMULATION_FILTERED_0	+ FrameOffsetNext].Get(),
				RenderTargets[TARGET_DEPTH_FILTERED_0			+ FrameOffsetNext].Get()
			};

			const UINT ResourceIndices[] =
			{
				TARGET_DEPTH_RESOLVED,
				TARGET_ACCUMULATION_RESOLVED,
				TARGET_DEPTH_FILTERED_0			+ FrameOffsetNext,
				TARGET_ACCUMULATION_FILTERED_0	+ FrameOffsetNext
			};

			static const D3D12_RESOURCE_STATES ResourceStates[] =
			{
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_RENDER_TARGET
			};

			RResource::SetResourceStates<_countof(ResourceIndices)>(ShaderResources.ResourceObjects, ResourceIndices, ResourceStates, CmdList, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

			CmdList.SetGraphicsRootDescriptorTable(Pipelines::VolumetricLighting::TemporalFilter::PARAMETER_CBV_SRV_DESCRIPTOR_TABLE,	DescriptorHeapRangeLocal);
			CmdList.SetGraphicsRootDescriptorTable(Pipelines::VolumetricLighting::TemporalFilter::PARAMETER_SRV_FRAME_DESCRIPTOR_TABLE, DescriptorHeapRangeLocal[SRV_ACCUMULATION_FILTERED_0 + FrameOffsetLast]);
			CmdList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			CmdList.SetRenderTargets<_countof(ppRenderTargets)>(ppRenderTargets);
			CmdList.DrawInstanced(1, 3);
		}

		void CVolumetricLighting::ApplyLighting_Composite(const CScene * pScene, const PostprocessDesc & Postprocess)
		{
			const RGrpCommandList & CmdList = CmdListCtx.GetRef();

			if (pScene->GetProperties().IsMSAA())
			{
				Pipelines::VolumetricLighting::ApplyLightingMSAA::Instance().Apply(CmdListCtx.Get());
			}
			else
			{
				Pipelines::VolumetricLighting::ApplyLighting::Instance().Apply(CmdListCtx.Get());
			}

			CmdList.SetViewport(pScene->GetView().GetViewport());
			CmdList.SetScissorRect(pScene->GetView().GetViewport().ScissorRect);

			const UINT FrameOffset = pScene->GetProperties().FrameIndexCurrent * 2;
			
			if (FilterMode != FILTER_TEMPORAL)
			{
				static const UINT ResourceIndices[] =
				{
					TARGET_LUT,
					TARGET_DEPTH_RESOLVED,
					TARGET_ACCUMULATION_RESOLVED
				};

				RResource::SetResourceStates<_countof(ResourceIndices)>(ShaderResources.ResourceObjects, ResourceIndices, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, CmdList, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

				if (pScene->GetProperties().IsMSAA())
				{
					CmdList.SetGraphicsRootDescriptorTable(Pipelines::VolumetricLighting::ApplyLighting::PARAMETER_SRV_FRAME_DESCRIPTOR_TABLE, DescriptorHeapRangeLocal[SRV_ACCUMULATION_RESOLVED]);
				}
				else
				{
					CmdList.SetGraphicsRootDescriptorTable(Pipelines::VolumetricLighting::ApplyLighting::PARAMETER_SRV_FRAME_DESCRIPTOR_TABLE, DescriptorHeapRangeLocal[SRV_ACCUMULATION]);
				}
			}
			else
			{
				const UINT ResourceIndices[] =
				{
					TARGET_LUT,
					TARGET_DEPTH_FILTERED_0			+ FrameOffset,
					TARGET_ACCUMULATION_FILTERED_0	+ FrameOffset
				};

				RResource::SetResourceStates<_countof(ResourceIndices)>(ShaderResources.ResourceObjects, ResourceIndices, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, CmdList, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

				CmdList.SetGraphicsRootDescriptorTable(Pipelines::VolumetricLighting::ApplyLighting::PARAMETER_SRV_FRAME_DESCRIPTOR_TABLE, DescriptorHeapRangeLocal[SRV_ACCUMULATION_FILTERED_0 + FrameOffset]);
			}

			const Float4 BlendFactors =
			{
				1.0f,
				1.0f,
				1.0f,
				1.0f
			};

			CmdList->OMSetBlendFactor(BlendFactors);
			CmdList->OMSetStencilRef(0xFF);

			CmdList.SetGraphicsRootDescriptorTable(Pipelines::VolumetricLighting::ApplyLighting::PARAMETER_CBV_SRV_DESCRIPTOR_TABLE,	DescriptorHeapRangeLocal);
			CmdList.SetGraphicsRootDescriptorTable(Pipelines::VolumetricLighting::ApplyLighting::PARAMETER_SRV_DEPTH_DESCRIPTOR_TABLE,	DescriptorHeapRangeSceneSRV);

			CmdList.SetRenderTarget(pScene->GetColorRTV().GetRef());

			CmdList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			CmdList.DrawInstanced(1, 3);
		}

		void CVolumetricLighting::ApplyLighting_End(const CScene * pScene, const PostprocessDesc & Postprocess)
		{
		}

		static inline float HaltonSequence(int Index, int Base)
		{
			float R = 0;
			float F = 1;

			int N = Index + 1;

			while (N > 0)
			{
				F /= float(Base);
				R += F * (N % Base);
				N /= Base;
			}

			return R;
		}

		Vector2f CVolumetricLighting::GetJitter() const
		{
			if (FilterMode == EFilterMode::FILTER_TEMPORAL)
			{
				return Vector2f
				(
					HaltonSequence(JitterIndex, 2) - 0.5f, 
					HaltonSequence(JitterIndex, 3) - 0.5f
				);
			}
			else
			{
				return Vector2f::ZeroVector;
			}
		}

		void CVolumetricLighting::BeginAccumulation_Start(const CScene * pScene)
		{
			const RGrpCommandList & CmdList = CmdListCtx.GetRef();

			CmdList.SetViewport(pScene->GetView().GetViewport());
			CmdList.SetScissorRect(pScene->GetView().GetViewport().ScissorRect);

			RResource * AccumulationResources[] =
			{
				DepthStencil->GetResource(),
				RenderTargets[TARGET_LUT]->GetResource(),
				RenderTargets[TARGET_ACCUMULATION]->GetResource()
			};

			static const D3D12_RESOURCE_STATES ResourceStates[] =
			{
				D3D12_RESOURCE_STATE_DEPTH_WRITE,
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_RENDER_TARGET
			};

			RResource::SetResourceStates<_countof(AccumulationResources)>(AccumulationResources, ResourceStates, CmdList, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

			SetupPerFrameConstants(pScene);
		}

		void CVolumetricLighting::BeginAccumulation_UpdateMediumLUT()
		{
			const RGrpCommandList & CmdList = CmdListCtx.GetRef();
			{
				Pipelines::VolumetricLighting::ComputePhaseLookup::Instance().Apply(CmdListCtx.Get());
			}

			CmdList.SetGraphicsRootDescriptorTable(0, DescriptorHeapRangeLocal);

			CmdList.SetViewport(LUTViewport);
			CmdList.SetScissorRect(LUTScissorRect);

			RenderTargets[TARGET_LUT]->Clear(CmdList, RGBAColor::Transparent.ColorArray);
			RenderTargets[TARGET_LUT]->SetAsRenderTarget(CmdList);
			
			CmdList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			CmdList.DrawInstanced(1, 3);
		}

		void CVolumetricLighting::BeginAccumulation_CopyDepth(const CScene * pScene) const
		{
			const RGrpCommandList & CmdList = CmdListCtx.GetRef();
			{
				Pipelines::VolumetricLighting::DownsamplePipeline::Instance().Apply(CmdListCtx.Get());
			}

			CmdList.SetGraphicsRootDescriptorTable(0, DescriptorHeapRangeLocal);
			CmdList.SetGraphicsRootDescriptorTable(1, DescriptorHeapRangeSceneSRV);

			DepthStencil->Clear(CmdList);

			CmdList.SetViewport(pScene->GetView().GetViewport());
			CmdList.SetScissorRect(pScene->GetView().GetViewport().ScissorRect);

			CmdList.SetRenderTarget(DepthStencil.GetRef());

			CmdList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			CmdList.DrawInstanced(1, 3);
		}

		void CVolumetricLighting::BeginAccumulation_End()
		{
			const RGrpCommandList & CmdList = CmdListCtx.GetRef();
			{
				RenderTargets[TARGET_ACCUMULATION]->Clear(CmdList, RGBAColor::Transparent.ColorArray);
			}
		}

		void CVolumetricLighting::RenderVolume_Start(const CScene * pScene, const ShadowMapDesc & ShadowMap)
		{
			const RGrpCommandList & CmdList = CmdListCtx.GetRef();

			DepthStencil->ClearStencil(CmdList);

			VolumeSettings.MaxResolution = std::max(ShadowMap.uWidth, ShadowMap.uHeight);

			SetupPerVolumeConstants(pScene, VolumeSettings, ShadowMap);
		}

		void CVolumetricLighting::RenderVolume_Directional(const CScene * pScene, const ShadowMapDesc & ShadowMap)
		{
			const RGrpCommandList & CmdList = CmdListCtx.GetRef();

			const RRenderTargetView & RTV = RenderTargets[TARGET_ACCUMULATION].GetRef();
			const RDepthStencilView & DSV = DepthStencil.GetRef();

			const uint32_t Resolution = VolumeSettings.GetCoarseResolution();

			Pipelines::VolumetricLighting::Directional::PipelineGeometryFrustumGrid::Instance().Apply(CmdListCtx.Get());
			{
				CmdList->OMSetStencilRef(0xFF);

				CmdList.SetGraphicsRootDescriptorTable(Pipelines::VolumetricLighting::Directional::PARAMETER_CBV_SRV_DESCRIPTOR_TABLE,		DescriptorHeapRangeLocal);
				CmdList.SetGraphicsRootDescriptorTable(Pipelines::VolumetricLighting::Directional::PARAMETER_SRV_DEPTH_DESCRIPTOR_TABLE,	DescriptorHeapRangeSceneSRV);
				CmdList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
				CmdList.SetRenderTarget(RTV, DSV);
				CmdList.DrawInstanced(1, 4 * Resolution * Resolution);
			}

			Pipelines::VolumetricLighting::Directional::PipelineGeometryFrustumBase::Instance().Apply(CmdListCtx.Get());
			{
				CmdList->OMSetStencilRef(0xFF);

				CmdList.SetGraphicsRootDescriptorTable(Pipelines::VolumetricLighting::Directional::PARAMETER_CBV_SRV_DESCRIPTOR_TABLE,		DescriptorHeapRangeLocal);
				CmdList.SetGraphicsRootDescriptorTable(Pipelines::VolumetricLighting::Directional::PARAMETER_SRV_DEPTH_DESCRIPTOR_TABLE,	DescriptorHeapRangeSceneSRV);
				CmdList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				CmdList.DrawInstanced(1, 6);
			}

			Pipelines::VolumetricLighting::Directional::PipelineSky::Instance().Apply(CmdListCtx.Get());
			{
				CmdList->OMSetStencilRef(0xFF);

				CmdList.SetGraphicsRootDescriptorTable(Pipelines::VolumetricLighting::Directional::PARAMETER_CBV_SRV_DESCRIPTOR_TABLE,		DescriptorHeapRangeLocal);
				CmdList.SetGraphicsRootDescriptorTable(Pipelines::VolumetricLighting::Directional::PARAMETER_SRV_DEPTH_DESCRIPTOR_TABLE,	DescriptorHeapRangeSceneSRV);
				CmdList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				CmdList.DrawInstanced(1, 3);
			}

			DSV.AsDepthRead(CmdList);

			Pipelines::VolumetricLighting::Directional::PipelineFinal::Instance().Apply(CmdListCtx.Get());
			{
				CmdList->OMSetStencilRef(0xFF);
				
				CmdList.SetGraphicsRootDescriptorTable(Pipelines::VolumetricLighting::Directional::PARAMETER_CBV_SRV_DESCRIPTOR_TABLE,		DescriptorHeapRangeLocal);
				CmdList.SetGraphicsRootDescriptorTable(Pipelines::VolumetricLighting::Directional::PARAMETER_SRV_DEPTH_DESCRIPTOR_TABLE,	DescriptorHeapRangeSceneSRV);
				CmdList.SetRenderTarget(RTV, DepthStencilReadOnly.GetRef());
				CmdList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				CmdList.DrawInstanced(1, 3);
			}
		}

		void CVolumetricLighting::RenderVolume_End()
		{
		}
	}
}
