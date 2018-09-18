#include "Precompiled.h"

#include "Scene/Scene.h"
#include "Scene/SceneView.h"
#include "Scene/SceneRenderer.h"

namespace D3D
{
	RenderPass::RenderPass
	(
		Time::TimePointSteady & RenderStartTimePoint,
		Time::TimePointSteady & RenderEndTimePoint,
		Time::TimePointSteady & RenderShadowMapTimePoint
	) :
		RenderEndTimePoint(
			RenderEndTimePoint),
		RenderStartTimePoint(
			RenderStartTimePoint),
		RenderShadowMapTimePoint(
			RenderShadowMapTimePoint)
	{
		RenderShadows = (RenderStartTimePoint - RenderShadowMapTimePoint) > 0;
	}

	CScene::CScene(const CScreen * pScreen) : OutputScreen(pScreen), Properties(GlobalDefault), Renderer(this)
	{
		ViewInput = View = new CSceneView();
		{
			Views.insert(std::make_pair(0, View));
		}
	}

	CScene::~CScene()
	{}

	void CScene::SetCamera
	(
		const SharedPointer<CCamera> & pCamera
	)
	{
		Ensure(pCamera);

		Camera = pCamera;

		if (ViewInput)
		{
			ViewInput->Update(Camera.Get());
		}
	}

	ErrorCode CSceneComponents::CreateSceneDSV(const UintPoint SizeDepth, const UintPoint SizeShadowMap)
	{
		ErrorCode Error;

		SharedPointer<RResource> ResourceDepth = new RResource();
		{
			if ((Error = ResourceDepth->Create(RResource::InitializeOptions::DepthStencil2D(
				SizeDepth.X,
				SizeDepth.Y,
				DepthResourceFormat,
				1,
				D3D12_RESOURCE_STATE_DEPTH_WRITE,
				1,
				0.0f))))
			{
				return Error;
			}
		}

		ResourceDepth->Get()->SetName(L"Scene Depth Texture");

		SharedPointer<RResource> ResourceShadowMap = new RResource();
		{
			if ((Error = ResourceShadowMap->Create(RResource::InitializeOptions::DepthStencil2D(
				SizeShadowMap.X,
				SizeShadowMap.Y,
				ShadowResourceFormat,
				1,
				D3D12_RESOURCE_STATE_DEPTH_WRITE,
				1,
				0.0f))))
			{
				return Error;
			}
		}

		ResourceShadowMap->Get()->SetName(L"Scene Shadow Map Texture");

		SharedPointer<RResource> ppResources[] =
		{
			ResourceDepth,
			ResourceShadowMap
		};

		if ((Error = DepthStencilViews.Initialize(ppResources)))
		{
			return Error;
		}

		DSVResources[DSV_SCENE]		= ResourceDepth.Get();
		DSVResources[DSV_SHADOW]	= ResourceShadowMap.Get();

		return S_OK;
	}

	ErrorCode CSceneComponents::CreateSceneRTV(const UintPoint Size)
	{
		ErrorCode Error;

		SharedPointer<RResource> ColorResource = new RResource();
		{
			if ((Error = ColorResource->Create(RResource::InitializeOptions
			(
				Size.X,
				Size.Y,
				ColorSceneFormat,
				1, 
				RGBAColor::Black.ColorArray,
				D3D12_RESOURCE_DIMENSION_TEXTURE2D,
				1,
				D3D12_RESOURCE_STATE_RENDER_TARGET)
			)))
			{
				return Error;
			}
		}

		ColorResource->Get()->SetName(L"Scene Color Texture");

		SharedPointer<RResource> NormalResource = new RResource();
		{
			if ((Error = NormalResource->Create(RResource::InitializeOptions
			(
				Size.X,
				Size.Y,
				NormalSceneFormat,
				1,
				RGBAColor::Black.ColorArray,
				D3D12_RESOURCE_DIMENSION_TEXTURE2D,
				1,
				D3D12_RESOURCE_STATE_RENDER_TARGET)
			)))
			{
				return Error;
			}
		}

		NormalResource->Get()->SetName(L"Scene Normal Texture");

		SharedPointer<RResource> GeometryResource = new RResource();
		{
			if ((Error = GeometryResource->Create(RResource::InitializeOptions
			(
				Size.X,
				Size.Y,
				GeometrySceneFormat,
				1,
				RGBAColor::Black.ColorArray,
				D3D12_RESOURCE_DIMENSION_TEXTURE2D,
				1,
				D3D12_RESOURCE_STATE_RENDER_TARGET)
			)))
			{
				return Error;
			}
		}

		GeometryResource->Get()->SetName(L"Scene Geometry Texture");

		SharedPointer<RResource> CompositeResource = new RResource();
		{
			if ((Error = CompositeResource->Create(RResource::InitializeOptions
			(
				Size.X,
				Size.Y,
				CompositeSceneFormat,
				1,
				RGBAColor::Black.ColorArray,
				D3D12_RESOURCE_DIMENSION_TEXTURE2D,
				1,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
			)))
			{
				return Error;
			}
		}

		CompositeResource->Get()->SetName(L"Scene Composite Texture");

		SharedPointer<RResource> ppResources[] =
		{
			ColorResource,
			NormalResource,
			GeometryResource,
			CompositeResource
		};

		if ((Error = RenderTargetViews.Initialize(ppResources)))
		{
			return Error;
		}

		RTVResources[RTV_COLOR]		= ColorResource.Get();
		RTVResources[RTV_NORMAL]	= NormalResource.Get();
		RTVResources[RTV_GEOMETRY]	= GeometryResource.Get();
		RTVResources[RTV_COMPOSITE] = CompositeResource.Get();

		return S_OK;
	}

	ErrorCode CSceneComponents::CreateSceneSRV()
	{
		ErrorCode Error;

		SRVResources[SRV_COLOR]		= RTVResources[RTV_COLOR];
		SRVResources[SRV_NORMAL]	= RTVResources[RTV_NORMAL];
		SRVResources[SRV_GEOMETRY]	= RTVResources[RTV_GEOMETRY];
		SRVResources[SRV_COMPOSITE] = RTVResources[RTV_COMPOSITE];
		SRVResources[SRV_DEPTH]		= DSVResources[DSV_SCENE];
		SRVResources[SRV_SHADOW]	= DSVResources[DSV_SHADOW];

		SharedPointer<RResource> ppResources[] =
		{
			SRVResources[SRV_COLOR],
			SRVResources[SRV_DEPTH],
			SRVResources[SRV_NORMAL],
			SRVResources[SRV_SHADOW],
			SRVResources[SRV_GEOMETRY],
			0,
			SRVResources[SRV_COMPOSITE]
		};

		if ((Error = ShaderResourceViews.Initialize(ppResources, D3D12_DESCRIPTOR_HEAP_FLAG_NONE)))
		{
			return Error;
		}

		return S_OK;
	}

	ErrorCode CSceneComponents::CreateSceneUAV()
	{
		ErrorCode Error;

		if ((Error = UnorderedAccessViews.InitializeHeap(D3D12_DESCRIPTOR_HEAP_FLAG_NONE)))
		{
			return Error;
		}

		return S_OK;
	}

	ErrorCode CScene::InitializeLight()
	{
		ErrorCode Error;

		if ((Error = Light.Initialize(this)))
		{
			return Error;
		}

		return S_OK;
	}

	void CScene::UpdateConstants()
	{
		const ViewSetup & Setup = View->GetViewSetup();
		const Light::LightProperties & LightProperties = Light.GetProperties();

		ViewDynamicConstants * DynamicConstants = reinterpret_cast<ViewDynamicConstants*>(ConstantBuffer->GetBufferData().GetCPUVirtualAddress(0));
		{
			if (Properties.LightPerspective)
			{
				DynamicConstants->ViewMatrix					= LightProperties.LightViewMatrix;
				DynamicConstants->ViewProjectionMatrix			= LightProperties.LightViewProjectionMatrix;
				DynamicConstants->ViewProjectionInverseMatrix	= LightProperties.LightViewProjectionInverseMatrix;
				DynamicConstants->ViewInverseMatrix				= LightProperties.LightViewInverseMatrix;
				DynamicConstants->ScreenToTranslatedWorldMatrix = LightProperties.LightScreenToLightMatrix;
				DynamicConstants->ViewUp						= LightProperties.Up;
				DynamicConstants->ViewOrigin					= LightProperties.Origin;
				DynamicConstants->ViewRotation					= LightProperties.Directional.Direction.ToOrientationRotation();
			}
			else
			{
				DynamicConstants->ViewFocus						= Setup.ViewFocus;
				DynamicConstants->ViewMatrix					= Setup.ViewMatrix;
				DynamicConstants->ViewProjectionMatrix			= Setup.ViewProjectionMatrix;
				DynamicConstants->ViewRotation					= Setup.ViewRotation;
				DynamicConstants->ViewInverseMatrix				= Setup.ViewInverseMatrix;
				DynamicConstants->ViewProjectionInverseMatrix	= Setup.ViewProjectionInverseMatrix;
				DynamicConstants->ViewUp						= Setup.ViewUp;
				DynamicConstants->ViewOrigin					= Setup.ViewOrigin;
				DynamicConstants->ScreenToTranslatedWorldMatrix = Setup.ScreenToTranslatedWorldMatrix;
			}

			DynamicConstants->FrameTimeCounter					= Properties.FrameCount;
		}

		ViewFixedConstants * FixedConstants = reinterpret_cast<ViewFixedConstants*>(ConstantBuffer->GetBufferData().GetCPUVirtualAddress(1));
		{
			FixedConstants->ProjectionMatrix		= Setup.ProjectionMatrix;
			FixedConstants->ProjectionInverseMatrix = Setup.ProjectionInverseMatrix;
			FixedConstants->ZNear					= Properties.ClipNear;
			FixedConstants->ZFar					= Properties.ClipFar;
			FixedConstants->DeviceZToWorldTransform = Setup.DeviceZToWorld;
		}

		ViewLightConstants * LightConstants = reinterpret_cast<ViewLightConstants*>(ConstantBuffer->GetBufferData().GetCPUVirtualAddress(2));
		{
			LightConstants->Color					= LightProperties.DiffuseColor.ColorVector;
			LightConstants->Direction				= LightProperties.Directional.Direction;
			LightConstants->Position				= LightProperties.Position;
			LightConstants->Origin					= LightProperties.Origin;
			LightConstants->ShiftAngles				= LightProperties.ShiftAngles;
			LightConstants->Up						= LightProperties.Up;
			LightConstants->ProjectionMatrix		= LightProperties.LightProjectionMatrix;
			LightConstants->ViewMatrix				= LightProperties.LightViewMatrix;
			LightConstants->ViewProjectionMatrix	= LightProperties.LightViewProjectionMatrix;
		}
	}

	ErrorCode CScene::InitializeResources()
	{		
		return S_OK;
	}

	ErrorCode CScene::InitializeRenderer()
	{
		ErrorCode Error = Renderer->Create();

		if (Error)
		{
			return Error;
		}

		OutputScreen->WaitForGPU();

		return S_OK;
	}

	ErrorCode CScene::InitializeView(CSceneView * View)
	{
		View->SetSize(LongPoint
		(
			OutputScreen->GetWindow().WindowWidth,
			OutputScreen->GetWindow().WindowHeight)
		);

		UINT ShadowMapSize = Math::NextPowerOfTwo(OutputScreen->GetWindow().WindowWidth);

		View->SetShadowMapSize(LongPoint
		(
			ShadowMapSize,
			ShadowMapSize
		));

		ScreenViewport ShadowMapViewport;
		{
			ShadowMapViewport.Height			= ShadowMapViewport.ScissorRect.bottom	= ShadowMapSize;
			ShadowMapViewport.Width				= ShadowMapViewport.ScissorRect.right	= ShadowMapSize;
			ShadowMapViewport.ScissorRect.left	= ShadowMapViewport.ScissorRect.top		= 0;
			ShadowMapViewport.MaxDepth			= 1.0f;
			ShadowMapViewport.MinDepth			= 0.0f;
			ShadowMapViewport.TopLeftX			= 0.0f;
			ShadowMapViewport.TopLeftY			= 0.0f;
		}

		View->SetViewportShadowMap(ShadowMapViewport);
		View->SetViewport(OutputScreen->GetViewport());

		if (Camera.Get())
		{
			View->Update(Camera.Get());
		}

		return S_OK;
	}

	ErrorCode CScene::Initialize()
	{
		if (!OutputScreen)
		{
			return E_FAIL;
		}

		ErrorCode Error;

		if ((Error = InitializeView(View)))
		{
			return Error;
		}

		if ((Error = CreateSceneRTV(UintPoint(
			View->GetSize().X,
			View->GetSize().Y))))
		{
			return Error;
		}

		if ((Error = CreateSceneDSV(
			UintPoint(
				View->GetSize().X,
				View->GetSize().Y),
			UintPoint(
				View->GetSizeShadowMap().X,
				View->GetSizeShadowMap().Y))))
		{
			return Error;
		}

		if ((Error = CreateSceneSRV()))
		{
			return Error;
		}

		if ((Error = CreateSceneUAV()))
		{
			return Error;
		}

		if ((Error = CreateSceneCBV()))
		{
			return Error;
		}

		if ((Error = InitializeLight()))
		{
			return Error;
		}

		if ((Error = InitializeRenderer()))
		{
			return Error;
		}

		return S_OK;
	}

	void CScene::SwitchToGlobalLightPerspective()
	{
		Properties.LightPerspective = true;
	}

	void CScene::SwitchToPerspective(UINT Id)
	{
		auto Iter = Views.Find(Id);

		if (Iter)
		{
			View = Iter->Get();
		}
	}

	void CScene::SetInputPerspective(UINT Id)
	{
		auto Iter = Views.find(Id);

		if (Iter != Views.end())
		{
			ViewInput = (*Iter).second.Get();
		}
	}

	void CScene::SwitchToPerspective()
	{
		Properties.LightPerspective = false;
	}

	bool CScene::AddView(UINT Id, SharedPointer<CSceneView> & View)
	{
		if (!Views.insert(std::make_pair(Id, View)).second)
		{
			false;
		}

		InitializeView(View.Get());

		return true;
	}

	void CScene::RenderScene()
	{
		static constexpr UINT MAX_JITTER_STEPS = 8;

		Properties.FrameIndexLast		= Properties.FrameIndexCurrent;
		Properties.FrameIndexCurrent	= OutputScreen->GetBackBufferIndex();
		Properties.IsRendering			= true;

		RenderPass RenderPass
		(
			Properties.NextFrameRenderTimePoint = Time::UHighResolutionClock::Now(), 
			Properties.LastFrameRenderTimePoint, 
			Properties.NextShadowMapRenderTimePoint
		);

		UpdateView();
		UpdateConstants();
		{
			Renderer->Render(RenderPass);
		}

		Properties.FrameCount++;
		Properties.JitterIndex = (Properties.JitterIndex + 1) % MAX_JITTER_STEPS;

		if (RenderPass.RenderShadows)
		{
			Properties.NextShadowMapRenderTimePoint = Time::UHighResolutionClock::Now();
			Properties.NextShadowMapRenderTimePoint += Time::Milliseconds(Properties.ShadowMapRenderInterval);
		}

		Properties.LastFrameRenderTimePoint = Properties.NextFrameRenderTimePoint;
		Properties.IsRendering				= false;
	}

	ErrorCode CScene::LoadIndoor(const UINT AreaID)
	{
		return E_FAIL;
	}

	ErrorCode CScene::LoadOutdoor(const UINT AreaID)
	{
		ErrorCode Error;

		IntPoint TerrainSizeBefore;
		IntPoint TerrainSizeAfter;

		Area.Construct();
		{
			CSceneOutdoor & Outdoor = static_cast<CSceneOutdoor&>(*Area.Get());

			if (Outdoor.GetTerrain())
			{
				TerrainSizeBefore = Outdoor.GetTerrain()->GetProperties().TerrainSizeScaled;
			}

			CCommandListContext * CmdListCtxComposite	= Renderer->GetCompositeCommandContext();
			CCommandListContext * CmdListCtxCompute		= Renderer->GetComputeCommandContext();
			CCommandListContext * CmdListCtxGeometry	= Renderer->GetGeometryCommandContext();
			CCommandListContext * CmdListCtxShadows		= Renderer->GetShadowMapCommandContext();

			if ((Error = Outdoor.LoadMapOutdoor(AreaID)))
			{
				return Error;
			}

			Properties.FrameIndexCurrent = OutputScreen->GetBackBufferIndex();

			if ((Error = CmdListCtxGeometry->ResetCommandAllocator(Properties.FrameIndexCurrent)))
			{
				return Error;
			}

			if ((Error = CmdListCtxGeometry->ResetCommandList(Properties.FrameIndexCurrent)))
			{
				return Error;
			}

			if ((Error = Outdoor.SetupForScene(
				this, 
				CmdListCtxGeometry,
				CmdListCtxShadows,
				CmdListCtxCompute)))
			{
				return Error;
			}

			if ((Error = CmdListCtxGeometry->Close()))
			{
				return Error;
			}

			CmdListCtxGeometry->Finish(Properties.FrameIndexCurrent);

			TerrainSizeAfter = Outdoor.GetTerrain()->GetProperties().TerrainSizeScaled;

			if (TerrainSizeAfter != TerrainSizeBefore)
			{
				if ((Error = OcclusionMap.Create(TerrainSizeAfter)))
				{
					return Error;
				}

				D3D12_BUFFER_UAV OcclusionMapBufferDescUAV = {};
				{
					OcclusionMapBufferDescUAV.CounterOffsetInBytes	= 0;
					OcclusionMapBufferDescUAV.FirstElement			= 0;
					OcclusionMapBufferDescUAV.NumElements			= TerrainSizeAfter.Product();
					OcclusionMapBufferDescUAV.Flags					= D3D12_BUFFER_UAV_FLAG_NONE;
					OcclusionMapBufferDescUAV.StructureByteStride	= sizeof(Float);
				}

				if ((Error = UnorderedAccessViews.Initialize(OcclusionMap.GetBufferResource().Get(), DXGI_FORMAT_R32_FLOAT, UAV_OCCLUSION)))
				{
					return Error;
				}

				D3D12_BUFFER_SRV OcclusionMapBufferDescSRV = {};
				{
					OcclusionMapBufferDescSRV.FirstElement			= OcclusionMapBufferDescUAV.FirstElement;
					OcclusionMapBufferDescSRV.NumElements			= OcclusionMapBufferDescUAV.NumElements;
					OcclusionMapBufferDescSRV.Flags					= D3D12_BUFFER_SRV_FLAG_NONE;
					OcclusionMapBufferDescSRV.StructureByteStride	= sizeof(Float);
				}

				if ((Error = ShaderResourceViews.Initialize(OcclusionMap.GetBufferResource().Get(), DXGI_FORMAT_R32_FLOAT, SRV_OCCLUSION)))
				{
					return Error;
				}

				CCommandListContext * CmdListCtxComposite = Renderer->GetCompositeCommandContext();
				{
					RDescriptorHeap * DescriptorHeapScene = GetDescriptorHeapSRV();

					RDescriptorHeap::CopyInto<1, 1>
					(
						&DescriptorHeapRange(DescriptorHeapScene, SRV_OCCLUSION),
						&DescriptorHeapRange(CmdListCtxComposite->GetDescriptorHeapRange(DIDSceneSRV)[SRV_OCCLUSION])
					);
				}
			}

			Renderer->Update();
			Renderer->RenderOcclusion();
		}
	}

	ErrorCode CScene::LoadArea(const UINT AreaID)
	{
		ErrorCode Error;

		constexpr UINT OutdoorAreaIDMax = 10000;

		if (AreaID < OutdoorAreaIDMax)
		{
			return LoadOutdoor(AreaID);
		}
		else
		{
			return LoadIndoor(AreaID);
		}

		return S_OK;
	}

	void CScene::UpdateView()
	{
		if (ViewInput)
		{
			ViewInput->Update(Camera.Get());
		}
	}

	void CScene::Update()
	{
		OutputScreen->NextFrame();
	}

	ErrorCode CSceneConstants::CreateSceneCBV()
	{
		ErrorCode Error;

		ConstantBuffer.Construct(GrpConstantBufferDescriptor::Create<256,
			ViewDynamicConstants,
			ViewFixedConstants,
			ViewLightConstants>());

		DescriptorHeapCBV.Construct();

		if ((Error = DescriptorHeapCBV->Create_CBV_SRV_UAV(NumCBVDescriptors, D3D12_DESCRIPTOR_HEAP_FLAG_NONE)))
		{
			return Error;
		}

		if ((Error = ConstantBuffer->Create(DescriptorHeapCBV.Get())))
		{
			return Error;
		}

		return S_OK;
	}
}
