#pragma once

#include "Screen.h"
#include "Scene.h"
#include "SceneComposite.h"
#include "PostProcess.h"
#include "OcclusionMap.h"

#include <Routine\ServiceThread.h>

namespace D3D
{
	struct RenderOptions
	{
		BOOL UseAsyncCompute : 1;
	};

	enum EDescriptorIdents
	{
		DIDSceneCBV,
		DIDSceneSRV,
		DIDSceneUAV
	};

	class CSceneRenderer
	{
		enum ERenderParts
		{
			RENDER_OUTDOOR		= 1,
			RENDER_INDOOR		= 2,
			RENDER_GEOMETRY		= 3,
			RENDER_WIREFRAME	= 4,
			RENDER_FRUSTUM		= 5,
			RENDER_MODELS		= 6,
			RENDER_LIGHTS		= 7,
			RENDER_SHADOWS		= 8,
			RENDER_PART_NUM
		};

		enum ECommandLists
		{
			CMD_LIST_GEOMETRY,
			CMD_LIST_SHADOWS,
			CMD_LIST_COMPOSITE,
			CMD_LIST_PRESENT,
			CMD_LIST_COMPUTE,
			CMD_LIST_NUM
		};

		Thread::CServiceThread RenderThreads[CMD_LIST_NUM];

	private:

		TBitset<RENDER_PART_NUM> RenderParts;

	protected:

		SharedPointer<CDrawInterface> DrawInterface;
		SharedPointer<CSceneComposite> SceneComposite;

		UniquePointer<CSceneLight::CRenderer>		LightRenderer;
		UniquePointer<CSceneComposite::CRenderer>	CompositeRenderer;
		UniquePointer<CSceneOutdoor::CRenderer>		OutdoorRenderer;
		UniquePointer<PostProcess::CPostProcess>	PostProcess;

	private:

		CCommandListGroup<CMD_LIST_NUM>	RenderingCommandLists;

		CCommandListContext	CommandContext;
		CCommandListContext	CommandContextCompute;

	protected:

		ConstPointer<CScene> Scene;

	private:

		ErrorCode CreateDrawInterface();
		ErrorCode CreateRenderThreads();

	private:

		void RenderGeometry();
		void RenderShadowMap();
		void RenderOutdoor();
		void RenderViewFrustum();
		void RenderComposite();
		void RenderToBackBuffer();
		void RenderLight();
		void RenderOcclusionMap();

		ErrorCode InitializeCommandLists();
		ErrorCode InitializeOutdoorRenderer();
		ErrorCode InitializeIndoorRenderer();
		ErrorCode InitializeRenderParts();

		void ClearScene
		(
			const RGrpCommandList & CmdList
		);
		void EndScene();
		void BeginScene
		(
			const RenderPass& RenderPass
		);

	public:

		inline CCommandListContext * GetCommandListContext()
		{
			return &CommandContext;
		}

		inline CCommandListContext * GetGeometryCommandContext()
		{
			return &RenderingCommandLists[CMD_LIST_GEOMETRY];
		}

		inline CCommandListContext * GetShadowMapCommandContext()
		{
			return &RenderingCommandLists[CMD_LIST_SHADOWS];
		}

		inline CCommandListContext * GetCompositeCommandContext()
		{
			return &RenderingCommandLists[CMD_LIST_COMPOSITE];
		}

		inline CCommandListContext * GetComputeCommandContext() 
		{
			return &CommandContextCompute;
		}

		inline const CScene * GetScene() const
		{
			return Scene.Get();
		}

		CSceneRenderer
		(
			const CScene * pScene
		);

	public:

		ErrorCode Create();

	public:

		void Flush();
		void Update();
		void RenderOcclusion();
		void Render
		(
			const RenderPass& RenderPass
		);

	public:

		inline void EnableRenderFrustum()
		{
			RenderParts[RENDER_FRUSTUM] = 1;
		}

		inline void EnableRenderOutdoor()
		{
			RenderParts[RENDER_OUTDOOR] = 1;
		}
	};
}