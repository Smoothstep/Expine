#include "Precompiled.h"

#include "Scene/Outdoor/TerrainStructure.h"
#include "Scene/Outdoor/TerrainQuadTree.h"

#include "Scene/SceneView.h"

#include "Pipeline/Pipelines.h"

namespace D3D
{
	namespace Terrain
	{
		Uint CStructure::GetTreeIndexByPosition(const IntPoint & Position) const
		{
			Int CountX = Math::Ceil(float(Position.X) / TerrainPatchSize.X);
			Int CountY = Math::Ceil(float(Position.Y) / TerrainPatchSize.Y);

			return CountX + CountY * TerrainPatchCount.X;
		}

		Uint CStructure::GetTreeOffsetByPosition(const IntPoint & Position) const
		{
			Int Offset;
			Int NumX = (Position.Y / TerrainPatchSize.Y) * (TerrainPatchSize.Y * TerrainSizeActual.X);
			Int NumY;

			if (Position.Y + TerrainPatchSize.Y > TerrainSizeActual.Y)
			{
				NumY = Position.X * (TerrainSizeActual.Y - Position.Y);
			}
			else
			{
				NumY = Position.X * TerrainPatchSize.Y;
			}

			Offset = NumY + NumX + TerrainPatchSize.X * (Position.Y % TerrainPatchSize.Y) * (Position.Y / TerrainPatchSize.Y);

			return Offset;
		}

		Int CStructure::IncrementTileOffset(const IntPoint & Size)
		{
			Mutex.lock();

			Int Ret = TerrainTileOffset;
			{
				TerrainTileOffset += Size.Product();
			}

			Mutex.unlock();

			return Ret;
		}

		ErrorCode CStructure::Create(const CCommandListContext & CmdListCtx, const InitializeParameters & Parameters)
		{
			ErrorCode Error;

			if (!Parameters.Valid())
			{
				return E_INVALIDARG;
			}

			TerrainTileSize = Parameters.ScaleFactor;

			IntPoint ScaledSize			= Parameters.Size		* TerrainTileSize;
			IntPoint ScaledSizePatch	= Parameters.PatchSize	* 1;

			TerrainSize			= ScaledSize;
			TerrainSizeActual	= Parameters.Size;
			TerrainPatchCount	= ScaledSize / ScaledSizePatch;
			TerrainPatchSize	= ScaledSizePatch;
			TerrainPatchNodes	= TerrainSize / TerrainPatchCount;

			Root = new CTerrainTree(this, CTerrainTree::InitializeParameters(IntPoint(0, 0), TerrainSizeActual));

			if (Error = TerrainMesh.CreateBuffers(CmdListCtx, this))
			{
				return Error;
			}

			return S_OK;
		}

		void MeshIndexData::CalculateIndicesForTree(CTerrainTree * pTree)
		{
			if (!pTree)
			{
				return;
			}

			if (pTree->GetPatch())
			{
				CalculateIndices
				(
					pTree->GetIndexLocation(),
					pTree->GetPosition().X,
					pTree->GetPosition().Y,
					pTree->GetSize().X,
					pTree->GetSize().Y
				);
			}
			else
			{
				CalculateIndicesForTree(pTree->GetSubTree(0));
				CalculateIndicesForTree(pTree->GetSubTree(1));
				CalculateIndicesForTree(pTree->GetSubTree(2));
				CalculateIndicesForTree(pTree->GetSubTree(3));
			}
		};

		ErrorCode MeshConstruct::CreateBuffers(const CCommandListContext & CmdListCtx, const CStructure * pStructure)
		{
			ErrorCode Error;

			const UINT TileSize = pStructure->GetTileSize();
			const IntPoint Size = pStructure->GetActualSize();

			Buffers = new CIGeometryBuffer<TerrainVertex, Uint32>();
			{
				D3D12_DRAW_INDEXED_ARGUMENTS Arguments;
				{
					Arguments.InstanceCount = 1;
					Arguments.BaseVertexLocation = 0;
					Arguments.StartInstanceLocation = 0;
				}

				int PatchesX = pStructure->GetPatchCount().X;
				int PatchesZ = pStructure->GetPatchCount().Y;

				int PatchSizeX = Size.X / PatchesX;
				int PatchSizeZ = Size.Y / PatchesZ;

				int LastX = Size.X % PatchesX;
				int LastZ = Size.Y % PatchesZ;

				auto & Vertices = Buffers->GetVertices();
				{
					Vertices.ResizeUninitialized(Size.X * Size.Y);
				}

				auto & Indices = Buffers->GetIndices();
				{
					Indices.ResizeNulled(Size.X * Size.Y * 4);
				}

				auto & Commands = Buffers->GetCommands();
				{
					Commands.ResizeUninitialized((PatchesX + (LastX ? 1 : 0)) * (PatchesZ + (LastZ ? 1 : 0)));
				}

				MeshIndexData IndexData(Indices.data(), Size.X, Size.Y);
				{
					IndexData.CalculateIndicesForTree(pStructure->GetRootTree());
				}

				MeshVertexData VertexData(Vertices.data(), Size.X, Size.Y);
				{
					VertexData.CalculateVertices
					(
						IndexData.Faces,
						pStructure->GetHeight(),
						pStructure->GetTextureMap(),
						0,
						0,
						0,
						Size.X,
						Size.Y,
						TileSize
					);
				}
			}

			if ((Error = Buffers->CreateBuffersDynamic()))
			{
				return Error;
			}

			Buffers->UploadData(CmdListCtx);

			return S_OK;
		}

		inline MeshIndexData::MeshIndexData(Uint32 * IndexArray, const UINT CountX, const UINT CountZ)
		{
			Faces = reinterpret_cast<MeshFace*>(IndexArray);

			NodeCountX = CountX;
			NodeCountZ = CountZ;
		}

		void MeshIndexData::CalculateIndices(const UINT Offset, const int StartX, const int StartZ, const int NumVerticesX, const int NumVerticesZ)
		{
			UINT O = Offset;

			UINT CountX = NodeCountX - 1;
			UINT CountZ = NodeCountZ - 1;

			for (int Z = 0; Z < NumVerticesZ - 1; Z++)
			{
				for (int X = 0; X < NumVerticesX - 1; X++)
				{
					Faces[O].Index[0] = StartX + X + (StartZ + Z) * NodeCountX;
					Faces[O].Index[1] = Faces[O].Index[0] + 1;
					Faces[O].Index[2] = Faces[O].Index[0] + NodeCountX;
					Faces[O].Index[3] = Faces[O].Index[2] + 1;

					O++;
				}

				O++;
			}

			int AddX = StartX + NumVerticesX < NodeCountX ? 1 : -1;
			int AddZ = StartZ + NumVerticesZ < NodeCountZ ? NodeCountX : -NodeCountX;

			O = Offset + (NumVerticesZ - 1) * NumVerticesX;

			for (int X = 0; X < NumVerticesX - 1; ++X)
			{
				Faces[O].Index[0] = (StartX + X) + (StartZ + NumVerticesZ - 1) * NodeCountX;
				Faces[O].Index[1] = Faces[O].Index[0] + 1;
				Faces[O].Index[2] = Faces[O].Index[0] + AddZ;
				Faces[O].Index[3] = Faces[O].Index[2] + 1;

				O++;
			}

			Faces[O].Index[0] = (StartX + NumVerticesX - 1) + (StartZ + NumVerticesZ - 1) * NodeCountX;;
			Faces[O].Index[1] = Faces[O].Index[0] + AddX;
			Faces[O].Index[2] = Faces[O].Index[0] + AddZ;
			Faces[O].Index[3] = Faces[O].Index[2] + AddX;

			O = Offset + NumVerticesX - 1;

			for (int Z = 0; Z < NumVerticesZ - 1; ++Z)
			{
				Faces[O].Index[0] = (StartX + NumVerticesX) + (StartZ + Z) * NodeCountX - 1;
				Faces[O].Index[1] = Faces[O].Index[0] + AddX;
				Faces[O].Index[2] = Faces[O].Index[0] + NodeCountX;
				Faces[O].Index[3] = Faces[O].Index[2] + AddX;

				O += NumVerticesX;
			}
		}

		MeshVertexData::MeshVertexData(TerrainVertex * VertexArray, const UINT CountX, const UINT CountZ)
		{
			NumVerticesX = CountX;
			NumVerticesZ = CountZ;

			Vertices = VertexArray;
		}

		inline void PackNormal(const Vector3f& Normal, Float& First, Float& Second)
		{
			Float16 F[4];

			F[0].Set(Normal.X);
			F[1].Set(Normal.Y);
			F[2].Set(Normal.Z);

			First  = *reinterpret_cast<Float*>(&F[0]);
			Second = *reinterpret_cast<Float*>(&F[2]);
		}

		inline void CalculateTangents(const Vector3f& P0, const Vector3f& P1, const Vector3f& P2, const Vector2f& T0, const Vector2f& T1, const Vector2f& T2, Vector3f& Tangent)
		{
			Vector3f Delta0 = P1 - P0;
			Vector3f Delta1 = P2 - P0;

			Delta0.Normalize();
			Delta1.Normalize();

			Vector2f DeltaTexCoord0 = T1 - T0;
			Vector2f DeltaTexCoord1 = T2 - T0;

			DeltaTexCoord0.Normalize();
			DeltaTexCoord1.Normalize();

			float Det =
				DeltaTexCoord0.X * DeltaTexCoord1.Y -
				DeltaTexCoord1.X * DeltaTexCoord0.Y;

			if (Math::Abs(Det) < FLT_EPSILON)
			{
				Tangent.X = 1.0f;
				Tangent.Y = 0.0f;
				Tangent.Z = 0.0f;
			}
			else
			{
				Det = 1.0f / Det;

				Tangent = Vector3f
				(
					(DeltaTexCoord1.Y * Delta0.X - DeltaTexCoord0.Y * Delta1.X) * Det,
					(DeltaTexCoord1.Y * Delta0.Y - DeltaTexCoord0.Y * Delta1.Y) * Det,
					(DeltaTexCoord1.Y * Delta0.Z - DeltaTexCoord0.Y * Delta1.Z) * Det
				);

				Tangent.Normalize();
			}
		}
	

		void MeshVertexData::CalculateVertices(const MeshFace * Indices, CHeight * Height, const CTextureMap * pTextureMap, const UINT Offset, const UINT StartX, const UINT StartZ, const UINT VertexCountX, const UINT VertexCountZ, const UINT TileSize)
		{
			float TextureU = 0.0f;
			float TextureV = 0.0f;

			UINT O = Offset;

			const auto & TextureSet = pTextureMap->GetTextureSet();
			const auto & TextureMap = pTextureMap->GetTextureMap();

			if (Height)
			{
				for (Int Z = 0; Z < VertexCountZ; ++Z)
				{
					for (Int X = 0; X < VertexCountX; ++X)
					{
						Vertices[O].Position = Vector3f
						(
							StartX + X * TileSize,
							StartZ + Z * TileSize,
							Height->GetActualHeight
							(
								StartX + X * TileSize,
								StartZ + Z * TileSize
							)
						);

						Vertices[O].Texture = Vector2f
						(
							TextureU,
							TextureV
						);

						const auto & Iter = TextureSet.at(TextureMap[O]);

						Vertices[O].AtlasIndices[TextureColor][0] = Iter.Offset;
						Vertices[O].AtlasIndices[TextureNormal][0] = Iter.OffsetNormal;
						Vertices[O].AtlasIndices[TextureParallax][0] = Iter.OffsetParallax;

						if (StartX + X + 1 < VertexCountX)
						{
							const auto & IterRight = TextureSet.at(TextureMap[O + 1]);

							Vertices[O].AtlasIndices[TextureColor][1] = IterRight.Offset;
							Vertices[O].AtlasIndices[TextureNormal][1] = IterRight.OffsetNormal;
							Vertices[O].AtlasIndices[TextureParallax][1] = IterRight.OffsetParallax;
						}
						else
						{
							Vertices[O].AtlasIndices[TextureColor][1] = -1;
							Vertices[O].AtlasIndices[TextureNormal][1] = -1;
							Vertices[O].AtlasIndices[TextureParallax][1] = -1;
						}

						if (StartZ + Z + 1 < VertexCountZ)
						{
							const auto & IterTop = TextureSet.at(TextureMap[O + VertexCountX]);

							Vertices[O].AtlasIndices[TextureColor][2] = IterTop.Offset;
							Vertices[O].AtlasIndices[TextureNormal][2] = IterTop.OffsetNormal;
							Vertices[O].AtlasIndices[TextureParallax][2] = IterTop.OffsetParallax;
						}
						else
						{
							Vertices[O].AtlasIndices[TextureColor][2] = -1;
							Vertices[O].AtlasIndices[TextureNormal][2] = -1;
							Vertices[O].AtlasIndices[TextureParallax][2] = -1;
						}

						if (StartZ + Z + 1 < VertexCountZ && 
							StartX + X + 1 < VertexCountX)
						{
							const auto & IterTopRight = TextureSet.at(TextureMap[O + VertexCountX + 1]);

							Vertices[O].AtlasIndices[TextureColor][3] = IterTopRight.Offset;
							Vertices[O].AtlasIndices[TextureNormal][3] = IterTopRight.OffsetNormal;
							Vertices[O].AtlasIndices[TextureParallax][3] = IterTopRight.OffsetParallax;
						}
						else
						{
							Vertices[O].AtlasIndices[TextureColor][3] = -1;
							Vertices[O].AtlasIndices[TextureNormal][3] = -1;
							Vertices[O].AtlasIndices[TextureParallax][3] = -1;
						}

						O++;

						TextureU += 1.0f;
					}

					TextureU  = 0.0f;
					TextureV += 1.0f;
				}
			}
			else
			{
				for (Int Z = 0; Z < VertexCountZ; ++Z)
				{
					for (Int X = 0; X < VertexCountX; ++X)
					{
						Vertices[O].Position = Vector3f
						(
							StartX + X * TileSize,
							StartZ + Z * TileSize,
							0
						);

						Vertices[O].Texture = Vector2f
						(
							TextureU,
							TextureV
						);

						TextureU = TextureU + 0.2f;

						O++;
					}

					TextureU = StartX;
					TextureV = TextureV += 1.0f;
				}
			}

			O = Offset;

#ifdef PER_VERTEX_NORMALS
			Uint NormalsX = NumVerticesX;
			Uint NormalsY = NumVerticesZ;
#else
			Uint NormalsX = Height->GetSizeX();
			Uint NormalsY = Height->GetSizeZ();
#endif

			Vector3f * Normals = new Vector3f[NormalsX * NormalsY];
			{
				std::fill(Normals, Normals + NormalsX * NormalsY, 0);
			}

			Uint GridX = NormalsX - 1;
			Uint GridY = NormalsY - 1;

#ifdef PER_VERTEX_NORMALS
			for (Uint Y = 0; Y < GridY; ++Y)
			{
				for (Uint X = 0; X < GridX; ++X)
				{
					const Vector3f& P0 = Vertices[(Y + 1) * NumVerticesX + X + 0].Position;
					const Vector3f& P1 = Vertices[(Y + 1) * NumVerticesX + X + 1].Position;
					const Vector3f& P2 = Vertices[(Y + 0) * NumVerticesX + X + 0].Position;

					const Vector3f D0 = P0 - P2;
					const Vector3f D1 = P2 - P1;

					Vector3f Normal = Vector3f::CrossProduct(D0, D1);
					{
						Normal.Normalize();
					}

					Normals[Y * GridX + X] = Normal;
				}
			}

			for (int Y = 0; Y < NormalsY; ++Y)
			{
				for (int X = 0; X < NormalsX; X++)
				{
					TerrainVertex::SetupTangents
					(
						Vertices[Indices[O].Index[2]],
						Vertices[Indices[O].Index[3]],
						Vertices[Indices[O].Index[0]]
					);

					TerrainVertex::SetupTangents
					(
						Vertices[Indices[O].Index[0]],
						Vertices[Indices[O].Index[3]],
						Vertices[Indices[O].Index[1]]
					);

					O++;
				}
			}
#else
			for (Uint Y = 0; Y < GridY; ++Y)
			{
				for (Uint X = 0; X < GridX; ++X)
				{
					const Vector3f V00 = Vector3f(0, 0, Height->GetActualHeight(X + 0, Y + 0));
					const Vector3f V01 = Vector3f(0, 1, Height->GetActualHeight(X + 0, Y + 1));
					const Vector3f V10 = Vector3f(1, 0, Height->GetActualHeight(X + 1, Y + 0));
					const Vector3f V11 = Vector3f(1, 1, Height->GetActualHeight(X + 1, Y + 1));

					const Vector3f FaceNormalL = ((V00 - V10) ^ (V10 - V11)).GetSafeNormal();
					const Vector3f FaceNormalR = ((V11 - V01) ^ (V01 - V00)).GetSafeNormal();

					Normals[(X + 1) + (Y + 0) * NormalsX] += FaceNormalL;
					Normals[(X + 0) + (Y + 1) * NormalsX] += FaceNormalR;
					Normals[(X + 0) + (Y + 0) * NormalsX] += FaceNormalL + FaceNormalR;
					Normals[(X + 1) + (Y + 1) * NormalsX] += FaceNormalL + FaceNormalR;
				}
			}
#endif

			O = Offset;

			for (int Y = 0; Y < NormalsY; ++Y)
			{
				for (int X = 0; X < NormalsX; X++)
				{
#ifdef PER_VERTEX_NORMALS
					Vector3f & vNormal = Vertices[O].Normal;

					if (X - 1 != -1 && Y - 1 != -1)
					{
						vNormal += Normals[(Y - 1) * GridX + X - 1];
					}

					if (X < GridX && Y - 1 != -1)
					{
						vNormal += Normals[(Y - 1) * GridX + X - 0];
					}

					if (Y < GridY && X - 1 != -1)
					{
						vNormal += Normals[(Y - 0) * GridX + X - 1];
					}

					if (X < GridX && Y < GridY)
					{
						vNormal += Normals[(Y - 0) * GridX + X - 0];
					}

					vNormal.Normalize();

					Vertices[O].Binormal = Vector3f::CrossProduct
					(
						Vertices[O].Tangent, 
						Vertices[O].Normal
					);

#else
					Normals[O].Normalize();
#endif
					O++;
				}
			}

#ifndef PER_VERTEX_NORMALS
			Height->SetNormalMap(Normals);
#else
			SafeReleaseArray(Normals);
#endif
		}
	}
}
