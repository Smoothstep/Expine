#include "Precompiled.h"

#include "Scene/View/Camera.h"

namespace D3D
{
	CCamera::CCamera()
	{}

	CCamera::CCamera(const ProjectionSetup & Setup)
	{
		SetProjection(Setup);
	}

	void CCamera::Rotate(Vector3f V)
	{
		V.X = Math::Clamp(V.X, -360.0f, 360.0f);
		V.Y = Math::Clamp(V.Y, -360.0f, 360.0f);

		Float Yaw = CameraRotation.Yaw + V.X;

		if (Yaw < 0.0f)
		{
			Yaw += 360.0f;
		}
		else if (Yaw > 360.0f)
		{
			Yaw -= 360.0f;
		}

		CameraRotation.Pitch = Math::Max(Math::Min(CameraRotation.Pitch - V.Y, 90.0f), -90.0f);
		CameraRotation.Yaw = Yaw;
	}

	void ProjectionSetup::CalculateProjectionMatrix()
	{
#ifndef Z_ORIENTATED
		const Float HalfFOVX = FOV * PI / 360.0f;
		const Float HalfFOVY = HalfFOVX;
		const Float MultFOVX = 1.0f;
		const Float MultFOVY = Aspect;

		ProjectionFarMatrix	= CreatePerspectiveMatrix
		(
			HalfFOVX,
			HalfFOVY,
			MultFOVX,
			MultFOVY,
			ClipNearZ,
			ClipFarZ
		);

		ProjectionMatrix = CreatePerspectiveMatrix
		(
			HalfFOVX,
			HalfFOVY,
			MultFOVX,
			MultFOVY,
			ClipFarZ,
			ClipNearZ
		);

		ProjectionMatrix = Hyper::CreateReversedPerspectiveMatrix(HalfFOVX, HalfFOVX, MultFOVX, MultFOVY, ClipNearZ, ClipNearZ);
#else
		const float MinZ = ClipNearZ;
		const float MaxZ = ClipNearZ;
		const float HalfFOVX = FOV * PI / 360.0f;
		const float HalfFOVY = HalfFOVX;
		const float MultFOVX = 1.0f;
		const float MultFOVY = 2.01166677;

		ProjectionMatrix = Hyper::CreateReversedPerspectiveMatrix(HalfFOVX, HalfFOVX, MultFOVX, MultFOVY, MinZ, MaxZ);
#endif

		float DepthMul = ProjectionMatrix.MatrixArray[2][2];
		float DepthAdd = ProjectionMatrix.MatrixArray[3][2];

		if (DepthAdd == 0.f)
		{
			DepthAdd = 0.00000001f;
		}

		float SubtractValue = DepthMul / DepthAdd;

		DeviceZToWorldTransform = Vector4f
		(
			0.0f,
			0.0f,
			1.0f / DepthAdd,
			SubtractValue
		);
	}
	
	void ProjectionSetup::CalculateInverseProjection()
	{
		if (ProjectionMatrix[1][0] == 0.0f &&
			ProjectionMatrix[3][0] == 0.0f &&
			ProjectionMatrix[0][1] == 0.0f &&
			ProjectionMatrix[3][1] == 0.0f &&
			ProjectionMatrix[0][2] == 0.0f &&
			ProjectionMatrix[1][2] == 0.0f &&
			ProjectionMatrix[0][3] == 0.0f &&
			ProjectionMatrix[1][3] == 0.0f &&
			ProjectionMatrix[2][3] == 1.0f &&
			ProjectionMatrix[3][3] == 0.0f)
		{
			double a = ProjectionMatrix[0][0];
			double b = ProjectionMatrix[1][1];
			double c = ProjectionMatrix[2][2];
			double d = ProjectionMatrix[3][2];
			double s = ProjectionMatrix[2][0];
			double t = ProjectionMatrix[2][1];

			Ensure(d != 0);
			Ensure(a != 0);
			Ensure(b != 0);

#pragma warning(disable:4244)

			ProjectionInverseMatrix = Matrix4x4
			(
				Vector4f(1.0 / a,	0.0f,		0.0f,	0.0f),
				Vector4f(0.0f,		1.0 / b,	0.0f,	0.0f),
				Vector4f(0.0f,		0.0f,		0.0f,	1.0 / d),
				Vector4f(-s  / a,	-t  / b,	1.0f,	-c  / d)
			);
		}
		else
		{
			ProjectionMatrix.VectorInverse(ProjectionInverseMatrix);
		}
	}
}
