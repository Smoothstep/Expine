#include "Precompiled.h"

#include "Buffer/BufferConstant.h"
#include "Pipeline/PSOAtmosphere.h"
#include "Resource/Texture/TextureResource.h"

#include "Scene/Scene.h"
#include "Scene/SceneView.h"
#include "Scene/SceneRenderer.h"
#include "Scene/Outdoor/AtmosphericScattering.h"

namespace D3D
{
	namespace Atmosphere
	{
		ErrorCode CScattering::InitializeBuffers()
		{
			ErrorCode Error;

			ConstantBuffer = new CConstantBuffer(new GrpConstantBufferDescriptor(
			{
				sizeof(AtmosphereConstants)
			}));

			if ((Error = ConstantBuffer->Create(ViewDescriptorHeap.Get())))
			{
				return Error;
			}

			ConstantBuffer->GetBufferData().MapData(0, 0, 0, &AtmosphereDescriptor);

			FogVertexBuffer = new CVertexBuffer(new GrpVertexBufferDescriptor(sizeof(Vector2f), 4));

			if ((Error = FogVertexBuffer->Create()))
			{
				return Error;
			}

			FogIndexBuffer = new CIndexBuffer(new GrpIndexBufferDescriptor(sizeof(UINT16), 6));

			if ((Error = FogIndexBuffer->Create()))
			{
				return Error;
			}

			const Vector2f Vertices[4] =
			{
				Vector2f(-1,-1),
				Vector2f(+1,-1),
				Vector2f(+1,+1),
				Vector2f(-1,+1)
			};

			FogVertexBuffer->GetBufferData().MapData(0, 0, 0, Vertices);

			const UINT16 Indices[6] =
			{
				0, 1, 2,
				2, 3, 0
			};

			FogIndexBuffer->GetBufferData().MapData(0, 0, 0, Indices);

			return S_OK;
		}

		void CScattering::PrepareRenderer(const CSceneRenderer * pSceneRenderer)
		{
			const CScene * pScene = pSceneRenderer->GetScene();

			CommandListContext->SetGraphicsRootDescriptorTable(0, ViewDescriptorHeapRange);
			CommandListContext->SetGraphicsRootDescriptorTable(1, ViewDescriptorHeapRange);
			CommandListContext->SetGraphicsRootDescriptorTable(2, ViewDescriptorHeapRangeCBV);
			CommandListContext->SetGraphicsRootDescriptorTable(3, ViewDescriptorHeapRangeSRV);

			AtmosphereDescriptor.AtmosphericFogSunDirection = Vector4f(-pScene->GetLight().GetProperties().Directional.Direction, 1);

			ConstantBuffer->GetBufferData().MapData(0, 0, 0, &AtmosphereDescriptor);
		}

		void CScattering::GetLayerValue(Uint Layer, Float & AtmosphereR, Vector4f & DhdH)
		{
			const float RadiusGround = 6360;
			const float RadiusAtmosphere = 6420;
			float R = Layer / std::max<float>(AtmosphereDescriptor.AtmosphericFogInscatterAltitudeSampleNum - 1.f, 1.f);
			R = R * R;
			R = sqrt(RadiusGround * RadiusGround + R * (RadiusAtmosphere * RadiusAtmosphere - RadiusGround * RadiusGround)) + (Layer == 0 ? 0.01 : (Layer == AtmosphereDescriptor.AtmosphericFogInscatterAltitudeSampleNum - 1 ? -0.001 : 0.0));
			float DMin = RadiusAtmosphere - R;
			float DMax = sqrt(R * R - RadiusGround * RadiusGround) + sqrt(RadiusAtmosphere * RadiusAtmosphere - RadiusGround * RadiusGround);
			float DMinP = R - RadiusGround;
			float DMaxP = sqrt(R * R - RadiusGround * RadiusGround);
			AtmosphereR = R;
			DhdH = Vector4f(DMin, DMax, DMinP, DMaxP);
		}

		void CScattering::UpdateRenderer()
		{
			switch (Phase)
			{
				case AP_Inscatter1:
				case AP_CopyInscatter1:
				case AP_CopyInscatterF:
				case AP_CopyInscatterFBack:
				case AP_InscatterN:
				case AP_CopyInscatterN:
				case AP_InscatterS:
				{
					TextureIndex++;

					if (TextureIndex >= AtmosphereProperties.InscatterAltitudeSampleNum)
					{
						Phase++;
						TextureIndex = 0;
					}
				}

				break;

				default:
				{
					Phase++;
				}

				break;
			}

			if (Phase == AP_EndOrder)
			{
				Phase = AP_StartOrder;
				Order++;
			}

			if (Phase == AP_StartOrder)
			{
				if (Order > AtmosphereProperties.MaxOrdering)
				{
					if (AtmosphereProperties.DensityHeight > 0.95f) // Fixed artifacts only for some value
					{
						Phase = AP_CopyInscatterF;
					}
					else
					{
						Phase = AP_MAX;
					}

					Order = 2;
				}
			}
		}

		void CScattering::RenderTransmittance()
		{
			CommandListContext->SetRenderTarget(RenderTargets[SRV_Transmittance].GetRef());
			CommandListContext->DrawInstanced(1, 3);
		}

		void CScattering::RenderIrradiance()
		{
			CommandListContext->SetRenderTarget(RenderTargets[SRV_Irradiance].GetRef());
			CommandListContext->DrawInstanced(1, 3);
		}

		void CScattering::RenderInscatter()
		{
			AtmosphereInscatterConstants Inscatter;
			{
				Inscatter.AtmosphereLayer = TextureIndex;
				{
					GetLayerValue(Inscatter.AtmosphereLayer, Inscatter.AtmosphereR, Inscatter.DhdH);
				}
			}

			CommandListContext->SetGraphicsRoot32BitConstants(4, 4, &Inscatter.DhdH);
			CommandListContext->SetGraphicsRoot32BitConstants(5, 1, &Inscatter.AtmosphereR);
			CommandListContext->SetGraphicsRoot32BitConstants(6, 1, &Inscatter.AtmosphereLayer);
			CommandListContext->SetGraphicsRoot32BitConstants(7, 1, &Inscatter.FirstOrder);

			const RRenderTargetView * RTVs[] =
			{
				RenderTargets[SRV_DeltaSR].Get(),
				RenderTargets[SRV_DeltaSM].Get()
			};

			CommandListContext->SetRenderTargets<_countof(RTVs)>(RTVs);
			CommandListContext->DrawInstanced(1, 3);
		}

		void CScattering::ClearIrradiance()
		{
			RenderTargets[SRV_Irradiance]->Clear(CommandListContext.GetRef(), RGBAColor::Transparent.ColorArray);
		}

		void CScattering::CopyInscatter()
		{
			AtmosphereInscatterConstants Inscatter;
			{
				Inscatter.AtmosphereLayer = TextureIndex;
				{
					GetLayerValue(Inscatter.AtmosphereLayer, Inscatter.AtmosphereR, Inscatter.DhdH);
				}
			}

			CommandListContext->SetGraphicsRoot32BitConstants(4, 4, &Inscatter.DhdH);
			CommandListContext->SetGraphicsRoot32BitConstants(5, 1, &Inscatter.AtmosphereR);
			CommandListContext->SetGraphicsRoot32BitConstants(6, 1, &Inscatter.AtmosphereLayer);
			CommandListContext->SetGraphicsRoot32BitConstants(7, 1, &Inscatter.FirstOrder);

			CommandListContext->SetRenderTarget(RenderTargets[SRV_Inscatter].GetRef());
			CommandListContext->DrawInstanced(1, 3);
		}

		void CScattering::InscatterS()
		{
			AtmosphereInscatterConstants Inscatter;
			{
				Inscatter.AtmosphereLayer = TextureIndex;
				{
					GetLayerValue(Inscatter.AtmosphereLayer, Inscatter.AtmosphereR, Inscatter.DhdH);
				}
			}

			CommandListContext->SetGraphicsRoot32BitConstants(4, 4, &Inscatter.DhdH);
			CommandListContext->SetGraphicsRoot32BitConstants(5, 1, &Inscatter.AtmosphereR);
			CommandListContext->SetGraphicsRoot32BitConstants(6, 1, &Inscatter.AtmosphereLayer);
			CommandListContext->SetGraphicsRoot32BitConstants(7, 1, &Inscatter.FirstOrder);

			CommandListContext->SetRenderTarget(RenderTargets[SRV_DeltaJ].GetRef());
			CommandListContext->DrawInstanced(1, 3);
		}

		void CScattering::IrradianceN()
		{
			UINT FirstOrder = Order == 2 ? 1 : 0;

			CommandListContext->SetGraphicsRoot32BitConstants(6, 1, &FirstOrder);

			CommandListContext->SetRenderTarget(RenderTargets[SRV_DeltaE].GetRef());
			CommandListContext->DrawInstanced(1, 3);
		}

		void CScattering::InscatterN()
		{
			AtmosphereInscatterConstants Inscatter;
			{
				Inscatter.AtmosphereLayer = TextureIndex;
				{
					GetLayerValue(Inscatter.AtmosphereLayer, Inscatter.AtmosphereR, Inscatter.DhdH);
				}

				Inscatter.FirstOrder = Order == 2 ? 1 : 0;
			}

			CommandListContext->SetGraphicsRoot32BitConstants(4, 4, &Inscatter.DhdH);
			CommandListContext->SetGraphicsRoot32BitConstants(5, 1, &Inscatter.AtmosphereR);
			CommandListContext->SetGraphicsRoot32BitConstants(6, 1, &Inscatter.AtmosphereLayer);
			CommandListContext->SetGraphicsRoot32BitConstants(7, 1, &Inscatter.FirstOrder);

			CommandListContext->SetRenderTarget(RenderTargets[SRV_DeltaSR].GetRef());
			CommandListContext->DrawInstanced(1, 3);
		}

		void CScattering::CopyIrradiance()
		{
			CommandListContext->SetRenderTarget(RenderTargets[SRV_Irradiance].GetRef());
			CommandListContext->DrawInstanced(1, 3);
		}

		void CScattering::CopyInscatterN()
		{
			AtmosphereInscatterConstants Inscatter;
			{
				Inscatter.AtmosphereLayer = TextureIndex;
				{
					GetLayerValue(Inscatter.AtmosphereLayer, Inscatter.AtmosphereR, Inscatter.DhdH);
				}
			}

			CommandListContext->SetGraphicsRoot32BitConstants(4, 4, &Inscatter.DhdH);
			CommandListContext->SetGraphicsRoot32BitConstants(5, 1, &Inscatter.AtmosphereR);
			CommandListContext->SetGraphicsRoot32BitConstants(6, 1, &Inscatter.AtmosphereLayer);
			CommandListContext->SetGraphicsRoot32BitConstants(7, 1, &Inscatter.FirstOrder);

			CommandListContext->SetRenderTarget(RenderTargets[SRV_Inscatter].GetRef());
			CommandListContext->DrawInstanced(1, 3);
		}

		void CScattering::CopyInscatterF()
		{
			AtmosphereInscatterConstants Inscatter;
			{
				Inscatter.AtmosphereLayer = TextureIndex;
				{
					GetLayerValue(Inscatter.AtmosphereLayer, Inscatter.AtmosphereR, Inscatter.DhdH);
				}
			}

			CommandListContext->SetGraphicsRoot32BitConstants(4, 4, &Inscatter.DhdH);
			CommandListContext->SetGraphicsRoot32BitConstants(5, 1, &Inscatter.AtmosphereR);
			CommandListContext->SetGraphicsRoot32BitConstants(6, 1, &Inscatter.AtmosphereLayer);
			CommandListContext->SetGraphicsRoot32BitConstants(7, 1, &Inscatter.FirstOrder);

			CommandListContext->SetRenderTarget(RenderTargets[SRV_DeltaSR].GetRef());
			CommandListContext->DrawInstanced(1, 3);
		}

		void CScattering::CopyInscatterFBack()
		{
			CopyInscatterN();
		}

		ErrorCode CScattering::InitializeViews()
		{
			SharedPointer<RResource> Textures[NumSRV];

			ErrorCode Error;

			for (UINT Index = 0; Index < NumSRV; ++Index)
			{
				Textures[Index] = new CTextureResource();

				D3D12_RTV_DIMENSION Dimension = D3D12_RTV_DIMENSION_TEXTURE2D;

				switch (Index)
				{
					case SRV_Transmittance:
					{
						if ((Error = Textures[Index]->Create(RResource::InitializeOptions::Texture2D(
							AtmosphereProperties.TransmittanceTexWidth,
							AtmosphereProperties.TransmittanceTexHeight,
							TransmittanceFormat))))
						{
							return Error;
						}
					}

					break;

					case SRV_DeltaE:
					case SRV_Irradiance:
					{
						if ((Error = Textures[Index]->Create(RResource::InitializeOptions::Texture2D(
							AtmosphereProperties.IrradianceTexWidth,
							AtmosphereProperties.IrradianceTexHeight,
							IrradianceFormat,
							1,
							RGBAColor::Transparent.ColorArray))))
						{
							return Error;
						}
					}

					break;

					case SRV_DeltaSR:
					case SRV_DeltaSM:
					case SRV_DeltaJ:
					case SRV_Inscatter:
					{
						if ((Error = Textures[Index]->Create(RResource::InitializeOptions::Texture3D(
							AtmosphereProperties.InscatterMuSNum * AtmosphereProperties.InscatterNuNum,
							AtmosphereProperties.InscatterNuNum,
							InscatterFormat,
							1,
							RGBAColor::Transparent.ColorArray,
							AtmosphereProperties.InscatterAltitudeSampleNum))))
						{
							return Error;
						}

						Dimension = D3D12_RTV_DIMENSION_TEXTURE3D;
					}

					break;
				}
			}

			Textures[SRV_DeltaE]->Get()->SetName(L"DeltaE");
			Textures[SRV_DeltaJ]->Get()->SetName(L"DeltaJ");
			Textures[SRV_DeltaSM]->Get()->SetName(L"DeltaSM");
			Textures[SRV_DeltaSR]->Get()->SetName(L"DeltaSR");
			Textures[SRV_Inscatter]->Get()->SetName(L"Inscatter");
			Textures[SRV_Irradiance]->Get()->SetName(L"Irradiance");
			Textures[SRV_Transmittance]->Get()->SetName(L"Transmittance");

			if ((Error = RenderTargets.Initialize(Textures)))
			{
				return Error;
			}

			if ((Error = ShaderResources.Initialize(Textures, ViewDescriptorHeap, NumCBV)))
			{
				return Error;
			}

			return S_OK;
		}

		void CScattering::GetCurrentRenderView(D3D12_VIEWPORT & Viewport, D3D12_RECT & ScissorRect)
		{
			Viewport	= { 0 };
			ScissorRect = { 0 };

			EAtmosphereTextureOffsets Offset = SRV_Transmittance;

			switch (Phase)
			{
				case AP_Transmittance:
				{
					break;
				}
				case AP_ClearIrradiance:
				case AP_CopyIrradiance:
				case AP_Irradiance1:
				case AP_IrradianceN:
				{
					Offset = SRV_Irradiance;
					break;
				}
				case AP_Inscatter1:
				case AP_CopyInscatter1:
				case AP_CopyInscatterF:
				case AP_CopyInscatterFBack:
				case AP_InscatterN:
				case AP_CopyInscatterN:
				case AP_InscatterS:
				{
					Offset = SRV_Inscatter;
					break;
				}
			}

			D3D12_RESOURCE_DESC ResourceDesc = RenderTargets[Offset]->GetResource()->Get()->GetDesc();

			Viewport.Height		= ScissorRect.bottom	= ResourceDesc.Height;
			Viewport.Width		= ScissorRect.right		= ResourceDesc.Width;
			Viewport.MaxDepth	= 1.0;
		}

		CScattering::CScattering()
		{
			AtmosphereProperties			= AtmosphereParameters(GlobalDefault);
			AtmosphereDescriptor			= AtmosphereConstants(AtmosphereProperties);
			AtmosphereInscatterDescriptor	= AtmosphereInscatterConstants(GlobalDefault);
		}

		CScattering::~CScattering()
		{}

		ErrorCode CScattering::Initialize()
		{
			ErrorCode Error;

			ViewDescriptorHeap = new RDescriptorHeap();

			if ((Error = ViewDescriptorHeap->Create_CBV_SRV_UAV(NumCBV + NumSRV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE)))
			{
				return Error;
			}

			if ((Error = InitializeBuffers()))
			{
				return Error;
			}

			if ((Error = InitializeViews()))
			{
				return Error;
			}

			return S_OK;
		}

		static inline UINT GetPipelinePhase(const UINT Phase)
		{
			switch (Phase)
			{
				case AP_Transmittance:
				{
					return eTransmittancePS;
				}
				case AP_ClearIrradiance:
				case AP_Irradiance1:
				{
					return eIrradiancePS;
				}
				case AP_Inscatter1:
				{
					return eInscatterPS;
				}
				case AP_CopyInscatter1:
				{
					return eInscatterCopyPS;
				}
				case AP_InscatterN:
				{
					return eInscatterNPS;
				}
				case AP_IrradianceN:
				{
					return eIrradianceNPS;
				}
				case AP_CopyInscatterF:
				{
					return eInscatterCopyFPS;
				}
				case AP_CopyInscatterFBack:
				{
					return eInscatterCopyFBackPS;
				}
				case AP_CopyInscatterN:
				{
					return eInscatterCopyNPS;
				}
				case AP_InscatterS:
				{
					return eInscatterSPS;
				}
				case AP_CopyIrradiance:
				{
					return eIrradianceCopyPS;
				}
			}

			return eNumPipelines;
		}

		static inline PipelineObjectBase * GetPipelineForPhase(const UINT Phase)
		{
			static PipelineObjectBase * PSO[] =
			{
				Pipelines::AtmosphericScattering::PipelineTransmittance::Instance_Pointer(),
				Pipelines::AtmosphericScattering::PipelineIrradiance::Instance_Pointer(),
				Pipelines::AtmosphericScattering::PipelineIrradianceN::Instance_Pointer(),
				Pipelines::AtmosphericScattering::PipelineIrradianceCopy::Instance_Pointer(),
				Pipelines::AtmosphericScattering::PipelineInscatter::Instance_Pointer(),
				Pipelines::AtmosphericScattering::PipelineInscatterCopy::Instance_Pointer(),
				Pipelines::AtmosphericScattering::PipelineInscatterCopyN::Instance_Pointer(),
				Pipelines::AtmosphericScattering::PipelineInscatterS::Instance_Pointer(),
				Pipelines::AtmosphericScattering::PipelineInscatterN::Instance_Pointer(),
				Pipelines::AtmosphericScattering::PipelineInscatterCopyF::Instance_Pointer(),
				Pipelines::AtmosphericScattering::PipelineInscatterCopyFBack::Instance_Pointer()
			};

			return PSO[Phase];
		}

		ErrorCode CScattering::Precompute(const CSceneRenderer * pSceneRenderer)
		{
			ErrorCode Error;

			if (!Finished())
			{
				UINT PipelinePhase;

				for (PipelinePhase = GetPipelinePhase(Phase); PipelinePhase >= eNumPipelines; PipelinePhase = GetPipelinePhase(Phase))
				{
					UpdateRenderer();
				}

				PipelineObjectBase * PSO = GetPipelineForPhase(PipelinePhase);

				if (PSO)
				{
					PSO->Apply(CommandListContext.Get());
				}

				PrepareRenderer(pSceneRenderer);

				D3D12_VIEWPORT Viewport;
				D3D12_RECT ScissorRect;

				GetCurrentRenderView(Viewport, ScissorRect);

				CommandListContext->SetViewport(Viewport);
				CommandListContext->SetScissorRect(ScissorRect);
				CommandListContext->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				const RGrpCommandList & CmdList = CommandListContext.GetRef();

				switch (Phase)
				{
					case AP_Transmittance:
					// Transmittance []
					{
						RenderTargets[SRV_Transmittance]->GetResource()->SetResourceState
						(
							D3D12_RESOURCE_STATE_RENDER_TARGET,
							CmdList
						);

						RenderTransmittance();
					}

					break;

					case AP_Irradiance1:
						// Irradiance : DeltaE [Transmittance]
					{
						static const UINT Indices[] =
						{
							SRV_Transmittance,
							SRV_Irradiance
						};

						static const D3D12_RESOURCE_STATES ResourceStates[] =
						{
							D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
							D3D12_RESOURCE_STATE_RENDER_TARGET
						};

						RResource::SetResourceStates<_countof(ResourceStates)>(ShaderResources.ResourceObjects, Indices, ResourceStates, CmdList, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

						RenderIrradiance();
					}

					break;

					case AP_Inscatter1:
						// Inscatter : DeltaSR & DeltaSM [Transmittance]
					{
						static const UINT Indices[] =
						{
							SRV_DeltaSR,
							SRV_DeltaSM
						};

						if (TextureIndex == 0)
						{
							RResource::SetResourceStates<_countof(Indices)>(ShaderResources.ResourceObjects, Indices, D3D12_RESOURCE_STATE_RENDER_TARGET, CmdList, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
						}

						RenderInscatter();
					}

					break;

					case AP_ClearIrradiance:
						// Clear Irradiance : Irradiance []
					{
						RenderTargets[SRV_Irradiance]->GetResource()->SetResourceState
						(
							D3D12_RESOURCE_STATE_RENDER_TARGET,
							CmdList
						);

						ClearIrradiance();
					}

					break;

					case AP_CopyInscatter1:

						// Copy Inscatter : Inscatter [DeltaSR & DeltaSM]
					{
						static const UINT Indices[] =
						{
							SRV_DeltaSR,
							SRV_DeltaSM,
							SRV_Inscatter
						};

						static const D3D12_RESOURCE_STATES ResourceStates[] =
						{
							D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
							D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
							D3D12_RESOURCE_STATE_RENDER_TARGET
						};

						if (TextureIndex == 0)
						{
							RResource::SetResourceStates<_countof(ResourceStates)>(ShaderResources.ResourceObjects, Indices, ResourceStates, CmdList, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
						}

						CopyInscatter();
					}

					break;

					case AP_InscatterS:
						// Inscatter S : DeltaJ [Transmittance & DeltaE & DeltaSR & DeltaSM]
					{
						RenderTargets[SRV_DeltaJ]->GetResource()->SetResourceState
						(
							D3D12_RESOURCE_STATE_RENDER_TARGET,
							CmdList
						);

						InscatterS();
					}

					break;

					case AP_IrradianceN:
						// Irradiance N : DeltaE [Transmittance & DeltaSR & DeltaSM]
					{
						RenderTargets[SRV_DeltaE]->GetResource()->SetResourceState
						(
							D3D12_RESOURCE_STATE_RENDER_TARGET,
							CmdList
						);

						IrradianceN();
					}

					break;

					case AP_InscatterN:
						// Inscatter N : DeltaSR [Transmittance DeltaJ]
					{
						static const UINT Indices[] =
						{
							SRV_DeltaJ,
							SRV_DeltaSR
						};

						static const D3D12_RESOURCE_STATES ResourceStates[] =
						{
							D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
							D3D12_RESOURCE_STATE_RENDER_TARGET
						};

						if (TextureIndex == 0)
						{
							RResource::SetResourceStates<_countof(ResourceStates)>(ShaderResources.ResourceObjects, Indices, ResourceStates, CmdList, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
						}

						InscatterN();
					}

					break;

					case AP_CopyIrradiance:
						// Copy Irradiance : Irradiance [DeltaE]
					{
						RenderTargets[SRV_DeltaE]->GetResource()->SetResourceState
						(
							D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
							CmdList
						);

						CopyIrradiance();
					}

					break;

					case AP_CopyInscatterN:
						// Copy Inscatter N : Inscatter [DeltaSR]
					{
						RenderTargets[SRV_DeltaSR]->GetResource()->SetResourceState
						(
							D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
							CmdList
						);

						CopyInscatterN();
					}

					break;

					case AP_CopyInscatterF:
						// Copy Inscatter F : DeltaSR [Inscatter]
					{
						static const UINT Indices[] =
						{
							SRV_Inscatter,
							SRV_DeltaSR
						};

						static const D3D12_RESOURCE_STATES ResourceStates[] =
						{
							D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
							D3D12_RESOURCE_STATE_RENDER_TARGET
						};

						if (TextureIndex == 0)
						{
							RResource::SetResourceStates<_countof(ResourceStates)>(ShaderResources.ResourceObjects, Indices, ResourceStates, CmdList, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
						}

						CopyInscatterF();
					}

					break;

					case AP_CopyInscatterFBack:
					{
						static const UINT Indices[] =
						{
							SRV_DeltaSR,
							SRV_Inscatter
						};

						static const D3D12_RESOURCE_STATES ResourceStates[] =
						{
							D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
							D3D12_RESOURCE_STATE_RENDER_TARGET
						};

						if (TextureIndex == 0)
						{
							RResource::SetResourceStates<_countof(ResourceStates)>(ShaderResources.ResourceObjects, Indices, ResourceStates, CmdList, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
						}

						CopyInscatterFBack();
					}

					break;
				}

				UpdateRenderer();
			}

			return S_OK;
		}

		void CScattering::Finalize(const CCommandListContext * pCommandListCtx)
		{
			if (ViewDescriptorHeapRange.Valid())
			{
				CDescriptorHeap * pDescriptorHeap = dynamic_cast<CDescriptorHeap*>(ViewDescriptorHeapRange.DescriptorHeap.Get());

				if (pDescriptorHeap)
				{
					pDescriptorHeap->UnregisterRange(ViewDescriptorHeapRange.Offset);
				}
			}

			CommandListContext = pCommandListCtx;

			Ensure
			(
				CommandListContext
			);

			ViewDescriptorHeapRange = CommandListContext->OccupyViewDescriptorRange(ViewDescriptorHeap->GetDescriptorsCount());
			ViewDescriptorHeapRange.DescriptorHeap->CopyDescriptorHeapEntries(ViewDescriptorHeapRange, ViewDescriptorHeap->AsDescriptorHeapRange());

			ViewDescriptorHeapRangeCBV = CommandListContext->GetDescriptorHeapRange(DIDSceneCBV);
			ViewDescriptorHeapRangeSRV = CommandListContext->GetDescriptorHeapRange(DIDSceneSRV);
		}

		void CScattering::Render(const CSceneRenderer * pSceneRenderer)
		{
			const CScene * pScene = pSceneRenderer->GetScene();

			CommandListContext->SetViewport(pScene->GetView().GetViewport());
			CommandListContext->SetScissorRect(pScene->GetView().GetViewport().ScissorRect);
			
			Pipelines::AtmosphericScattering::PipelineFog::Instance().Apply(CommandListContext.Get());
			{
				PrepareRenderer(pSceneRenderer);
			}

			CommandListContext->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			CommandListContext->SetRenderTarget(pScene->GetColorRTV().GetRef());
			CommandListContext->DrawInstanced(1, 3);
		}

		bool CScattering::Finished()
		{
			return Phase == AP_MAX;
		}

		AtmosphereParameters::AtmosphereParameters(const Default &)
		{
			InscatterAltitudeSampleNum	= 32;
			DensityHeight				= 0.75f;
			MaxOrdering					= 4;
			TransmittanceTexWidth		= 256;
			TransmittanceTexHeight		= 64;
			IrradianceTexWidth			= 64;
			IrradianceTexHeight			= 16;
			InscatterMuSNum				= 32;
			InscatterNuNum				= 8;
		}

		AtmosphereConstants::AtmosphereConstants(const AtmosphereParameters & AtmosphereProperties)
		{
			AtmosphericFogAltitudeScale					= 1.0f;
			AtmosphericFogDensityOffset					= 0.0f;
			AtmosphericFogDensityScale					= 1.0f;
			AtmosphericFogDistanceOffset				= 0.0f;
			AtmosphericFogDistanceScale					= 1.0f;
			AtmosphericFogGroundOffset					= -100000.0f;
			AtmosphericFogHeightScaleRayleigh			= AtmosphereProperties.DensityHeight * AtmosphereProperties.DensityHeight * AtmosphereProperties.DensityHeight * 64.0f;
			AtmosphericFogInscatterAltitudeSampleNum	= AtmosphereProperties.InscatterAltitudeSampleNum;
			AtmosphericFogPower							= 1.0f;
			AtmosphericFogStartDistance					= 1500.0f * 0.00001f;
			AtmosphericFogSunColor						= Vector4f(0.8f, 0.7f, 0.9f, 1.0f) * 1.5f;
			AtmosphericFogSunDirection					= Vector4f(0.5f, 0.5f, 0.0f, 0.0f);
			AtmosphericFogSunDiscScale					= 2.0f;
			AtmosphericFogSunPower						= 1.0f;
		}
}
}
