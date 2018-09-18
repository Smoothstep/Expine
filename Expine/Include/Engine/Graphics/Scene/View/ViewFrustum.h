#pragma once

#include "DirectX/D3D.h"
#include "Draw/DrawInterface.h"

namespace D3D
{
	struct ViewFrustum
	{
		typedef Plane FrustumPermutedPlanes[8];

	private:

		static const Vector4f CubeCorner[8];

	private:

		enum EPlanes
		{
			PLANE_NEAR,
			PLANE_LEFT,
			PLANE_RIGHT,
			PLANE_TOP,
			PLANE_BOTTOM,
			PLANE_FAR
		};
		
	private:

		TVector<Plane> ViewPlanes;

	private:

		FrustumPermutedPlanes PermutedViewPlanes;

	private:

		void Initialize();

		bool IntersectsBoxWithPermutedPlanes
		(
			const M128 & BoxOrigin,
			const M128 & BoxExtent
		)	const;

		bool IntersectsSphere
		(
			const Vector3f & BoxOrigin,
			const float Radius,
			bool & bFullyContained
		)	const;

	public:

		bool Update
		(
			const Matrix4x4 &	WorldViewProjectionMatrix,
			const bool			bUseNearPlane	= false,
			const bool			bUseFarPlane	= false
		);

		void TransformCubeToFrustum
		(
			const Matrix4x4 & WorldViewProjectionMatrix
		);

		static void CreateWireframe
		(
			const Matrix4x4 & FrustumMatrix,
			CGeometryLine * Lines,
			CGeometryTriangle * Triangles
		);

		static void CreateWireframe
		(
			const float			Angle,
			const float			DistanceStart,
			const float			DistanceEnd,
			const float			AspectRatio,
			const Vector4f		& Color,
			const Vector4f		& ColorLines,
			const Matrix4x4		& FrustumMatrix,
			CGeometryLine		* Lines,
			CGeometryTriangle	* Triangles
		);

		bool IntersectsBox
		(
			const Vector3f & Origin,
			const Vector3f & Extent
		)	const;

		bool Contains
		(
			const Vector3f & P
		)	const;

		bool Intersects
		(
			const Vector3f & P
		)	const;

		inline bool Contains
		(
			const Vector2f & P
		)	const;

		inline bool Contains
		(
			const Vector2f & P0,
			const Vector2f & P1
		)	const;

		bool Intersects
		(
			const Vector2f & P0,
			const Vector2f & P1
		)	const;

		inline bool Intersects
		(
			const Vector2f & P
		)	const;

		inline bool Intersects
		(
			const Vector3f & P,
			const Float R
		)	const;

		inline bool Intersects
		(
			const Vector3f & Point1,
			const Vector3f & Point2
		)	const;
	};
}