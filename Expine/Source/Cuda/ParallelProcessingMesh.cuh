#include <cuda.h>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <vector_types.h>

extern "C" void FComputeNormals(float3 * MeshNormals, float3 * MeshVertexPositions, int * Indices, int MeshFaceCount, int MeshVertexCount);