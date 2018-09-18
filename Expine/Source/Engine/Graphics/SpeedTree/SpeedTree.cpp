#include "Precompiled.h"

#include "SpeedTree/SpeedTree.h"
#include "Scene/SceneEnvironment.h"
#include "Resource/Texture/TextureManager.h"
#include "Process/ParallelProcessing.h"

namespace D3D
{
	ErrorCode CSpeedTree::CMaterialLoader::LoadMaterial(const SpeedTree::SRenderState * pRenderState, WindType WindType, LodType LodType, int32_t NumBillboards)
	{
		ErrorCode Error;

		auto TexturePathDiffuse			= pRenderState->m_apTextures[SpeedTree::TL_DIFFUSE];
		auto TexturePathDiffuseDetails	= pRenderState->m_apTextures[SpeedTree::TL_DETAIL_DIFFUSE];
		auto TexturePathNormal			= pRenderState->m_apTextures[SpeedTree::TL_NORMAL];
		auto TexturePathNormalDetails	= pRenderState->m_apTextures[SpeedTree::TL_DETAIL_NORMAL];
		auto TexturePathSpecular		= pRenderState->m_apTextures[SpeedTree::TL_SPECULAR_MASK];

		ShaderTextureInfo 
			TexInfoDiffuse,
			TexInfoDiffuseDetails,
			TexInfoNormal,
			TexInfoNormalDetails,
			TexInfoSpecular;

		KMaterial * Material = new KMaterial();

		if (!pRenderState->m_bBranchesPresent &&
			!pRenderState->m_bDiffuseAlphaMaskIsOpaque &&
			!pRenderState->m_bRigidMeshesPresent)
		{
			Material->TwoSided = !(pRenderState->m_bHorzBillboard || pRenderState->m_bVertBillboard);
			Material->BlendMode = BlendMasked;
			Material->CastShadowAsMasked = true;
		}

		if (pRenderState->m_bBranchesPresent &&
			pRenderState->m_eBranchSeamSmoothing == SpeedTree::EFFECT_OFF)
		{
			States[EnableBranchSeamSmoothing] = 1;
			Values[ValueSeamSmoothAmount] = pRenderState->m_fBranchSeamWeight;
		}

		if (TexturePathDiffuse)
		{
			if (Error = CTextureManager::Instance().GetOrAddTexture(
				String(TexturePathDiffuse),
				String(TexturePathDiffuse),
				TexInfoDiffuse))
			{
				return Error;
			}

			States[EnableDiffuseMap] = 1;
		}

		if (pRenderState->m_bBranchesPresent)
		{
			if (TexturePathDiffuseDetails)
			{
				if (Error = CTextureManager::Instance().GetOrAddTexture(
					String(TexturePathDiffuseDetails),
					String(TexturePathDiffuseDetails),
					TexInfoDiffuseDetails))
				{
					return Error;
				}

				States[EnableDetailMap] = 1;
			}
		}

		if (TexturePathSpecular)
		{
			if (Error = CTextureManager::Instance().GetOrAddTexture(
				String(TexturePathSpecular),
				String(TexturePathSpecular),
				TexInfoSpecular))
			{
				return Error;
			}

			States[EnableSpecularMap] = 1;
		}

		if (TexturePathNormal)
		{
			if (Error = CTextureManager::Instance().GetOrAddTexture(
				String(TexturePathNormal),
				String(TexturePathNormal),
				TexInfoNormal))
			{
				return Error;
			}

			States[EnableNormalMap] = 1;

			if (TexturePathNormalDetails)
			{
				if (Error = CTextureManager::Instance().GetOrAddTexture(
					String(TexturePathNormalDetails),
					String(TexturePathNormalDetails),
					TexInfoNormalDetails))
				{
					return Error;
				}
			}
		}

		GeometryType GeometryType;

		if (!pRenderState->m_bRigidMeshesPresent)
		{
			Values[ValueBillboardThreshold] = 0.9f - Math::Clamp(static_cast<float>(NumBillboards - 8) / 16.0f, 0.0f, 1.0f) * 0.8f;

			if (pRenderState->m_bBranchesPresent)
			{
				GeometryType = GeometryBranch;
			}
			else if (pRenderState->m_bFrondsPresent)
			{
				GeometryType = GeometryFrond;
			}
			else if (pRenderState->m_bLeavesPresent)
			{
				GeometryType = GeometryLeaf;
			}
			else if (pRenderState->m_bFacingLeavesPresent)
			{
				GeometryType = GeometryFacingLeaf;
			}
			else
			{
				GeometryType = GeometryBillboard;
			}

			States[EnableRigidMeshes] = 0;
		}

		if (Material->TwoSided)
		{
			States[EnableTwoSidedMaterials] = 1;
		}

		StaticMaterial = new SPTMaterial(0, Material, pRenderState, GeometryType, WindType);

		return S_OK;
	}

	void CSpeedTree::ProcessTriangleCorners(const int32_t TriangleIdx, const int32_t Corner, const SpeedTree::SDrawCall & DrawCall, const SpeedTree::SRenderState * RenderState, const uint16_t * pIndices16, const uint32_t * pIndices32, KMesh & Mesh, const int32_t Offset, const int32_t NumUVs)
	{
		Float4 SPTData;

		int32_t Index = TriangleIdx * 3 + Corner;
		int32_t VertexIdx = DrawCall.m_b32BitIndices ? pIndices32[Index] : pIndices16[Index];
		
		Mesh.WedgeIndices.push_back(VertexIdx + Offset);

		DrawCall.GetProperty(SpeedTree::VERTEX_PROPERTY_NORMAL, VertexIdx, SPTData);
		{
			const Vector3f& Normal = *Mesh.WedgeTangentZ.emplace(Mesh.WedgeTangentZ.end(), -SPTData[0], SPTData[1], SPTData[2]);
			
			DrawCall.GetProperty(SpeedTree::VERTEX_PROPERTY_TANGENT, VertexIdx, SPTData);
			{
				Mesh.WedgeTangentY.push_back(Normal ^ *Mesh.WedgeTangentX.emplace(Mesh.WedgeTangentX.end(), -SPTData[0], SPTData[1], SPTData[2]));
			}
		}

		DrawCall.GetProperty(SpeedTree::VERTEX_PROPERTY_AMBIENT_OCCLUSION, VertexIdx, SPTData);
		{
			Mesh.WedgeColors.emplace_back(SPTData[0], SPTData[0], SPTData[0], 1.0f);
		}

		int32_t BaseTexcoordIdx = Mesh.WedgeTexcoords[0].size();

		for (int32_t PadIdx = 0; PadIdx < NumUVs; ++PadIdx)
		{
			Mesh.WedgeTexcoords[PadIdx].AddUnitialized(1);
		}

		DrawCall.GetProperty(SpeedTree::VERTEX_PROPERTY_DIFFUSE_TEXCOORDS, VertexIdx, SPTData);
		{
			Mesh.WedgeTexcoords[0].front() = Vector2f(SPTData[0], SPTData[1]);
		}

		DrawCall.GetProperty(SpeedTree::VERTEX_PROPERTY_LIGHTMAP_TEXCOORDS, VertexIdx, SPTData);
		{
			Mesh.WedgeTexcoords[1].front() = Vector2f(SPTData[0], SPTData[1]);
		}

		DrawCall.GetProperty(SpeedTree::VERTEX_PROPERTY_WIND_BRANCH_DATA, VertexIdx, SPTData);
		{
			Mesh.WedgeTexcoords[2].front() = Vector2f(SPTData[0], SPTData[1]);
		}

		if (RenderState->m_bFacingLeavesPresent)
		{
			DrawCall.GetProperty(SpeedTree::VERTEX_PROPERTY_LEAF_CARD_LOD_SCALAR, VertexIdx, SPTData);
			{
				Mesh.WedgeTexcoords[3].front() = Vector2f(SPTData[0], SPTData[1]);
				Mesh.WedgeTexcoords[4].front() = Vector2f::ZeroVector;
			}
		}
		else
		{
			DrawCall.GetProperty(SpeedTree::VERTEX_PROPERTY_LOD_POSITION, VertexIdx, SPTData);
			{
				Mesh.WedgeTexcoords[3].front() = Vector2f(SPTData[0], SPTData[1]);
				Mesh.WedgeTexcoords[4].front() = Vector2f(SPTData[2], 0.0f);
			}
		}

		if (RenderState->m_bBranchesPresent)
		{
			DrawCall.GetProperty(SpeedTree::VERTEX_PROPERTY_DETAIL_TEXCOORDS, VertexIdx, SPTData);
			{
				Mesh.WedgeTexcoords[5].front() = Vector2f(SPTData[0], SPTData[1]);
			}

			DrawCall.GetProperty(SpeedTree::VERTEX_PROPERTY_BRANCH_SEAM_DIFFUSE, VertexIdx, SPTData);
			{
				Mesh.WedgeTexcoords[6].front()		= Vector2f(SPTData[0], SPTData[1]);
				Mesh.WedgeTexcoords[4].front().Y	= SPTData[2];
			}
		}
		else if (RenderState->m_bFrondsPresent)
		{
			DrawCall.GetProperty(SpeedTree::VERTEX_PROPERTY_WIND_EXTRA_DATA, VertexIdx, SPTData);
			{
				Mesh.WedgeTexcoords[5].front() = Vector2f(SPTData[0], SPTData[1]);
				Mesh.WedgeTexcoords[6].front() = Vector2f(SPTData[2], 0.0f);
			}
		}
		else if (
			RenderState->m_bLeavesPresent || 
			RenderState->m_bFacingLeavesPresent)
		{
			if (RenderState->m_bFacingLeavesPresent)
			{
				DrawCall.GetProperty(SpeedTree::VERTEX_PROPERTY_POSITION, VertexIdx, SPTData);
			}
			else
			{
				DrawCall.GetProperty(SpeedTree::VERTEX_PROPERTY_LEAF_ANCHOR_POINT, VertexIdx, SPTData);
			}

			Mesh.WedgeTexcoords[4].front().Y		= -SPTData[0];
			Mesh.WedgeTexcoords[5].front()			= Vector2f(SPTData[1], SPTData[2]);

			DrawCall.GetProperty(SpeedTree::VERTEX_PROPERTY_WIND_EXTRA_DATA, VertexIdx, SPTData);
			{
				Mesh.WedgeTexcoords[6].front()		= Vector2f(SPTData[0], SPTData[1]);
				Mesh.WedgeTexcoords[7].front().X	= SPTData[2];
			}

			DrawCall.GetProperty(SpeedTree::VERTEX_PROPERTY_WIND_FLAGS, VertexIdx, SPTData);
			{
				Mesh.WedgeTexcoords[6].front()		= Vector2f(SPTData[0], SPTData[1]);
				Mesh.WedgeTexcoords[7].front().Y	= SPTData[0];
			}
		}
	}

	ErrorCode CSpeedTree::LoadSPT(const String & FileName, bool bGrass, float ScaleFactor)
	{
		if (!Core.LoadTree(FileName.c_str(), bGrass, ScaleFactor))
		{
			return E_FAIL;
		}

		return S_OK;
	}

	ErrorCode CSpeedTree::LoadSPT(File::CFile * pFile, bool bGrass, float ScaleFactor)
	{
		Uint	ContentSize = pFile->GetContentRef().size();
		Byte *	Content		= pFile->GetContentRef().data();

		if (!Core.LoadTree(Content, ContentSize, false, bGrass, ScaleFactor))
		{
			return E_FAIL;
		}

		return S_OK;
	}

	ErrorCode CSpeedTree::CreateMesh(const String& Name, KMaterialStatic * ParentMaterial)
	{
		ErrorCode Error;

		const SpeedTree::SGeometry * Geometry = Core.GetGeometry();

		if (Geometry->m_nNumLods					== 0 && 
			Geometry->m_sVertBBs.m_nNumBillboards	== 0)
		{
			return E_FAIL;
		}
		
		KStaticMesh * ExistingMesh = CStaticMeshManager::Instance().FindMesh(Name);
		KStaticMesh * StaticMesh = new KStaticMesh();

		TVector<KMaterialStatic*> OldMaterials;

		if (ExistingMesh)
		{
			OldMaterials = ExistingMesh->StaticMaterials;

			for (auto & Material : OldMaterials)
			{
				Material->FreeComponents();
			}
		}

		WindType WindType;

		const SpeedTree::CWind & Wind = Core.GetWind();
		{
			if (Wind.IsOptionEnabled(SpeedTree::CWind::BRANCH_DIRECTIONAL_FROND_1))
			{
				WindType = Palm;
			}
			else if (Wind.IsOptionEnabled(SpeedTree::CWind::LEAF_TUMBLE_1))
			{
				WindType = Best;
			}
			else if (Wind.IsOptionEnabled(SpeedTree::CWind::BRANCH_SIMPLE_1))
			{
				WindType = Better;
			}
			else if (Wind.IsOptionEnabled(SpeedTree::CWind::LEAF_RIPPLE_VERTEX_NORMAL_1))
			{
				WindType = Fast;
			}
			else
			{
				WindType = Fastest;
			}
		}

		THashMap<int32_t, int32_t> RenderStateToMeshIndexMap;

		Float4 VertexPos;
		Float4 LeafCardCorners;

		for (int32_t LodIdx = 0; LodIdx < Geometry->m_nNumLods; ++LodIdx)
		{
			const SpeedTree::SLod & Lod = Geometry->m_pLods[LodIdx];

			KMesh * Mesh = new KMesh();

			int32_t NumUVs = 7;

			for (int32_t DrawCallIdx = 0; DrawCallIdx < Lod.m_nNumDrawCalls; ++DrawCallIdx)
			{
				const SpeedTree::SDrawCall & DrawCall		= Lod.m_pDrawCalls[DrawCallIdx];
				const SpeedTree::SRenderState * RenderState = DrawCall.m_pRenderState;

				if (RenderState->m_bLeavesPresent || 
					RenderState->m_bFacingLeavesPresent)
				{
					NumUVs = 8;
					break;
				}
			}

			for (int32_t DrawCallIdx = 0; DrawCallIdx < Lod.m_nNumDrawCalls; ++DrawCallIdx)
			{
				const SpeedTree::SDrawCall & DrawCall = Lod.m_pDrawCalls[DrawCallIdx];
				const SpeedTree::SRenderState * RenderState = DrawCall.m_pRenderState;

				int32_t * OldMaterialIdx = RenderStateToMeshIndexMap.Find(DrawCall.m_nRenderStateIndex);
				int32_t MaterialIdx;

				if (!OldMaterialIdx)
				{
					CMaterialLoader Loader;
					{
						if ((Error = Loader.LoadMaterial(RenderState, WindType, Lod.m_nNumDrawCalls == 1 ? LodType::LodSmooth : LodType::LodPop, Core.GetGeometry()->m_sVertBBs.m_nNumBillboards)))
						{
							return Error;
						}
					}

					MaterialIdx = StaticMesh->StaticMaterials.size();

					RenderStateToMeshIndexMap.insert(std::make_pair(
						DrawCall.m_nRenderStateIndex, MaterialIdx));

					StaticMesh->StaticMaterials.push_back(Loader.GetMaterial());
				}
				else
				{
					MaterialIdx = *OldMaterialIdx;
				}

				size_t VertexOffset = Mesh->VertexPositions.size();

				for (int32_t VertexIdx = 0; VertexIdx < DrawCall.m_nNumVertices; ++VertexIdx)
				{
					DrawCall.GetProperty(SpeedTree::VERTEX_PROPERTY_POSITION, VertexIdx, VertexPos);

					if (RenderState->m_bFacingLeavesPresent)
					{
						DrawCall.GetProperty(SpeedTree::VERTEX_PROPERTY_LEAF_CARD_CORNER, VertexIdx, LeafCardCorners);

						VertexPos[0] += LeafCardCorners[0];
						VertexPos[1] += LeafCardCorners[1];
						VertexPos[2] += LeafCardCorners[2];
					}

					Mesh->VertexPositions.push_back(Vector3f(-VertexPos[0], VertexPos[1], VertexPos[2]));
				}

				const Byte * pIndices = DrawCall.m_pIndexData;

				const Uint32 * pIndices32 = reinterpret_cast<const Uint32*>(pIndices);
				const Uint16 * pIndices16 = reinterpret_cast<const Uint16*>(pIndices);

				const Int32 NumTriangles = DrawCall.m_nNumIndices / 3;
				const Int32 NumTrianglesExisting = Mesh->WedgeIndices.size() / 3;

				Mesh->FaceMaterialIndices.ResizeUninitialized(NumTriangles);
				Mesh->FaceSmoothingMasks.ResizeUninitialized(NumTriangles);

				for (Int32 TriangleIdx = 0; TriangleIdx < NumTriangles; ++TriangleIdx)
				{
					Mesh->FaceMaterialIndices.push_back(MaterialIdx);
					Mesh->FaceSmoothingMasks.push_back(0);

					for (Int32 Corner = 0; Corner < 3; ++Corner)
					{
						ProcessTriangleCorners(TriangleIdx, Corner, DrawCall, RenderState, pIndices16, pIndices32, *Mesh, VertexOffset, NumUVs);
					}
				}

				KStaticMeshSourceModel * LodModel = &*StaticMesh->SourceModels.emplace(StaticMesh->SourceModels.end());
				{
					LodModel->BuildSettings.GenerateLightmapUVs				= false;
					LodModel->BuildSettings.RecomputeNormals				= false;
					LodModel->BuildSettings.RecomputeTangents				= false;
					LodModel->BuildSettings.RemoveDegenerates				= true;
					LodModel->BuildSettings.UseFullPrecisionUVs				= false;
					LodModel->BuildSettings.UseHighPrecisionTangentBasis	= false;
					LodModel->ScreenSize									= 0.1f / Math::Max(2.0f, static_cast<float>(StaticMesh->StaticMaterials.size()));
					LodModel->Mesh											= Mesh;

					CParallelProcessManager::Instance().AddProcessingUnit(new CMeshAttributeProcessor(LodModel));
				}

				const int32_t LodIdx = StaticMesh->SourceModels.size();

				for (int32_t MaterialIdx = 0; MaterialIdx < StaticMesh->StaticMaterials.size(); ++MaterialIdx)
				{
					KMeshSectionInfo & Info = StaticMesh->SectionInfoMap.Get(LodIdx, MaterialIdx);
					{
						Info.MaterialIdx = MaterialIdx;
					}
				}
			}

			if (Core.GetGeometry()->m_sVertBBs.m_nNumBillboards > 0)
			{

			}
		}

		return S_OK;
	}

	inline const char * CSpeedTree::GetLastError() const
	{
		return Core.GetError();
	}

	CSpeedTreeObject::CSpeedTreeObject(CSpeedTree * Spt, CSceneEnvironment * Environment) :
		Spt(Spt),
		Environment(Environment)
	{}

	bool CSpeedTreeObjectController::InsideFrustum(const ViewFrustum & Frustum) const
	{
		switch (Environment->GetAreaIntersectionState(GetCenterPosition()))
		{
			default:
			case AreaIntersectsFrustum:
			case AreaOutsideFrustum:
			{
				return false;
			}

			case AreaInsideFrustum:
			{
				return true;
			}
		}
	}

	bool CSpeedTreeObjectController::IntersectsFrustum(const ViewFrustum & Frustum) const
	{
		switch (Environment->GetAreaIntersectionState(GetCenterPosition()))
		{
			default: case AreaOutsideFrustum:
			{
				return false;
			}

			case AreaInsideFrustum:
			case AreaIntersectsFrustum:
			{
				return true;
			}
		}
	}

	void CSpeedTreeObject::Render(const CSceneRenderer * Renderer) const
	{
	}

	CSpeedTree::SPTMaterial::SPTMaterial(
				KMaterialStatic			* MaterialParent, 
				KMaterial				* Material, 
		const	SpeedTree::SRenderState * RenderState, 
		const	GeometryType			TypeGeometry, 
		const	WindType				TypeWind
	) : 
		KMaterialStatic(MaterialParent, Material)
		, TypeWind(TypeWind)
		, TypeGeometry(TypeGeometry)
		, RenderState(RenderState)
	{}
}
