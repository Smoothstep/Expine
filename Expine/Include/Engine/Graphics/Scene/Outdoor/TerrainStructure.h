#pragma once

#include "Raw/RawRenderTarget.h"
#include "TerrainHeight.h"
#include "Draw/Geometry.h"

#include "Raw/RawShaderResourceView.h"
#include "Raw/RawUnorderedAccessView.h"
#include "Raw/RawDepthStencilView.h"

#include "Resource/Texture/TextureMap.h"
#include "Resource/Texture/TextureResource.h"

#include <Utils/File/File.h>

#include <Utils/StringOp.h>

namespace D3D
{
	class CSceneView;

	namespace Terrain
	{
		class CStructure;
		class CTerrainTree;

		struct QueryResult;

		enum TerrainTextureTypes
		{
			TextureColor,
			TextureNormal,
			TextureParallax,
			NumTextureTypes
		};

		struct TerrainVertex
		{
			Vector3f Position;
			Vector2f Texture;
			Vector3f Normal;
			Vector3f Tangent;
			Vector3f Binormal;

			Uint8 AtlasIndices[NumTextureTypes][4];

			static inline void SetupTangents
			(
				TerrainVertex & V0,
				TerrainVertex & V1,
				TerrainVertex & V2
			)
			{
				Vector3f Delta0 = V1.Position - V0.Position;
				Vector3f Delta1 = V2.Position - V0.Position;

				Delta0.Normalize();
				Delta1.Normalize();

				Vector2f DeltaTexCoord0 = V1.Texture - V0.Texture;
				Vector2f DeltaTexCoord1 = V2.Texture - V0.Texture;

				DeltaTexCoord0.Normalize();
				DeltaTexCoord1.Normalize();

				float Det = 
					DeltaTexCoord0.X * DeltaTexCoord1.Y - 
					DeltaTexCoord1.X * DeltaTexCoord0.Y;

				if (Math::Abs(Det) < FLT_EPSILON)
				{
					V0.Tangent.X = 1.0f;
					V0.Tangent.Y = 0.0f;
					V0.Tangent.Z = 0.0f;
				}
				else
				{
					Det = 1.0f / Det;

					V0.Tangent = Vector3f
					(
						(DeltaTexCoord1.Y * Delta0.X - DeltaTexCoord0.Y * Delta1.X) * Det,
						(DeltaTexCoord1.Y * Delta0.Y - DeltaTexCoord0.Y * Delta1.Y) * Det,
						(DeltaTexCoord1.Y * Delta0.Z - DeltaTexCoord0.Y * Delta1.Z) * Det
					); 

					V0.Tangent.Normalize();
				}

				V1.Tangent = 
				V2.Tangent = 
				V0.Tangent;
			}
		};

		struct VertexMeshLayer
		{
			TerrainVertex * Vertices;

			inline void Allocate
			(
				UINT NumVertices
			)
			{
				Vertices = new TerrainVertex[NumVertices];
			}
		};

		struct MeshFace
		{
			Uint32 Index[4];
		};

		struct MeshVertexData
		{
			TerrainVertex * Vertices;

			UINT NumVerticesX;
			UINT NumVerticesZ;

			MeshVertexData
			(
				TerrainVertex *		VertexArray,
				const UINT			CountX,
				const UINT			CountZ
			);

			void CalculateVertices
			(
				const	MeshFace	*	pIndices,
						CHeight		*	pHeight,
				const	CTextureMap	*	pTextureMap,
				const	UINT			Offset,
				const	UINT			StartX,
				const	UINT			StartZ,
				const	UINT			VertexCountX, 
				const	UINT			VertexCountZ,
				const	UINT			TileSize
			);
		};

		struct MeshIndexData
		{
			MeshFace * Faces = NULL;

			int NodeCountX = 0;
			int NodeCountZ = 0;

			MeshIndexData
			(
				Uint32	*	IndexArray,
				const UINT	CountX,
				const UINT	CountZ
			);

			void CalculateIndicesForTree
			(
				CTerrainTree * pTree
			);

			void CalculateIndices
			(
				const UINT	Offset,
				const int	StartX,
				const int	StartZ,
				const int	NumVerticesX,
				const int	NumVerticesZ
			);
		};

		struct MeshConstruct
		{
			UniquePointer<CIGeometryBuffer<TerrainVertex, Uint32> > Buffers;

			ErrorCode CreateBuffers
			(
				const CCommandListContext	& CmdListCtx,
				const CStructure			* pStructure
			);
		};

		class CTerrainTree;
		class CStructure
		{
		public:

			struct InitializeParameters
			{
				static constexpr Int MinSizeX = 2;
				static constexpr Int MinSizeY = 2;

				IntPoint Size;
				IntPoint PatchSize;
				IntPoint PatchNodes;
				Int ScaleFactor;

				inline InitializeParameters
				(
					const IntPoint Size,
					const IntPoint PatchSize,
					const Int ScaleFactor
				) :
					Size(Size), 
					PatchSize(PatchSize), 
					ScaleFactor(ScaleFactor)
				{}

				inline bool ValidSize() const
				{
					if (Size.X < MinSizeX || 
						Size.Y < MinSizeY)
					{
						return false;
					}

					return true;
				}

				inline bool ValidScale() const
				{
					return (Size * ScaleFactor).Min() > 0;
				}

				inline bool ValidPatchSize() const
				{
					if (PatchSize.X > Size.X || 
						PatchSize.Y > Size.Y)
					{
						return false;
					}

					return true;
				}

				inline bool Valid() const
				{
					return 
						ValidSize() && 
						ValidScale() && 
						ValidPatchSize();
				}
			};

		private:

			IntPoint TerrainSize;
			IntPoint TerrainSizeActual;
			IntPoint TerrainPatchCount;
			IntPoint TerrainPatchSize;
			IntPoint TerrainPatchNodes;

			Int TerrainTileSize;
			Int TerrainTileOffset;

			UniquePointer<CTerrainTree> Root;

			TMutex Mutex;

		private:

			MeshConstruct TerrainMesh;

		private:

			ConstPointer<CHeight>		TerrainHeightMap;
			ConstPointer<CTextureMap>	TerrainTextureMap;

		public:

			inline IntPoint GetSize() const
			{
				return TerrainSize;
			}

			inline IntPoint GetActualSize() const
			{
				return TerrainSizeActual;
			}

			inline Int GetTileSize() const
			{
				return TerrainTileSize;
			}

			inline IntPoint GetPatchCount() const
			{
				return TerrainPatchCount;
			}

			inline IntPoint GetPatchSize() const
			{
				return TerrainPatchSize;
			}

			inline CTerrainTree * GetRootTree() const
			{
				return Root.Get();
			}

			inline void SetHeightMap
			(
				CHeight * pHeightMap
			)
			{
				TerrainHeightMap = pHeightMap;
			}

			inline void SetTextureMap
			(
				CTextureMap * pTextureMap
			)
			{
				TerrainTextureMap = pTextureMap;
			}

			inline CHeight * GetHeight() const
			{
				return TerrainHeightMap.Get();
			}

			inline CTextureMap * GetTextureMap() const
			{
				return TerrainTextureMap.Get();
			}

			inline const MeshConstruct & GetMesh() const
			{
				return TerrainMesh;
			}

			Uint GetTreeIndexByPosition
			(
				const IntPoint & Position
			)	const;

			Uint GetTreeOffsetByPosition
			(
				const IntPoint & Position
			)	const;

			Int IncrementTileOffset
			(
				const IntPoint & Position
			);

		public:

			ErrorCode Create
			(
				const CCommandListContext	& CmdListCtx,
				const InitializeParameters	& Parameters
			);
		};
	}
}