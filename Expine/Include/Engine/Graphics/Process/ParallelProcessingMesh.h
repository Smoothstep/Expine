#pragma once

#include "ParallelProcessingUnit.h"
#include "Object/Mesh.h"

namespace D3D
{
	class CMeshAttributeProcessor : public IParallelProcessingUnit
	{
	private:

		KStaticMeshSourceModel * Source;

	private:

		void GPUCalculateN()
		{

		}

		void CalculateN()
		{
			Vector3f *	MeshNormals			= Source->Mesh->WedgeTangentZ.data();
			Int32	 *	MeshIndices			= Source->Mesh->WedgeIndices.data();
			Int32		MeshFaceCount		= Source->Mesh->WedgeIndices.size() / 3;
			Vector3f *	MeshVertexPositions = Source->Mesh->VertexPositions.data();
			Int32		MeshVertexCount		= Source->Mesh->VertexPositions.size();

			Source->Mesh->WedgeTangentZ.resize(MeshVertexCount);

			for (Int32 I = 0; I < MeshFaceCount; ++I)
			{
				Int32 I0 = MeshIndices[I * 3 + 0];
				Int32 I1 = MeshIndices[I * 3 + 1];
				Int32 I2 = MeshIndices[I * 3 + 2];

				const Vector3f & P0 = MeshVertexPositions[I0];
				const Vector3f & P1 = MeshVertexPositions[I1];
				const Vector3f & P2 = MeshVertexPositions[I2];

				Vector3f D0 = P0 - P1;
				Vector3f D1 = P2 - P1;

				Vector3f D = (D0 ^ D1).GetSafeNormal();

				MeshNormals[I0] += D;
				MeshNormals[I1] += D;
				MeshNormals[I2] += D;
			}

			for (Int32 I = 0; I < MeshVertexCount; ++I)
			{
				MeshNormals[I].Normalize();
			}
		}

		void CalculateB()
		{

		}

		void CalculateT()
		{

		}

		void CalculateTB()
		{

		}

		void CalculateNB()
		{

		}

		void CalculateTBN()
		{

			Vector3f *	MeshNormals			= Source->Mesh->WedgeTangentZ.data();
			Vector3f *	MeshTangents		= Source->Mesh->WedgeTangentX.data();
			Vector3f *	MeshBinormals		= Source->Mesh->WedgeTangentY.data();
			Int32	 *	MeshIndices			= Source->Mesh->WedgeIndices.data();
			Int32		MeshFaceCount		= Source->Mesh->WedgeIndices.size() / 3;
			Vector3f *	MeshVertexPositions = Source->Mesh->VertexPositions.data();
			Vector2f *	MeshTexCoords		= Source->Mesh->WedgeTexcoords.front().data();
			Int32		MeshVertexCount		= Source->Mesh->VertexPositions.size();

			Source->Mesh->WedgeTangentX.resize(MeshVertexCount);
			Source->Mesh->WedgeTangentY.resize(MeshVertexCount);
			Source->Mesh->WedgeTangentZ.resize(MeshVertexCount);

			for (Int32 I = 0; I < MeshFaceCount; ++I)
			{
				const Int32 Tri = I * 3;

				const Int32 I0 = MeshIndices[Tri + 0];
				const Int32 I1 = MeshIndices[Tri + 1];
				const Int32 I2 = MeshIndices[Tri + 2];

				const Vector3f & P0 = MeshVertexPositions[I0];
				const Vector3f & P1 = MeshVertexPositions[I1];
				const Vector3f & P2 = MeshVertexPositions[I2];

				Vector3f D0 = P0 - P1;
				Vector3f D1 = P2 - P1;

				Vector3f D = (D0 ^ D1).GetSafeNormal();

				MeshNormals[I0] += D;
				MeshNormals[I1] += D;
				MeshNormals[I2] += D;

				D0.Normalize();
				D1.Normalize();

				const Vector2f & T0 = MeshTexCoords[I0];
				const Vector2f & T1 = MeshTexCoords[I1];
				const Vector2f & T2 = MeshTexCoords[I2];

				Vector2f DeltaTexCoord0 = T0 - T1;
				Vector2f DeltaTexCoord1 = T2 - T1;

				float Det =
					DeltaTexCoord0.X * DeltaTexCoord1.Y -
					DeltaTexCoord1.X * DeltaTexCoord0.Y;

				if (Math::Abs(Det) < FLT_EPSILON)
				{
					MeshTangents[Tri] = Vector3f(1, 0, 0);
				}
				else
				{
					Det = 1.0f / Det;

					MeshTangents[Tri] = Vector3f
					(
						(DeltaTexCoord1.Y * D0.X - DeltaTexCoord0.Y * D1.X) * Det,
						(DeltaTexCoord1.Y * D0.Y - DeltaTexCoord0.Y * D1.Y) * Det,
						(DeltaTexCoord1.Y * D0.Z - DeltaTexCoord0.Y * D1.Z) * Det
					);

					MeshTangents[Tri].Normalize();
				}

				MeshTangents[Tri + 2] =
				MeshTangents[Tri + 1] =
				MeshTangents[Tri + 0];
			}

			for (Int32 I = 0; I < MeshVertexCount; ++I)
			{
				MeshNormals[I].Normalize();
				MeshBinormals[I] = MeshTangents[I] ^ MeshNormals[I];
			}
		}

	public:

		CMeshAttributeProcessor(KStaticMeshSourceModel * StaticMeshSourceModel)
		{
			Source = StaticMeshSourceModel;
		}

		virtual int32_t GetPriority() const
		{
			return Source->Mesh->VertexPositions.size();
		}

		virtual int32_t GetIterationCount() const
		{
			return 1;
		}

		virtual void Process(int32_t N) override
		{
			if (Source->BuildSettings.GenerateLightmapUVs)
			{

			}

			if (Source->BuildSettings.RecomputeNormals)
			{
				if (Source->BuildSettings.RecomputeTangents)
				{
					CalculateTBN();
				}
				else
				{
					CalculateNB();
				}
			}
			else if (Source->BuildSettings.RecomputeTangents)
			{
				CalculateTB();
			}
		}

	};
}