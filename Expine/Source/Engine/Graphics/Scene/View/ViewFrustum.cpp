#include "Precompiled.h"

#include "Scene/View/ViewFrustum.h"

#include "Draw/DrawInterface.h"

namespace D3D
{
	const Vector4f ViewFrustum::CubeCorner[8] =
	{
		Vector4f(-1,-1,-1,+1),
		Vector4f(+1,-1,-1,+1),
		Vector4f(-1,+1,-1,+1),
		Vector4f(+1,+1,-1,+1),
		Vector4f(-1,-1,+1,+1),
		Vector4f(+1,-1,+1,+1),
		Vector4f(-1,+1,+1,+1),
		Vector4f(+1,+1,+1,+1)
	};

	void ViewFrustum::TransformCubeToFrustum(const Matrix4x4 & WorldViewProjectionInverseMatrix)
	{
		Vector4f Transformed[8];

#pragma unroll(8) 
		for (UINT N = 0; N < 8; ++N)
		{
			Transformed[N] = WorldViewProjectionInverseMatrix.TransformVector4f(CubeCorner[N]);
		}

		ViewPlanes[PLANE_NEAR] = Plane(
			Transformed[0].TransformToVector3(),
			Transformed[1].TransformToVector3(),
			Transformed[2].TransformToVector3());

		ViewPlanes[PLANE_FAR] = Plane(
			Transformed[5].TransformToVector3(),
			Transformed[6].TransformToVector3(),
			Transformed[7].TransformToVector3());

		ViewPlanes[PLANE_RIGHT] = Plane(
			Transformed[1].TransformToVector3(),
			Transformed[5].TransformToVector3(),
			Transformed[7].TransformToVector3());

		ViewPlanes[PLANE_LEFT] = Plane(
			Transformed[0].TransformToVector3(),
			Transformed[4].TransformToVector3(),
			Transformed[6].TransformToVector3());

		ViewPlanes[PLANE_BOTTOM] = Plane(
			Transformed[0].TransformToVector3(),
			Transformed[1].TransformToVector3(),
			Transformed[5].TransformToVector3());

		ViewPlanes[PLANE_TOP] = Plane(
			Transformed[0].TransformToVector3(),
			Transformed[1].TransformToVector3(),
			Transformed[5].TransformToVector3());

		Initialize();
	}

	void ViewFrustum::CreateWireframe(const Matrix4x4 & FrustumMatrix, CGeometryLine * Lines, CGeometryTriangle * Triangles)
	{
		SimpleColorVertex Vertices[2][2][2];

		for (uint32 Z = 0; Z < 2; Z++)
		{
			for (uint32 Y = 0; Y < 2; Y++)
			{
				for (uint32 X = 0; X < 2; X++)
				{
					Vector4f UnprojectedVertex = FrustumMatrix.TransformVector4f(
						Vector4f
						(
							(X ? -1.0f : 1.0f),
							(Y ? -1.0f : 1.0f),
							(Z ?  0.0f : 1.0f),
							1.0f
						)
					);

					if (UnprojectedVertex.W == 0)
					{
						Vertices[X][Y][Z].Position = Vector3f::ZeroVector;
					}
					else
					{
						Vertices[X][Y][Z].Position = Vector3f(UnprojectedVertex) / UnprojectedVertex.W;
					}

					Vertices[X][Y][Z].Color = Vector4f(1.0, 1.0, 1.0, 1.0);
				}
			}
		}

		Lines->AddLine(Vertices[0][0][0], Vertices[0][0][1]);
		Lines->AddLine(Vertices[1][0][0], Vertices[1][0][1]);
		Lines->AddLine(Vertices[0][1][0], Vertices[0][1][1]);
		Lines->AddLine(Vertices[1][1][0], Vertices[1][1][1]);

		Lines->AddLine(Vertices[0][0][0], Vertices[0][1][0]);
		Lines->AddLine(Vertices[1][0][0], Vertices[1][1][0]);
		Lines->AddLine(Vertices[0][0][1], Vertices[0][1][1]);
		Lines->AddLine(Vertices[1][0][1], Vertices[1][1][1]);

		Lines->AddLine(Vertices[0][0][0], Vertices[1][0][0]);
		Lines->AddLine(Vertices[0][1][0], Vertices[1][1][0]);
		Lines->AddLine(Vertices[0][0][1], Vertices[1][0][1]);
		Lines->AddLine(Vertices[0][1][1], Vertices[1][1][1]);
	}

	void ViewFrustum::CreateWireframe(const float Angle, const float DistanceStart, const float DistanceEnd, const float AspectRatio, const Vector4f & Color, const Vector4f & ColorLines, const Matrix4x4 & FrustumMatrix, CGeometryLine * Lines, CGeometryTriangle * Triangles)
	{
		Vector3f Direction(0, 0, 1);
		Vector3f LeftVector(1, 0, 0);
		Vector3f UpVector(0, 1, 0);

		SimpleColorVertex Verts[8];

		const float HozHalfAngleInRadians = Math::DegreesToRadians(Angle * 0.5f);

		float HozLength = 0.0f;
		float VertLength = 0.0f;

		if (Angle > 0.0f)
		{
			HozLength = DistanceStart * Math::Tan(HozHalfAngleInRadians);
			VertLength = HozLength / AspectRatio;
		}
		else
		{
			const float OrthoWidth = (Angle == 0.0f) ? 1000.0f : -Angle;
			HozLength = OrthoWidth * 0.5f;
			VertLength = HozLength / AspectRatio;
		}

		// near plane verts
		Verts[0].Position = (Direction * DistanceStart) + (UpVector * VertLength) + (LeftVector * HozLength);
		Verts[1].Position = (Direction * DistanceStart) + (UpVector * VertLength) - (LeftVector * HozLength);
		Verts[2].Position = (Direction * DistanceStart) - (UpVector * VertLength) - (LeftVector * HozLength);
		Verts[3].Position = (Direction * DistanceStart) - (UpVector * VertLength) + (LeftVector * HozLength);

		if (Angle > 0.0f)
		{
			HozLength = DistanceEnd * Math::Tan(HozHalfAngleInRadians);
			VertLength = HozLength / AspectRatio;
		}

		// far plane verts
		Verts[4].Position = (Direction * DistanceEnd) + (UpVector * VertLength) + (LeftVector * HozLength);
		Verts[5].Position = (Direction * DistanceEnd) + (UpVector * VertLength) - (LeftVector * HozLength);
		Verts[6].Position = (Direction * DistanceEnd) - (UpVector * VertLength) - (LeftVector * HozLength);
		Verts[7].Position = (Direction * DistanceEnd) - (UpVector * VertLength) + (LeftVector * HozLength);

		Verts[0].Position = Verts[1].Position = Verts[2].Position = Verts[3].Position = Vector4f(1000, 1000, 1000, 1);
		Verts[4].Position = Vector4f(0,		2000,	0, 1);
		Verts[5].Position = Vector4f(0,		0,		0, 1);
		Verts[6].Position = Vector4f(2000,	0,		0, 1);
		Verts[7].Position = Vector4f(2000,	2000,	0, 1);
		// 4 = Left Top 
		// 5 = Right Top
		// 6 = Right Bottom
		// 7 = Left Bottom

		// 0 - 4 - 5
		// 0 - 6 - 7
		// 0 - 4 - 7
		// 0 - 5 - 6
		// 4 - 5 - 6

		Uint16 Indices[] =
		{
			0, 4, 5,
			0, 4, 7,
			0, 5, 6,
			0, 6, 7,
			4, 5, 6,
			5, 6, 7
		};

		constexpr Int32 NumVertices = _countof(Verts);
		constexpr Int32 NumIndices = _countof(Indices);
		constexpr Int32 NumFaces = NumIndices / 3;

		Vector3f Normals[NumVertices];
		Vector3f NormalsFaces[NumFaces];
		int32_t Counter[NumVertices];

		std::fill(std::begin(Normals), std::end(Normals), 0);
		std::fill(std::begin(Counter), std::end(Counter), 0);

		for (Int32 I = 0; I < NumFaces; ++I)
		{
			Vector3f D;
			{
				Int32 I0 = Indices[(I) * 3 + 0];
				Int32 I1 = Indices[(I) * 3 + 1];
				Int32 I2 = Indices[(I) * 3 + 2];
				Vector3f P0 = Verts[I0].Position;
				Vector3f P1 = Verts[I1].Position;
				Vector3f P2 = Verts[I2].Position;

				Vector3f D0 = P0 - P1;
				Vector3f D1 = P2 - P1;

				D = (D0 ^ D1).GetSafeNormal();

				NormalsFaces[I] += D;
				Normals[I0] += D;
				Normals[I1] += D;
				Normals[I2] += D;
			}
		}

		for (int32 X = 0; X < 8; ++X)
		{
			Verts[X].Color = ColorLines;
			Verts[X].Position = Verts[X].Position;//FrustumMatrix.TransformPosition();
		}

		Lines->AddLine(Verts[0], Verts[1]);
		Lines->AddLine(Verts[1], Verts[2]);
		Lines->AddLine(Verts[2], Verts[3]);
		Lines->AddLine(Verts[3], Verts[0]);

		Lines->AddLine(Verts[4], Verts[5]);
		Lines->AddLine(Verts[5], Verts[6]);
		Lines->AddLine(Verts[6], Verts[7]);
		Lines->AddLine(Verts[7], Verts[4]);

		Lines->AddLine(Verts[0], Verts[4]);
		Lines->AddLine(Verts[1], Verts[5]);
		Lines->AddLine(Verts[2], Verts[6]);
		Lines->AddLine(Verts[3], Verts[7]);

		if (Triangles)
		{
			SimpleColorVertex VertsCopy[8];
			{
				for (size_t N = 0; N < 8; ++N)
				{
					VertsCopy[N].Color = Vector4f((Normals[N]).GetSafeNormal(), 1);
					VertsCopy[N].Position = Verts[N].Position;
				}
			}

			Triangles->Add(VertsCopy, Indices, _countof(VertsCopy), _countof(Indices));
		}
	}

	bool ViewFrustum::IntersectsBox(const Vector3f & Origin, const Vector3f & Extent) const
	{
		Vector3f Range = Extent / 2;
		Vector3f Center = Origin + Range;

		if (!IntersectsBoxWithPermutedPlanes(
			VectorLoadFloat3(&Center),
			VectorLoadFloat3(&Range)))
		{
			return false;
		}

		return true;
	}

	bool ViewFrustum::Contains(const Vector3f & P) const
	{
		for (UINT N = 0; N < 6; ++N)
		{
			float Distance = ViewPlanes[N].PlaneDot(P);

			if (Distance < 0)
			{
				return false;
			}
		}

		return true;
	}

	bool ViewFrustum::Intersects(const Vector3f & P) const
	{
		return Contains(P);
	}

	bool ViewFrustum::Contains(const Vector2f & P) const
	{
		if (P.X < ViewPlanes[PLANE_LEFT].X || P.X > ViewPlanes[PLANE_RIGHT].X)
		{
			return false;
		}

		if (P.Y < ViewPlanes[PLANE_LEFT].Y || P.Y > ViewPlanes[PLANE_RIGHT].Y)
		{
			return false;
		}

		return true;
	}

	bool ViewFrustum::Contains(const Vector2f & P0, const Vector2f & P1) const
	{
		if (P0.X < ViewPlanes[PLANE_LEFT].X || P1.X > ViewPlanes[PLANE_RIGHT].X)
		{
			return true;
		}

		if (P0.Y < ViewPlanes[PLANE_LEFT].Y || P1.Y > ViewPlanes[PLANE_RIGHT].Y)
		{
			return true;
		}

		return false;
	}

	bool ViewFrustum::Intersects(const Vector2f & P0, const Vector2f & P1) const
	{
		Vector3f V0(P0.X, P0.Y, 0.0);
		Vector3f V1(P1.X, P1.Y, 99999.0);

		return IntersectsBox(V0, V1);
	}

	bool ViewFrustum::Intersects(const Vector2f & P) const
	{
		if (P.X >= ViewPlanes[PLANE_LEFT].X || P.X <= ViewPlanes[PLANE_RIGHT].X)
		{
			if (P.Y >= ViewPlanes[PLANE_BOTTOM].Y || P.Y <= ViewPlanes[PLANE_TOP].Y)
			{
				return true;
			}
		}

		return false;
	}

	bool ViewFrustum::Intersects(const Vector3f & P, const Float R) const
	{
		for (UINT N = 0; N < 6; ++N)
		{
			if (ViewPlanes[N].PlaneDot(P) < -R)
			{
				return false;
			}
		}

		return true;
	}

	bool ViewFrustum::Intersects(const Vector3f & Point1, const Vector3f & Point2) const
	{
		for (UINT N = 0; N < 6; ++N)
		{
			Vector3f Intersection = Math::LinePlaneIntersection(Point1, Point2, ViewPlanes[N]);

			if (Vector3f::DotProduct(Point1, Intersection) < 0)
			{
				return false;
			}
		}

		return true;
	}

	void ViewFrustum::Initialize()
	{
		int32 NumToAdd		= ViewPlanes.size() / 4;
		int32 NumRemaining	= ViewPlanes.size() % 4;

		for (int32 Count = 0, Offset = 0; Count < NumToAdd; Count++, Offset += 4)
		{
			PermutedViewPlanes[0] = Plane(ViewPlanes[Offset + 0].X, ViewPlanes[Offset + 1].X, ViewPlanes[Offset + 2].X, ViewPlanes[Offset + 3].X);
			PermutedViewPlanes[1] = Plane(ViewPlanes[Offset + 0].Y, ViewPlanes[Offset + 1].Y, ViewPlanes[Offset + 2].Y, ViewPlanes[Offset + 3].Y);
			PermutedViewPlanes[2] = Plane(ViewPlanes[Offset + 0].Z, ViewPlanes[Offset + 1].Z, ViewPlanes[Offset + 2].Z, ViewPlanes[Offset + 3].Z);
			PermutedViewPlanes[3] = Plane(ViewPlanes[Offset + 0].W, ViewPlanes[Offset + 1].W, ViewPlanes[Offset + 2].W, ViewPlanes[Offset + 3].W);
		}

		Plane Last1, Last2, Last3, Last4;

		switch (NumRemaining)
		{
		case 3:
		{
			Last1 = ViewPlanes[NumToAdd * 4 + 0];
			Last2 = ViewPlanes[NumToAdd * 4 + 1];
			Last3 = ViewPlanes[NumToAdd * 4 + 2];
			Last4 = Last1;
			break;
		}
		case 2:
		{
			Last1 = ViewPlanes[NumToAdd * 4 + 0];
			Last2 = ViewPlanes[NumToAdd * 4 + 1];
			Last3 = Last4 = Last1;
			break;
		}
		case 1:
		{
			Last1 = ViewPlanes[NumToAdd * 4 + 0];
			Last2 = Last3 = Last4 = Last1;
			break;
		}
		default:
		{
			Last1 = Plane(0, 0, 0, 0);
			Last2 = Last3 = Last4 = Last1;
			break;
		}
		}

		PermutedViewPlanes[4] = Plane(Last1.X, Last2.X, Last3.X, Last4.X);
		PermutedViewPlanes[5] = Plane(Last1.Y, Last2.Y, Last3.Y, Last4.Y);
		PermutedViewPlanes[6] = Plane(Last1.Z, Last2.Z, Last3.Z, Last4.Z);
		PermutedViewPlanes[7] = Plane(Last1.W, Last2.W, Last3.W, Last4.W);
	}

	bool ViewFrustum::IntersectsBoxWithPermutedPlanes(const M128 & BoxOrigin, const M128 & BoxExtent) const
	{
		M128 OrigX = VectorReplicate(BoxOrigin, 0);
		M128 OrigY = VectorReplicate(BoxOrigin, 1);
		M128 OrigZ = VectorReplicate(BoxOrigin, 2);

		M128 ExtentX = VectorReplicate(BoxExtent, 0);
		M128 ExtentY = VectorReplicate(BoxExtent, 1);
		M128 ExtentZ = VectorReplicate(BoxExtent, 2);

		M128 AbsExt = VectorAbs(BoxExtent);
		M128 AbsExtentX = VectorReplicate(AbsExt, 0);
		M128 AbsExtentY = VectorReplicate(AbsExt, 1);
		M128 AbsExtentZ = VectorReplicate(AbsExt, 2);

		const Plane* RESTRICT PermutedPlanePtr = PermutedViewPlanes;

		for (int32 Count = 0, Num = 8; Count < Num; Count += 4)
		{
			M128 PlanesX = VectorLoadAligned(PermutedPlanePtr);
			PermutedPlanePtr++;
			M128 PlanesY = VectorLoadAligned(PermutedPlanePtr);
			PermutedPlanePtr++;
			M128 PlanesZ = VectorLoadAligned(PermutedPlanePtr);
			PermutedPlanePtr++;
			M128 PlanesW = VectorLoadAligned(PermutedPlanePtr);
			PermutedPlanePtr++;

			M128 DistX = VectorMultiply(OrigX, PlanesX);
			M128 DistY = VectorMultiplyAdd(OrigY, PlanesY, DistX);
			M128 DistZ = VectorMultiplyAdd(OrigZ, PlanesZ, DistY);
			M128 Distance = VectorSubtract(DistZ, PlanesW);

			M128 PushX = VectorMultiply(AbsExtentX, VectorAbs(PlanesX));
			M128 PushY = VectorMultiplyAdd(AbsExtentY, VectorAbs(PlanesY), PushX);
			M128 PushOut = VectorMultiplyAdd(AbsExtentZ, VectorAbs(PlanesZ), PushY);

			if (VectorAnyGreaterThan(Distance, PushOut))
			{
				return false;
			}
		}

		return true;
	}

	bool ViewFrustum::IntersectsSphere(const Vector3f & Origin, const float Radius, bool & bFullyContained) const
	{
		M128 Orig = VectorLoadFloat3(&Origin);
		M128 VRadius = VectorLoadFloat1(&Radius);
		M128 NegativeVRadius = VectorNegate(VRadius);

		M128 OrigX = VectorReplicate(Orig, 0);
		M128 OrigY = VectorReplicate(Orig, 1);
		M128 OrigZ = VectorReplicate(Orig, 2);

		const Plane * RESTRICT PermutedPlanePtr = PermutedViewPlanes;

		for (int32 Count = 0; Count < 8; Count += 4)
		{
			M128 PlanesX = VectorLoadAligned(PermutedPlanePtr);
			PermutedPlanePtr++;
			M128 PlanesY = VectorLoadAligned(PermutedPlanePtr);
			PermutedPlanePtr++;
			M128 PlanesZ = VectorLoadAligned(PermutedPlanePtr);
			PermutedPlanePtr++;
			M128 PlanesW = VectorLoadAligned(PermutedPlanePtr);
			PermutedPlanePtr++;

			M128 DistX = VectorMultiply(OrigX, PlanesX);
			M128 DistY = VectorMultiplyAdd(OrigY, PlanesY, DistX);
			M128 DistZ = VectorMultiplyAdd(OrigZ, PlanesZ, DistY);
			M128 Distance = VectorSubtract(DistZ, PlanesW);

			int32 Mask = VectorAnyGreaterThan(Distance, VRadius);

			if (Mask)
			{
				return false;
			}

			Mask = VectorAnyGreaterThan(Distance, NegativeVRadius);

			if (Mask)
			{
				bFullyContained = false;
			}
		}

		return true;
	}

	bool ViewFrustum::Update(const Matrix4x4 & ViewProjectionMatrix, const bool bUseNearPlane, const bool bUseFarPlane)
	{
		ViewPlanes.clear();
		ViewPlanes.reserve(6);

		Plane Temp;

		if (ViewProjectionMatrix.GetFrustumTopPlane(Temp))
		{
			ViewPlanes.push_back(Temp);
		}

		if (ViewProjectionMatrix.GetFrustumBottomPlane(Temp))
		{
			ViewPlanes.push_back(Temp);
		}

		if (ViewProjectionMatrix.GetFrustumRightPlane(Temp))
		{
			ViewPlanes.push_back(Temp);
		}

		if (ViewProjectionMatrix.GetFrustumLeftPlane(Temp))
		{
			ViewPlanes.push_back(Temp);
		}

		if (bUseNearPlane)
		{
			if (ViewProjectionMatrix.GetFrustumNearPlane(Temp))
			{
				ViewPlanes.push_back(Temp);
			}
		}

		if (bUseFarPlane)
		{
			if (ViewProjectionMatrix.GetFrustumFarPlane(Temp))
			{
				ViewPlanes.push_back(Temp);
			}
		}

		Initialize();

		return true;
	}
}
