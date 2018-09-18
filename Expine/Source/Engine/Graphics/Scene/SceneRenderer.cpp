#include "Precompiled.h"

#include "Scene/SceneRenderer.h"
#include "Scene/SceneView.h"
#include "PostProcess/PostProcess.h"

#include "Pipeline/Pipelines.h"
#include "Pipeline/PSOLine.h"
#include "Pipeline/PSOTriangle.h"

#include <future>

#define D3D_TEST

namespace D3D
{
	namespace Test
	{
		/*
		static inline void DrawGeometryTestCube
		(
			CDrawInterface * DrawInterface
		)
		{
			CGeometryLine * Geometry = new CGeometryLine(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			{
				const SimpleColorVertex Vertices[] =
				{
					{ { -1.0, -1.0,  1.0 }, { 1.0, 0.0, 0.0, 1.0 } },
					{ {  1.0, -1.0,  1.0 }, { 0.0, 1.0, 0.0, 1.0 } },
					{ {  1.0,  1.0,  1.0 }, { 1.0, 0.0, 0.0, 1.0 } },
					{ { -1.0,  1.0,  1.0 }, { 0.0, 1.0, 0.0, 1.0 } },
					{ { -1.0, -1.0, -1.0 }, { 1.0, 0.0, 0.0, 1.0 } },
					{ {  1.0, -1.0, -1.0 }, { 0.0, 1.0, 0.0, 1.0 } },
					{ {  1.0,  1.0, -1.0 }, { 1.0, 0.0, 0.0, 1.0 } },
					{ { -1.0,  1.0, -1.0 }, { 0.0, 1.0, 0.0, 1.0 } },
				};

				Geometry->AddFaces(Vertices, Geometries::CubeIndices, _countof(Vertices), _countof(Geometries::CubeIndices));

				D3D12_DRAW_INDEXED_ARGUMENTS Arguments;
				{
					Arguments.InstanceCount = 1;
					Arguments.BaseVertexLocation = 0;
					Arguments.StartIndexLocation = 0;
					Arguments.StartInstanceLocation = 0;
					Arguments.IndexCountPerInstance = _countof(Geometries::CubeIndices);
				}

				Geometry->AddCommand(Arguments);
			}

			ThrowOnError(Geometry->CreateBuffers());

			DrawInterface->AddDynamicGeometry(Geometry, Pipelines::PipelineWireframeTriangle::Instance_Pointer());
		}*/
	}

	ErrorCode CSceneRenderer::CreateDrawInterface()
	{
		std::promise<ErrorCode> Error;

		RenderThreads[CMD_LIST_GEOMETRY].GetFunctionSequence().AddFunction(new CFunction([&]
		{
			if (!Scene->GetOutputScreen())
			{
				Error.set_value(E_FAIL);
			}
			else
			{
				DrawInterface = new CDrawInterface(&RenderingCommandLists[CMD_LIST_COMPOSITE]);
				{
					DrawInterface->SetRenderTarget(Scene->GetGeometryRTV().Get());
				}

				Error.set_value(DrawInterface->Create());
			}
		}));

		RenderThreads[CMD_LIST_GEOMETRY].Run();
		RenderThreads[CMD_LIST_GEOMETRY].WaitForCompletion();

		ErrorCode EC = Error.get_future().get();

		if (EC)
		{
			return EC;
		}

		RenderThreads[CMD_LIST_GEOMETRY].GetFunctionSequence().ClearAll();
		RenderThreads[CMD_LIST_GEOMETRY].GetFunctionSequence().AddFunction(new CFunction(&RenderGeometry, this));

		return S_OK;
	}

	ErrorCode CSceneRenderer::CreateRenderThreads()
	{
		for (UINT N = 0; N < CMD_LIST_NUM; ++N)
		{
			if (!RenderThreads[N].Initialize())
			{
				return GetLastError();
			}
		}

		RenderThreads[CMD_LIST_COMPOSITE].GetFunctionSequence().AddFunction(new CFunction(&RenderComposite, this));
		RenderThreads[CMD_LIST_COMPUTE].GetFunctionSequence().AddFunction(new CFunction(&RenderOcclusionMap, this));

		return S_OK;
	}

	void CSceneRenderer::RenderGeometry()
	{
		const UINT Frame = Scene->GetOutputScreen()->GetBackBufferIndex();

		if (RenderParts[RENDER_FRUSTUM])
		{
			RenderViewFrustum();
		}

		const RGrpCommandList & CmdList = RenderingCommandLists[CMD_LIST_GEOMETRY];

		const RDepthStencilView & DepthStencil = Scene->GetDepthDSV().GetRef();
		const RRenderTargetView & RenderTarget = Scene->GetGeometryRTV().GetRef();

		CmdList.SetRenderTarget(RenderTarget, DepthStencil);
		{
			RenderTarget.Clear(CmdList);
			DepthStencil.Clear(CmdList);
		}

		DrawInterface->DrawGeometry(Scene->GetView());
	}

	void CSceneRenderer::RenderShadowMap()
	{
		if (OutdoorRenderer)
		{
			OutdoorRenderer->RenderShadows();
		}
	}

	void CSceneRenderer::RenderOutdoor()
	{
		if (OutdoorRenderer)
		{
			OutdoorRenderer->Render();
		}
	}

	void CSceneRenderer::RenderOcclusionMap()
	{
		UINT Frame = Scene->GetProperties().FrameIndexCurrent;

		ThrowOnError(CommandContextCompute.ResetCommandList(Frame));

		if (OutdoorRenderer)
		{
			Scene->GetOcclusionMap().GetBufferResource()->AsUnorderedAccessView(CommandContextCompute);
			{
				OutdoorRenderer->RenderOcclusionMap();
			}
		}

		ThrowOnError(CommandContextCompute.Close());
		{
			CommandContextCompute.Finish(Frame);
		}
	}

	void CSceneRenderer::RenderViewFrustum()
	{
		const CSceneView & View = Scene->GetViewForInput();

		CGeometryLine * Lines = new CGeometryLine();
		CGeometryTriangle * Triangles = new CGeometryTriangle();

		ViewFrustum::CreateWireframe(90.0, 0.1, 100000.0, View.GetAspect(), 
			Vector4f(0.4, 0.4, 1.0, 0.1), 
			Vector4f(0.4, 0.4, 1.0, 0.8), View.GetViewSetup().ViewInverseMatrix, Lines, Triangles);
		{
			Lines->CreateBuffers();
			Triangles->CreateBuffers();

			DrawInterface->AddDynamicGeometry(Lines, 
				Pipelines::PipelineColorLine::Instance_Pointer());
			DrawInterface->AddDynamicGeometry(Triangles, 
				Pipelines::PipelineColorTriangle::Instance_Pointer());
		}
	}

	void CSceneRenderer::RenderComposite()
	{
		CompositeRenderer->Render();
	}

	void CSceneRenderer::RenderToBackBuffer()
	{
		RRenderTargetView * BackBuffer = Scene->GetOutputScreen()->GetBackBuffer();

		BackBuffer->GetResource()->AsRenderTarget(CommandContext);
		BackBuffer->Clear(CommandContext);

		if (PostProcess)
		{
			BackBuffer->SetAsRenderTarget(CommandContext);

			Pipelines::PipelinePresent::Instance().Apply(&CommandContext);
			{
				CommandContext.SetGraphicsRootDescriptorTable(Pipelines::PipelinePresent::SceneComposite, DIDSceneSRV);
			}

			CommandContext.SetViewport(Scene->GetOutputScreen()->GetViewport());
			CommandContext.SetScissorRect(Scene->GetOutputScreen()->GetViewport().ScissorRect);
			CommandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			CommandContext.DrawInstanced(1, 3);
		}
		else
		{
			PostProcess::PostProcessTarget Target;
			{
				Target.RenderTarget		= BackBuffer;
				Target.SizeDestination	= Scene->GetView().GetSize();
				Target.SizeSource		= Scene->GetView().GetSize();
				Target.ViewPort			= Scene->GetView().GetViewport();
				Target.ViewRect			= Scene->GetView().GetViewport().ScissorRect;
			}

			PostProcess->Process(Target);
		}

		BackBuffer->GetResource()->AsPresent(CommandContext);
	}

	void CSceneRenderer::RenderLight()
	{
		if (!LightRenderer)
		{
			return;
		}

		Light::PostprocessDesc PostProcess;
		{
			PostProcess.bDoFog				= true;
			PostProcess.bIgnoreSkyFog		= false;
			PostProcess.eUpsampleQuality	= Light::UpsampleQuality::BILINEAR;
			PostProcess.fBlendfactor		= 1.0f;
			PostProcess.fFilterThreshold	= 0.20f;
			PostProcess.fMultiscatter		= 0.000002f;
			PostProcess.fTemporalFactor		= 0.95f;
			PostProcess.vFogLight			= Scene->GetLight().GetProperties().GetLightIntesity();
			PostProcess.mUnjitteredViewProj = Scene->GetView().GetViewSetup().ViewProjectionMatrix;
		}

		LightRenderer->BeginAccumulation();
		LightRenderer->StartVolumeRender();
		LightRenderer->EndAccumulation();

		LightRenderer->ApplyLighting(PostProcess);
	}

	ErrorCode CSceneRenderer::InitializeCommandLists()
	{
		ErrorCode Error;

		if ((Error = CommandContext.Create(true)))
		{
			return Error;
		}

		CommandContext->SetName(L"CMDList Scene Graphics");

		DescriptorHeapRange RangeCBV = CommandContext.OccupyViewDescriptorRange(NumCBVDescriptors);
		DescriptorHeapRange RangeSRV = CommandContext.OccupyViewDescriptorRange(NumSRVDescriptors);
		DescriptorHeapRange RangeUAV = CommandContext.OccupyViewDescriptorRange(NumUAVDescriptors);
		{
			RangeCBV.DescriptorHeap->CopyDescriptorHeapEntries(RangeCBV, Scene->GetDescriptorHeapCBV()->AsDescriptorHeapRange());
			RangeSRV.DescriptorHeap->CopyDescriptorHeapEntries(RangeSRV, Scene->GetDescriptorHeapSRV()->AsDescriptorHeapRange());
			RangeUAV.DescriptorHeap->CopyDescriptorHeapEntries(RangeUAV, Scene->GetDescriptorHeapUAV()->AsDescriptorHeapRange());

			CommandContext.SetDescriptorHeapTableStaticRange(DIDSceneCBV, RangeCBV);
			CommandContext.SetDescriptorHeapTableStaticRange(DIDSceneSRV, RangeSRV);
			CommandContext.SetDescriptorHeapTableStaticRange(DIDSceneUAV, RangeUAV);
		}

		if ((Error = CommandContextCompute.Create(false, D3D12_COMMAND_LIST_TYPE_COMPUTE)))
		{
			return Error;
		}

		CommandContextCompute->SetName(L"CMDList Scene Compute");

		if ((Error = RenderingCommandLists.Create(true)))
		{
			return Error;
		}

		for (UINT N = 0; N < CMD_LIST_NUM; ++N)
		{
			switch (N)
			{
				case CMD_LIST_COMPOSITE:
				{
					RenderingCommandLists[N]->SetName(L"CMDList Composite");
				}

				break;

				case CMD_LIST_GEOMETRY:
				{
					RenderingCommandLists[N]->SetName(L"CMDList Geometry");
				}

				break;

				case CMD_LIST_PRESENT:
				{
					RenderingCommandLists[N]->SetName(L"CMDList Present");
				}

				break;

				case CMD_LIST_SHADOWS:
				{
					RenderingCommandLists[N]->SetName(L"CMDList Shadows");
				}

				break;
			}

			RangeCBV = RenderingCommandLists[N].OccupyViewDescriptorRange(NumCBVDescriptors);
			RangeSRV = RenderingCommandLists[N].OccupyViewDescriptorRange(NumSRVDescriptors);
			RangeUAV = RenderingCommandLists[N].OccupyViewDescriptorRange(NumUAVDescriptors);

			RangeCBV.DescriptorHeap->CopyDescriptorHeapEntries(RangeCBV, Scene->GetDescriptorHeapCBV()->AsDescriptorHeapRange());
			RangeSRV.DescriptorHeap->CopyDescriptorHeapEntries(RangeSRV, Scene->GetDescriptorHeapSRV()->AsDescriptorHeapRange());
			RangeUAV.DescriptorHeap->CopyDescriptorHeapEntries(RangeUAV, Scene->GetDescriptorHeapUAV()->AsDescriptorHeapRange());

			RenderingCommandLists[N].SetDescriptorHeapTableStaticRange(DIDSceneCBV, RangeCBV);
			RenderingCommandLists[N].SetDescriptorHeapTableStaticRange(DIDSceneSRV, RangeSRV);
			RenderingCommandLists[N].SetDescriptorHeapTableStaticRange(DIDSceneUAV, RangeUAV);
		}

		return S_OK;
	}

	ErrorCode CSceneRenderer::InitializeOutdoorRenderer()
	{
		ErrorCode Error;

		OutdoorRenderer = new CSceneOutdoor::CRenderer(this);

		if ((Error = OutdoorRenderer->Create()))
		{
			return Error;
		}

		return S_OK;
	}

	ErrorCode CSceneRenderer::InitializeIndoorRenderer()
	{
		return ErrorCode();
	}

	ErrorCode CSceneRenderer::InitializeRenderParts()
	{
		ErrorCode Error;

		LightRenderer = new CSceneLight::CRenderer(this);

		if ((Error = LightRenderer->Create()))
		{
			return Error;
		}

		CompositeRenderer = new CSceneComposite::CRenderer(this);

		if ((Error = CompositeRenderer->Create()))
		{
			return Error;
		}

		PostProcess = new PostProcess::CPostProcess(this);

		if ((Error = PostProcess->Create()))
		{
			return Error;
		}

		return S_OK;
	}

	CSceneRenderer::CSceneRenderer(const CScene * pScene) : Scene(pScene)
	{
		RenderParts[RENDER_FRUSTUM]		= true;
		RenderParts[RENDER_GEOMETRY]	= true;
		RenderParts[RENDER_OUTDOOR]		= true;
		RenderParts[RENDER_SHADOWS]		= true;
		RenderParts[RENDER_LIGHTS]		= true;
	}

	ErrorCode CSceneRenderer::Create()
	{
		if (!Scene)
		{
			return E_FAIL;
		}

		ErrorCode Error;

		if ((Error = CreateRenderThreads()))
		{
			return Error;
		}

		if ((Error = InitializeCommandLists()))
		{
			return Error;
		}

		if ((Error = CreateDrawInterface()))
		{
			return Error;
		}

		if ((Error = CommandContext.GetCommandList()->Close()))
		{
			return Error;
		}

		CommandContext.Finish(0);

		if ((Error = InitializeRenderParts()))
		{
			return Error;
		}

		if ((Error = RenderingCommandLists.Close()))
		{
			return Error;
		}
		
		RenderingCommandLists.Finish(0);

		return S_OK;
	}

	void CSceneRenderer::Flush()
	{
		DrawInterface->Flush();
	}

	void CSceneRenderer::ClearScene(const RGrpCommandList & CmdList)
	{
		const RRenderTargetView & RenderTargetColor		= Scene->GetColorRTV().GetRef();
		const RRenderTargetView & RenderTargetNormals	= Scene->GetNormalRTV().GetRef();
		const RRenderTargetView & RenderTargetGeometry	= Scene->GetGeometryRTV().GetRef();
		const RRenderTargetView & RenderTargetComposite	= Scene->GetCompositeRTV().GetRef();
		const RDepthStencilView & DepthStencil			= Scene->GetDepthDSV().GetRef();
		const RDepthStencilView & ShadowMap				= Scene->GetShadowDSV().GetRef();

		CmdList.Clear(DepthStencil);
		CmdList.Clear(RenderTargetColor);
		CmdList.Clear(RenderTargetNormals);
		CmdList.Clear(RenderTargetGeometry);
	}

	void CSceneRenderer::EndScene()
	{
		const UINT Frame = Scene->GetOutputScreen()->GetBackBufferIndex();

		const RRenderTargetView & RenderTargetColor		= Scene->GetColorRTV().GetRef();
		const RRenderTargetView & RenderTargetNormals	= Scene->GetNormalRTV().GetRef();
		const RRenderTargetView & RenderTargetGeometry	= Scene->GetGeometryRTV().GetRef();
		const RRenderTargetView & RenderTargetComposite = Scene->GetCompositeRTV().GetRef();
		const RDepthStencilView & DepthStencil			= Scene->GetDepthDSV().GetRef();
		const RDepthStencilView & ShadowMap				= Scene->GetShadowDSV().GetRef();
		
		CCommandListContext & CmdContextComposite = RenderingCommandLists[CMD_LIST_COMPOSITE];

		ThrowOnError(CmdContextComposite.ResetCommandAllocator(Frame));
		ThrowOnError(CmdContextComposite.ResetCommandList(Frame));

		Scene->GetOcclusionMap().GetBufferResource()->AsPixelShaderResource(CmdContextComposite);

		// Composite

		RenderComposite();

		CmdContextComposite.SetRenderTarget(RenderTargetComposite, DepthStencil);

		if (RenderParts[RENDER_FRUSTUM])
		{
			RenderViewFrustum();
		}

		DrawInterface->DrawGeometry(Scene->GetView());

		ThrowOnError(CmdContextComposite.Close());
		{
			CmdContextComposite.Finish(Frame);
		}

		// Render to back buffer

		ThrowOnError(CommandContext.ResetCommandAllocator(Frame));
		ThrowOnError(CommandContext.ResetCommandList(Frame));

		RResource * Resources[] =
		{
			RenderTargetColor.GetResource(),
			RenderTargetNormals.GetResource(),
			RenderTargetGeometry.GetResource(),
			RenderTargetComposite.GetResource(),
			DepthStencil.GetResource(),
			ShadowMap.GetResource()
		};

		static const D3D12_RESOURCE_STATES ResourceStates[] =
		{
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			D3D12_RESOURCE_STATE_DEPTH_WRITE
		};

		RResource::SetResourceStates<_countof(Resources)>
		(
			Resources, 
			ResourceStates, 
			CommandContext,
			D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES
		);

		RenderToBackBuffer();

		ThrowOnError(CommandContext.Close());
		{
			CommandContext.Finish(Frame);
		}

		Scene->GetOutputScreen()->PresentSync();
		Scene->Update();
	}

	void CSceneRenderer::BeginScene(const RenderPass& RenderPass)
	{
		const UINT Frame = Scene->GetOutputScreen()->GetBackBufferIndex();

		DrawInterface->Flush();

		CCommandListContext & CmdContextShadows  = RenderingCommandLists[CMD_LIST_SHADOWS];
		CCommandListContext & CmdContextGeometry = RenderingCommandLists[CMD_LIST_GEOMETRY];

		ThrowOnError(CmdContextGeometry.ResetCommandAllocator(Frame));
		ThrowOnError(CmdContextGeometry.ResetCommandList(Frame));

		Scene->GetOcclusionMap().GetBufferResource()->AsNonPixelShaderResource(CmdContextGeometry);

		const RRenderTargetView & RenderTargetColor		= Scene->GetColorRTV().GetRef();
		const RRenderTargetView & RenderTargetNormals	= Scene->GetNormalRTV().GetRef();
		const RRenderTargetView & RenderTargetGeometry	= Scene->GetGeometryRTV().GetRef();
		const RRenderTargetView & RenderTargetComposite	= Scene->GetCompositeRTV().GetRef();
		const RDepthStencilView & DepthStencil			= Scene->GetDepthDSV().GetRef();
		const RDepthStencilView & ShadowMap				= Scene->GetShadowDSV().GetRef();

		RResource * RTVResources[4] =
		{
			RenderTargetColor.GetResource(),
			RenderTargetNormals.GetResource(),
			RenderTargetGeometry.GetResource(),
			RenderTargetComposite.GetResource()
		};

		RResource * DSVResources[2] =
		{
			DepthStencil.GetResource(),
			ShadowMap.GetResource()
		};

		const D3D12_RESOURCE_STATES ResourceStatesRTV[4] =
		{
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		};

		const D3D12_RESOURCE_STATES ResourceStatesDSV[2] =
		{
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		};

		const CSceneView & View = Scene->GetView();

		ClearScene(CmdContextGeometry);

		CmdContextGeometry.SetViewport(View.GetViewport());
		CmdContextGeometry.SetScissorRect(View.GetViewport().ScissorRect);

		if (RenderParts[RENDER_OUTDOOR])
		{
			RenderOutdoor();
		}

		if (RenderParts[RENDER_INDOOR])
		{
			// TODO
		}

		ThrowOnError(CmdContextGeometry.Close());

		CmdContextGeometry.Finish(Frame);

		ThrowOnError(CmdContextShadows.ResetCommandAllocator(Frame));
		ThrowOnError(CmdContextShadows.ResetCommandList(Frame));

		if (RenderPass.RenderShadows && RenderParts[RENDER_SHADOWS])
		{
			ShadowMap.ClearDepth(CmdContextShadows);

			CmdContextShadows.SetViewport(Scene->GetView().GetViewportShadowMap());
			CmdContextShadows.SetScissorRect(Scene->GetView().GetViewportShadowMap().ScissorRect);

			CmdContextShadows.SetNullIndexBuffer();
			CmdContextShadows.SetNullVertexBuffer();

			RenderShadowMap();
		}

		RResource::SetResourceStates<_countof(DSVResources)>(DSVResources, ResourceStatesDSV, CmdContextShadows, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

		if (RenderParts[RENDER_OUTDOOR])
		{
			OutdoorRenderer->RenderFog();
		}

		if (RenderParts[RENDER_LIGHTS])
		{
			RenderLight();
		}

		RResource::SetResourceStates<_countof(RTVResources)>(RTVResources, ResourceStatesRTV, CmdContextShadows, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

		ThrowOnError(CmdContextShadows.Close());

		CmdContextShadows.Finish(Frame);
	}

	void CSceneRenderer::Render(const RenderPass& RenderPass)
	{
		BeginScene(RenderPass);
		EndScene();
	}

	void CSceneRenderer::Update()
	{
		const CSceneArea * Area = Scene->GetArea();

		if (Area)
		{
			switch (Area->GetAreaType())
			{
				case AreaOutdoor:
				{
					if (!OutdoorRenderer)
					{
						ThrowOnError(InitializeOutdoorRenderer());
					}

					OutdoorRenderer->Update();
				}
				
				break;

				case AreaIndoor:
				{
					// TODO
				}
				
				break;
			}
		}
	}

	void CSceneRenderer::RenderOcclusion()
	{
		RenderThreads[CMD_LIST_COMPUTE].Run();
		RenderThreads[CMD_LIST_COMPUTE].WaitForCompletion();
	}
}
