#include "ParallelProcessingMesh.cuh"
#include <helper_math.h>

#include "Cuda.h"

__global__ void ComputeNormals(float3 * MeshNormals, float3 * MeshVertexPositions, int * Indices, int MeshFaceCount, int MeshVertexCount)
{
	for (int I = 0; I < MeshFaceCount; ++I)
	{
		int I0 = 0;
		int I1 = 1;
		int I2 = 2;

		float3 P0 = MeshVertexPositions[I0];
		float3 P1 = MeshVertexPositions[I1];
		float3 P2 = MeshVertexPositions[I2];

		float3 D0 = P0 - P1;
		float3 D1 = P2 - P1;
	
		float3 D = normalize(cross(D0, D1));

		MeshNormals[I0] += D;
		MeshNormals[I1] += D;
		MeshNormals[I2] += D;
	}

	for (int I = 0; I < MeshVertexCount; ++I)
	{
		MeshNormals[I] = normalize(MeshNormals[I]);
	}
}

void FComputeNormals(float3 * MeshNormals, float3 * MeshVertexPositions, int * Indices, int MeshFaceCount, int MeshVertexCount)
{
	ComputeNormals CudaKernelArgs2(1, 1) (MeshNormals, MeshVertexPositions, Indices, MeshFaceCount, MeshVertexCount);
}