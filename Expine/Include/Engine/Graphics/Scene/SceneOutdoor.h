#pragma once

#include "Scene/Outdoor/Terrain.h"
#include "Scene/Outdoor/Atmosphere.h"
#include "Scene/SceneArea.h"

namespace D3D
{
	struct OutdoorProperties
	{
		BOOL								TerrainExists;
		Terrain::TerrainProperties			TerrainProperties;

		BOOL								AtmosphereExists;
		Atmosphere::AtmosphereProperties	AtmosphereProperties;
	};

	struct OutdoorLoadParameters : public AreaLoadParameters
	{
		inline bool Valid() const
		{
			return AreaLoadParameters::Valid();
		}

		UINT MaxTerrainInnerNodes;
	};

	class CScene;
	class CSceneRenderer;
	class CSceneOutdoor : public CSceneArea
	{
	public:

		class CRenderer
		{
		private:

			ConstPointer<CScene>								Scene;
			ConstPointer<CSceneRenderer>						SceneRenderer;
			ConstPointer<CSceneOutdoor>							SceneOutdoor;

			UniquePointer<Terrain::CTerrain::CRenderer>			RendererTerrain;
			UniquePointer<Atmosphere::CAtmosphere::CRenderer>	RendererAtmosphere;

		public:

			ErrorCode Create();

		public:

			CRenderer
			(
				const CSceneRenderer * pSceneRenderer
			);

			void Update();

			void Render();
			void RenderShadows();
			void RenderFog();
			void RenderOcclusionMap();
		};

	private:

		UniquePointer<Terrain::CTerrain> Terrain;
		UniquePointer<Atmosphere::CAtmosphere> Atmosphere;

	private:

		OutdoorProperties Properties;

	private:

		ConstPointer<CCommandListContext> CmdListCtx;
		ConstPointer<CCommandListContext> CmdListCtxShadows;
		ConstPointer<CCommandListContext> CmdListCtxCompute;

	public:

		inline Terrain::CTerrain * GetTerrain() const
		{
			return Terrain.Get();
		}

		inline Atmosphere::CAtmosphere * GetAtmosphere() const
		{
			return Atmosphere.Get();
		}

		inline virtual EAreaType GetAreaType() const override
		{
			return AreaOutdoor;
		}

	private:

		ErrorCode CreateTerrain();
		ErrorCode CreateAtmosphere();

	public:

		ErrorCode LoadMapOutdoor
		(
			const UINT MapID
		);

		virtual ErrorCode Load
		(
			const AreaLoadParameters & Parameters
		) override;

		ErrorCode SetupForScene
		(
			const CScene				* pScene,
			const CCommandListContext	* pCommandListCtx,
			const CCommandListContext	* pCommandListCtxShadows,
			const CCommandListContext	* pCommandListCtxCompute
		);
	};
}