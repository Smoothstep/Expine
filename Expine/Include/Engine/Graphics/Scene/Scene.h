#pragma once

#include "SceneView.h"
#include "SceneLight.h"

#include "RawRenderTarget.h"
#include "RawShaderResourceView.h"
#include "RawDepthStencilView.h"

#include "SceneOutdoor.h"

#include "OcclusionMap.h"

#include "Time\Clock.h"

namespace D3D
{
	struct RenderPass
	{
		inline RenderPass
		(
			Time::TimePointSteady& RenderStartTimePoint,
			Time::TimePointSteady& RenderEndTimePoint,
			Time::TimePointSteady& RenderShadowMapTimePoint
		) :
			RenderEndTimePoint(RenderEndTimePoint),
			RenderStartTimePoint(RenderStartTimePoint),
			RenderShadowMapTimePoint(RenderShadowMapTimePoint)
		{
			RenderShadows = (RenderStartTimePoint - RenderShadowMapTimePoint) > 0;
		}

		Time::TimePointSteady& RenderStartTimePoint;
		Time::TimePointSteady& RenderEndTimePoint;
		Time::TimePointSteady& RenderShadowMapTimePoint;

		BOOL RenderShadows;
	};

	enum ESceneSRV
	{
		SRV_COLOR,
		SRV_DEPTH,
		SRV_NORMAL,
		SRV_SHADOW,
		SRV_GEOMETRY,
		SRV_OCCLUSION,
		SRV_COMPOSITE,
		NumSRVDescriptors
	};

	enum ESceneRTV
	{
		RTV_COLOR,
		RTV_NORMAL,
		RTV_GEOMETRY,
		RTV_COMPOSITE,
		NumRTVDescriptors
	};

	enum ESceneUAV
	{
		UAV_OCCLUSION,
		NumUAVDescriptors
	};

	enum ESceneDSV
	{
		DSV_SCENE,
		DSV_SHADOW,
		NumDSVDescriptors
	};

	enum ESceneConstants
	{
		CBV_DYNAMIC,
		CBV_FIXED,
		CBV_LIGHT,
		NumCBVDescriptors
	};

	class CScreen;
	class CSceneRenderer;
	class CSceneConstants
	{
	public:

		struct ViewFixedConstants
		{
			Matrix4x4 ProjectionMatrix;
			Matrix4x4 ProjectionInverseMatrix;

			Float ZNear;
			Float ZFar;	PADDING(2);

			Vector4f DeviceZToWorldTransform;
		};

		struct ViewLightConstants
		{
			Vector3f	Color;			PADDING(1);
			Vector3f	Position;		PADDING(1);
			Vector3f	Origin;			PADDING(1);
			Vector3f	Up;				PADDING(1);
			Vector3f	Direction;		PADDING(1);
			Vector2f	ShiftAngles;	PADDING(2);
			Matrix4x4	ViewMatrix;
			Matrix4x4	ProjectionMatrix;
			Matrix4x4	ViewProjectionMatrix;
		};

		struct ViewDynamicConstants
		{
			Vector3f ViewOrigin;	PADDING(1);
			Vector3f ViewFocus;		PADDING(1);
			Vector3f ViewUp;		PADDING(1);

			Rotation ViewRotation;	PADDING(1);

			Matrix4x4 ViewMatrix;
			Matrix4x4 ViewProjectionMatrix;
			Matrix4x4 ViewInverseMatrix;
			Matrix4x4 ViewProjectionInverseMatrix;
			Matrix4x4 ScreenToTranslatedWorldMatrix;
			Uint FrameTimeCounter;
		};

	protected:

		SharedPointer<CConstantBuffer> ConstantBuffer;

	protected:

		SharedPointer<RDescriptorHeap> DescriptorHeapCBV;

	public:

		inline const SharedPointer<CConstantBuffer> & GetConstantBuffer() const
		{
			return ConstantBuffer;
		}

		inline const SharedPointer<RDescriptorHeap> & GetDescriptorHeapCBV() const
		{
			return DescriptorHeapCBV;
		}

	public:

		ErrorCode CreateSceneCBV();
	};

	class CSceneComponents : public CSceneConstants
	{
	public:

		static constexpr DXGI_FORMAT ColorSceneFormat		= DXGI_FORMAT_R8G8B8A8_UNORM;
		static constexpr DXGI_FORMAT NormalSceneFormat		= DXGI_FORMAT_R8G8B8A8_UNORM;
		static constexpr DXGI_FORMAT CompositeSceneFormat	= DXGI_FORMAT_R8G8B8A8_UNORM;
		static constexpr DXGI_FORMAT GeometrySceneFormat	= DXGI_FORMAT_R8G8B8A8_UNORM;
		static constexpr DXGI_FORMAT ShadowSceneFormat		= DXGI_FORMAT_R8G8B8A8_UNORM;
		static constexpr DXGI_FORMAT DepthResourceFormat	= DXGI_FORMAT_R32G8X24_TYPELESS;
		static constexpr DXGI_FORMAT ShadowResourceFormat	= DXGI_FORMAT_R32_TYPELESS;
		static constexpr DXGI_FORMAT DepthFormat			= DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		static constexpr DXGI_FORMAT ShadowMapFormat		= DXGI_FORMAT_D32_FLOAT;
		static constexpr DXGI_FORMAT OcclusionMapFormat		= DXGI_FORMAT_R8_UINT;

	protected:

		RenderTargetViewGroup<NumRTVDescriptors>	RenderTargetViews;
		DepthStencilViewGroup<NumDSVDescriptors>	DepthStencilViews;
		ShaderResourceViewGroup<NumSRVDescriptors>	ShaderResourceViews;
		UnorderedAccessViewGroup<NumUAVDescriptors> UnorderedAccessViews;

	protected:

		RResource * RTVResources[NumRTVDescriptors];
		RResource * DSVResources[NumDSVDescriptors];
		RResource * SRVResources[NumSRVDescriptors];

	protected:

		ErrorCode CreateSceneDSV
		(
			const UintPoint SizeDepth,
			const UintPoint SizeShadowMap
		);

		ErrorCode CreateSceneRTV
		(
			const UintPoint Size
		);

		ErrorCode CreateSceneSRV();
		ErrorCode CreateSceneUAV();

	public:

		inline const RenderTargetViewGroup<NumRTVDescriptors>		& GetRenderTargetViews()		const	{	return RenderTargetViews;		}
		inline const ShaderResourceViewGroup<NumSRVDescriptors>		& GetShaderResourceViews()		const	{	return ShaderResourceViews;		}
		inline const DepthStencilViewGroup<NumDSVDescriptors>		& GetDepthStencilViews()		const	{	return DepthStencilViews;		}
		inline const UnorderedAccessViewGroup<NumUAVDescriptors>	& GetUnorderedAccessViews()		const	{	return UnorderedAccessViews;	}


		inline RDescriptorHeap * GetDescriptorHeapSRV() const	{	return ShaderResourceViews.DescriptorHeap.Get();	}
		inline RDescriptorHeap * GetDescriptorHeapRTV() const	{	return RenderTargetViews.DescriptorHeap.Get();		}
		inline RDescriptorHeap * GetDescriptorHeapDSV() const	{	return DepthStencilViews.DescriptorHeap.Get();		}
		inline RDescriptorHeap * GetDescriptorHeapUAV() const	{	return UnorderedAccessViews.DescriptorHeap.Get();	}


		inline const SharedPointer<RRenderTargetView>		& GetColorRTV()		const	{	return RenderTargetViews[RTV_COLOR];			}
		inline const SharedPointer<RRenderTargetView>		& GetNormalRTV()	const	{	return RenderTargetViews[RTV_NORMAL];			}
		inline const SharedPointer<RRenderTargetView>		& GetGeometryRTV()	const	{	return RenderTargetViews[RTV_GEOMETRY];			}
		inline const SharedPointer<RRenderTargetView>		& GetCompositeRTV() const	{	return RenderTargetViews[RTV_COMPOSITE];		}

		inline const SharedPointer<RShaderResourceView>		& GetColorSRV()		const	{	return ShaderResourceViews[SRV_COLOR];			}
		inline const SharedPointer<RShaderResourceView>		& GetNormalSRV()	const	{	return ShaderResourceViews[SRV_NORMAL];			}
		inline const SharedPointer<RShaderResourceView>		& GetDepthSRV()		const	{	return ShaderResourceViews[SRV_DEPTH];			}
		inline const SharedPointer<RShaderResourceView>		& GetShadowSRV()	const	{	return ShaderResourceViews[SRV_SHADOW];			}
		inline const SharedPointer<RShaderResourceView>		& GetCompositeSRV() const	{	return ShaderResourceViews[SRV_COMPOSITE];		}

		inline const SharedPointer<RDepthStencilView>		& GetDepthDSV()		const	{	return DepthStencilViews[DSV_SCENE];			}
		inline const SharedPointer<RDepthStencilView>		& GetShadowDSV()	const	{	return DepthStencilViews[DSV_SHADOW];			}

		inline const SharedPointer<RUnorderedAccessView>	& GetOcclusionUAV()	const	{	return UnorderedAccessViews[UAV_OCCLUSION];		}

	public:

		inline void SetAsShaderResource
		(
			const RGrpCommandList & CmdList
		)
		{
			RResource::SetResourceStates<NumRTVDescriptors>(RTVResources, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, CmdList, 0);
		}

		inline void SetAsRenderTargets
		(
			const RGrpCommandList & CmdList
		)
		{
			RResource::SetResourceStates<NumRTVDescriptors>(RTVResources, D3D12_RESOURCE_STATE_RENDER_TARGET, CmdList, 0);
		}

		inline void ClearScene
		(
			const RGrpCommandList & CmdList
		)
		{
			RenderTargetViews[RTV_COLOR]->Clear(CmdList, RGBAColor::Black.ColorArray);
			RenderTargetViews[RTV_NORMAL]->Clear(CmdList, RGBAColor::Black.ColorArray);
			RenderTargetViews[RTV_GEOMETRY]->Clear(CmdList, RGBAColor::Black.ColorArray);
			RenderTargetViews[RTV_COMPOSITE]->Clear(CmdList, RGBAColor::Black.ColorArray);
		}
	};

	struct SceneProperties
	{
		static constexpr Float DefaultFieldOfView		= 90.0f;
		static constexpr Float DefaultNearClip			= 1.0f;
		static constexpr Float DefaultFarClip			= 10000.0f;
		static constexpr INT64 DefaultSMRenderInterval	= 10000000;

		Float FieldOfView;
		Float ClipFar;
		Float ClipNear;

		UINT FrameCount			=  0;
		UINT FrameIndexCurrent	= -1;
		UINT FrameIndexLast		= -1;
		UINT SampleCount		=  1;
		UINT JitterIndex		=  0;

		bool LightPerspective = false;

		TAtomic<bool> IsRendering = false;

		INT64 ShadowMapRenderInterval = DefaultSMRenderInterval;

		Time::TimePointSteady NextShadowMapRenderTimePoint;
		Time::TimePointSteady LastShadowMapRenderTimePoint;

		Time::TimePointSteady NextFrameRenderTimePoint;
		Time::TimePointSteady LastFrameRenderTimePoint;

		inline bool IsMSAA() const
		{
			return SampleCount > 1;
		}

		inline bool IsFirstFrame() const
		{
			return FrameIndexCurrent >= FrameIndexLast;
		}

		SceneProperties() = default;
		SceneProperties(const Default &)
		{
			FieldOfView		= DefaultFieldOfView;
			ClipNear		= DefaultNearClip;
			ClipFar			= DefaultFarClip;
		}
	};

	class CScene : public CSceneComponents
	{
		friend class CSceneRenderer;

	private:

		ConstPointer<CScreen> OutputScreen;

	protected:

		TMap<UINT, SharedPointer<CSceneView> > Views;

		CSceneView*		View;
		CSceneView*		ViewInput;
		CSceneLight		Light;
		CSceneOcclusion	OcclusionMap;
		
		SharedPointer<CSceneRenderer>	Renderer;
		SharedPointer<CCamera>			Camera;
		SharedPointer<CSceneArea>		Area;

		SceneProperties Properties;

	public:

		inline const CSceneView & GetView() const
		{
			return *View;
		}

		inline const CSceneView & GetViewForInput() const
		{
			return *ViewInput;
		}

		inline const CSceneLight & GetLight() const
		{
			return Light;
		}

		inline const CSceneOcclusion & GetOcclusionMap() const
		{
			return OcclusionMap;
		}

		inline const CSceneArea * GetArea() const
		{
			return Area.Get();
		}

		inline const SceneProperties & GetProperties() const
		{
			return Properties;
		}

		inline CScreen * GetOutputScreen() const
		{
			return OutputScreen.Get();
		}

	public:

		inline void SetCamera
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

		inline const SharedPointer<CCamera> & GetCamera() const
		{
			return Camera;
		}

	public:

		CScene
		(
			const CScreen * pScreen
		);

	private:

		ErrorCode InitializeRenderer();
		ErrorCode InitializeView(CSceneView * View);
		ErrorCode InitializeResources();
		ErrorCode InitializeLight();

	protected:

		void Update();
		void UpdateView();
		void UpdateConstants();

		ErrorCode LoadOutdoor
		(
			const UINT AreaID
		);

		ErrorCode LoadIndoor
		(
			const UINT AreaID
		);

	public:

		ErrorCode Initialize();
		ErrorCode LoadArea
		(
			const UINT AreaID
		);

	public:

		void RenderScene();
		void SwitchToGlobalLightPerspective();
		void SwitchToPerspective();
		void SwitchToPerspective(UINT Id);
		void SetInputPerspective(UINT Id);

	public:

		bool AddView(UINT Id, SharedPointer<CSceneView> & View);
	};
}