#include "Precompiled.h"
#include "Utils/Database/SQLite.h"
#include "Scene/SceneRenderer.h"
#include "Scene/SceneOutdoor.h"

namespace D3D
{
	ErrorCode CSceneOutdoor::CreateTerrain()
	{
		ErrorCode Error;

		Terrain = new Terrain::CTerrain;

		if ((Error = Terrain->Create(CmdListCtx.Get(), CmdListCtxShadows.Get(), CmdListCtxCompute.Get(), Properties.TerrainProperties)))
		{
			return Error;
		}

		return S_OK;
	}

	ErrorCode CSceneOutdoor::CreateAtmosphere()
	{
		ErrorCode Error;

		Atmosphere = new Atmosphere::CAtmosphere();

		if ((Error = Atmosphere->Create(Properties.AtmosphereProperties)))
		{
			return Error;
		}

		return S_OK;
	}

	ErrorCode CSceneOutdoor::LoadMapOutdoor(const UINT MapID)
	{
		ErrorCode Error;

		SharedPointer<Database::SQLite::CDatabase> DB = Database::SQLite::CDatabaseManager::Instance().OpenDatabase(L"C:\\Users\\a\\Documents\\Navicat\\MySQL\\servers\\d\\Game.db");

		if (!DB)
		{
			return ERROR_DATABASE_DOES_NOT_EXIST;
		}

		AreaLoadParameters AreaLoadParameters = {};
		{
			AreaLoadParameters.AreaId = MapID;
		}

		if ((Error = CSceneArea::Load(AreaLoadParameters)))
		{
			return Error;
		}

		StringStream Query;
		{
			Query << "SELECT * FROM Maps WHERE ID = " << MapID;
		}

		Database::SQLite::DataSet
			ResultSetMap,
			ResultSetTerrain,
			ResultSetAtmosphere;

		if (DB->ExecutePrepared(Query.str(), ResultSetMap) != SQLITE_OK)
		{
			return ERROR_DS_DATABASE_ERROR;
		}

		Properties.TerrainExists	= FALSE;
		Properties.AtmosphereExists = FALSE;

		if (!ResultSetMap.empty())
		{
			if (ResultSetMap.GetRecordAs("Width",				Properties.TerrainProperties.TerrainSize.X)		&&
				ResultSetMap.GetRecordAs("Height",				Properties.TerrainProperties.TerrainSize.Y)		&&
				ResultSetMap.GetRecordAs("ScaleFactor",			Properties.TerrainProperties.ScaleFactor)		&&
				ResultSetMap.GetRecordAs("ScaleFactorHeight",	Properties.TerrainProperties.ScaleFactorHeight) &&
				ResultSetMap.GetRecordAs("TerrainID",			Properties.TerrainProperties.TerrainID))
			{
				Query = StringStream();
				{
					Query << "SELECT * FROM Terrain WHERE ID = " << Properties.TerrainProperties.TerrainID;
				}

				if (DB->ExecutePrepared(Query.str(), ResultSetTerrain) != SQLITE_OK)
				{
					return ERROR_DS_DATABASE_ERROR;
				}

				if (!ResultSetTerrain.empty())
				{
					String HeightMapFormat;

					if (ResultSetTerrain.GetRecordAs("TextureSet",		Properties.TerrainProperties.TextureSet)	&&
						ResultSetTerrain.GetRecordAs("TextureMap",		Properties.TerrainProperties.TextureMap)	&&
						ResultSetTerrain.GetRecordAs("HeightMap",		Properties.TerrainProperties.HeightMap)		&&
						ResultSetTerrain.GetRecordAs("Name",			Properties.TerrainProperties.Name)			&&
						ResultSetTerrain.GetRecordAs("HeightFormat",	HeightMapFormat))
					{
						if ((Properties.TerrainProperties.HeightDataType = Terrain::GetHeightMapFormat(HeightMapFormat)) != Terrain::UNKNOWN_FORMAT)
						{
							auto FileNotExists = [=](const WString& FilePath) -> bool
							{
								CErrorLog::Log<LogError>() << String::MakeString<String::Whitespace>("LoadMapOutdoor:", String(FilePath.begin(), FilePath.end()), "does not exist") << CErrorLog::EndLine;
								return false;
							};

							if (File::DoAllFilesExist(	FileNotExists,
														Properties.TerrainProperties.TextureMap,
														Properties.TerrainProperties.TextureSet,
														Properties.TerrainProperties.HeightMap))
							{
								Properties.TerrainExists =
									Properties.TerrainProperties.TerrainSize.X	> 0 &&
									Properties.TerrainProperties.TerrainSize.Y	> 0 &&
									Properties.TerrainProperties.ScaleFactor	> 0;

								if (Properties.TerrainExists)
								{
									ResultSetTerrain.GetRecordAs("ColorMap",	Properties.TerrainProperties.ColorMap);
									ResultSetTerrain.GetRecordAs("NormalMap",	Properties.TerrainProperties.NormalMap);

									Properties.TerrainProperties.TerrainSizeScaled = 
										Properties.TerrainProperties.TerrainSize * 
										Properties.TerrainProperties.ScaleFactor;
								}
							}
						}
					}
				}
			}

			if (ResultSetMap.GetRecordAs("AtmosphereID", Properties.AtmosphereProperties.AtmosphereID))
			{
				Query = StringStream();
				{
					Query << "SELECT * FROM Atmosphere WHERE ID = " << Properties.AtmosphereProperties.AtmosphereID;
				}

				if (DB->ExecutePrepared(Query.str(), ResultSetAtmosphere) != SQLITE_OK)
				{
					return ERROR_DS_DATABASE_ERROR;
				}

				if (!ResultSetAtmosphere.empty())
				{
					Properties.AtmosphereExists = TRUE;
				}
			}
		}

		return S_OK;
	}

	ErrorCode CSceneOutdoor::Load(const AreaLoadParameters & Parameters)
	{
		ErrorCode Error;

		const OutdoorLoadParameters & Params = static_cast<const OutdoorLoadParameters&>(Parameters);
		{
			if ((Error = LoadArea(Params.AreaId)))
			{
				return Error;
			}

			if ((Error = LoadMapOutdoor(Params.AreaId)))
			{
				return Error;
			}
		}
		
		return S_OK;
	}

	ErrorCode CSceneOutdoor::SetupForScene(const CScene * pScene, const CCommandListContext * pCommandListCtx, const CCommandListContext * pCommandListCtxShadows, const CCommandListContext * pCommandListCtxCompute)
	{
		CHECK_NULL_ARG(pScene);
		CHECK_NULL_ARG(pCommandListCtx);

		ErrorCode Error;

		CmdListCtx			= pCommandListCtx;
		CmdListCtxShadows	= pCommandListCtxShadows;
		CmdListCtxCompute	= pCommandListCtxCompute;

		if (Properties.TerrainExists)
		{
			if ((Error = CreateTerrain()))
			{
				return Error;
			}
		}
		else
		{
			Terrain = NULL;
		}

		if (Properties.AtmosphereExists)
		{
			if ((Error = CreateAtmosphere()))
			{
				return Error;
			}
		}
		else
		{
			Atmosphere = NULL;
		}

		return S_OK;
	}

	CSceneOutdoor::CRenderer::CRenderer(const CSceneRenderer * pSceneRenderer)
	{
		Scene			= pSceneRenderer->GetScene();
		SceneRenderer	= pSceneRenderer;
		SceneOutdoor	= dynamic_cast<const CSceneOutdoor*>(Scene->GetArea());
	}

	ErrorCode CSceneOutdoor::CRenderer::Create()
	{
		ErrorCode Error;

		if (SceneOutdoor->GetTerrain())
		{
			RendererTerrain = new Terrain::CTerrain::CRenderer(SceneRenderer.Get());

			if ((Error = RendererTerrain->Create()))
			{
				return Error;
			}
		}

		if (SceneOutdoor->GetAtmosphere())
		{
			RendererAtmosphere = new Atmosphere::CAtmosphere::CRenderer(SceneRenderer.Get());

			if ((Error = RendererAtmosphere->Create()))
			{
				return Error;
			}
		}

		return S_OK;
	}

	void CSceneOutdoor::CRenderer::Update()
	{
		if (RendererTerrain)
		{
			RendererTerrain->Update();
		}
		else if (SceneOutdoor->GetTerrain())
		{
			RendererTerrain = new Terrain::CTerrain::CRenderer(SceneRenderer.Get());
		}

		if (RendererAtmosphere)
		{
			RendererAtmosphere->Update();
		}
		else if (SceneOutdoor->GetAtmosphere())
		{
			RendererAtmosphere = new Atmosphere::CAtmosphere::CRenderer(SceneRenderer.Get());
			
			ThrowOnError(RendererAtmosphere->Create());
			ThrowOnError(RendererAtmosphere->Precompute());
		}
	}

	void CSceneOutdoor::CRenderer::Render()
	{
		if (RendererTerrain)
		{
			RendererTerrain->RenderGeometry();
		}
	}

	void CSceneOutdoor::CRenderer::RenderShadows()
	{
		if (RendererTerrain)
		{
			RendererTerrain->RenderShadowMap();
		}
	}

	void CSceneOutdoor::CRenderer::RenderFog()
	{
		if (RendererAtmosphere)
		{
			RendererAtmosphere->Render();
		}
	}

	void CSceneOutdoor::CRenderer::RenderOcclusionMap()
	{
		if (RendererTerrain)
		{
			RendererTerrain->RenderOcclusionMap();
		}
	}
}
