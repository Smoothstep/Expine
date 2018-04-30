#pragma once

#include "Material.h"

namespace D3D
{
	struct KMesh
	{
		TVector<int32_t> FaceMaterialIndices;
		TVector<int32_t> FaceSmoothingMasks;
		TVector<int32_t> WedgeIndices;

		TVector<Vector3f> WedgeTangentX;
		TVector<Vector3f> WedgeTangentY;
		TVector<Vector3f> WedgeTangentZ;

		TVector<RGBAColor> WedgeColors;
		TArray<TVector<Vector2f>, 8> WedgeTexcoords;

		TVector<Vector3f> VertexPositions;
	};

	struct KBuildSettings
	{
		bool RecomputeNormals;
		bool RecomputeTangents;
		bool RemoveDegenerates;
		bool UseHighPrecisionTangentBasis;
		bool UseFullPrecisionUVs;
		bool GenerateLightmapUVs;
	};

	struct KStaticMeshSourceModel
	{
		KBuildSettings BuildSettings;
		float ScreenSize;
		KMesh * Mesh;
	};

	struct KMeshSectionInfo
	{
		int32_t MaterialIdx;
	};

	struct KSectionInfoMap
	{
	private:

		TMap<Uint32, KMeshSectionInfo> Map;

	private:

		static constexpr inline Uint32 CreateIndex(Int32 LodIdx, Int32 SectionIdx)
		{
			return ((LodIdx & 0xFFFF) << 16) | (SectionIdx & 0xFFFF);
		}

	public:

		void Set(Int32 LodIdx, Int32 SectionIdx, KMeshSectionInfo & Info)
		{
			Map[CreateIndex(LodIdx, SectionIdx)] = Info;
		}

		const KMeshSectionInfo & At(Int32 LodIdx, Int32 SectionIdx) const
		{
			return Map.at(CreateIndex(LodIdx, SectionIdx));
		}

		KMeshSectionInfo & Get(Int32 LodIdx, Int32 SectionIdx)
		{
			return Map.at(CreateIndex(LodIdx, SectionIdx));
		}
	};

	struct KStaticMesh
	{
		TVector<KMaterialStatic*> StaticMaterials;
		TVector<KStaticMeshSourceModel> SourceModels;
		KSectionInfoMap SectionInfoMap;
		int32_t LightMapResolution;
		int32_t LightMapCoordinateIdx;
	};

	class CStaticMeshManager : public CSingleton<CStaticMeshManager>
	{
	private:

		TMap<String, KStaticMesh*> MeshMap;

	public:

		inline KStaticMesh * FindMesh
		(
			const String& MeshId
		)
		{
			KStaticMesh ** Mesh = MeshMap.Find(MeshId);

			if (Mesh)
			{
				return *Mesh;
			}

			return nullptr;
		}

		inline void AddMesh
		(
			const String & Id,
			KStaticMesh * Mesh
		)
		{
			MeshMap.insert_or_assign(Id, Mesh);
		}

		template <class... Args>
		inline void AddMesh
		(
			const String & Id,
			const Args&&... Arguments
		)
		{
			MeshMap.insert_or_assign(Id, new KStaticMesh(std::forward<Args>(Arguments)...));
		}
	};
}