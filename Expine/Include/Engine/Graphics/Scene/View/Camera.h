#pragma once

#include "DirectX/D3D.h"
#include "Hyper.h"

namespace D3D
{
	struct ProjectionSetup
	{
	private:

		static constexpr Float GProjectionSignY		= 1.0f;
		static constexpr Float GMinClipZ			= 0.0f;

		Float FOV;
		Float Aspect;
		Float ClipNearZ;
		Float ClipFarZ;

		Float ViewDistance;

		Vector4f DeviceZToWorldTransform;

		Matrix4x4 ProjectionMatrix;
		Matrix4x4 ProjectionInverseMatrix;
		Matrix4x4 ProjectionFarMatrix;

	private:

		void CalculateProjectionMatrix();
		void CalculateInverseProjection();

	public:

		inline Float GetViewDistance() const
		{
			return ViewDistance;
		}

		inline const Matrix4x4 & GetProjectionFarMatrix() const
		{
			return ProjectionFarMatrix;
		}

		inline const Matrix4x4 & GetProjectionMatrix() const
		{
			return ProjectionMatrix;
		}

		inline const Matrix4x4 & GetProjectionInverseMatrix() const
		{
			return ProjectionInverseMatrix;
		}

		inline const Vector4f & GetDeviceZToWorldTransform() const
		{
			return DeviceZToWorldTransform;
		}

		inline void SetParameters
		(
			const Float FOV,
			const Float Aspect,
			const Float ClipNearZ,
			const Float ClipFarZ
		)
		{
			this->FOV		= FOV;
			this->Aspect	= Aspect;
			this->ClipFarZ	= ClipFarZ;
			this->ClipNearZ = ClipNearZ;

			this->ViewDistance = ClipFarZ - ClipNearZ;

			CalculateProjectionMatrix();
			CalculateInverseProjection();
		}

	public:

		inline ProjectionSetup() {};
		inline ProjectionSetup
		(
			const Float FOV,
			const Float Aspect,
			const Float ClipNearZ,
			const Float ClipFarZ
		)
		{
			Ensure(ClipNearZ > SMALL_NUMBER);
			Ensure(ClipFarZ	 > SMALL_NUMBER);

			SetParameters(FOV, Aspect, ClipNearZ, ClipFarZ);
		}

		inline ProjectionSetup GetAdjustedProjectionMatrix() const
		{
			Matrix4x4 ClipSpaceScale = Matrix4x4::Scale(
			{
				1.0f,
				GProjectionSignY,
				1.0f - GMinClipZ
			});

			Matrix4x4 ClipSpaceTranslation = Matrix4x4::Translation(
			{
				0.0f,
				0.0f,
				GMinClipZ
			});

			ProjectionSetup Setup;
			{
				Setup.FOV				= FOV;
				Setup.Aspect			= Aspect;
				Setup.ClipFarZ			= ClipFarZ;
				Setup.ClipNearZ			= ClipNearZ;
				Setup.ProjectionMatrix	= ProjectionMatrix * ClipSpaceScale * ClipSpaceTranslation;

				Setup.CalculateInverseProjection();
			}

			return Setup;
		}
	};

	class CCamera
	{
	private:

		ProjectionSetup Projection;
		ProjectionSetup ProjectionAdjusted;

	private:

		Vector3f CameraPosition = Vector3f(0.0f, 0.0f, 40.0f);

	private:

		Rotation CameraRotation = Rotation::ZeroRotation;

	public:

		inline const ProjectionSetup & GetProjection() const
		{
			return Projection;
		}

		inline const ProjectionSetup & GetProjectionAdjusted() const
		{
			return ProjectionAdjusted;
		}

		inline const Vector3f & GetPosition() const
		{
			return CameraPosition;
		}

		inline const Rotation & GetRotation() const
		{
			return CameraRotation;
		}

	public:

		inline void SetProjection
		(
			const ProjectionSetup & Setup
		)
		{
			Projection			= Setup;
			ProjectionAdjusted	= Setup.GetAdjustedProjectionMatrix();
		}

		inline void SetPosition
		(
			const Vector3f & Position
		)
		{
			CameraPosition = Position;
		}

		inline void SetRotation
		(
			const Rotation & Rotation
		)
		{
			CameraRotation = Rotation;
		}

	public:

		CCamera();
		CCamera
		(
			const ProjectionSetup & Setup
		);

		inline void RotateX
		(
			const Float AngleDegree
		)
		{
			CameraRotation.Pitch += AngleDegree;
		}

		inline void RotateY
		(
			const Float AngleDegree
		)
		{
			CameraRotation.Yaw += AngleDegree;
		}

		inline void RotateZ
		(
			const Float AngleDegree
		)
		{
			CameraRotation.Roll += AngleDegree;
		}

		void Rotate
		(
			Vector3f V
		);

		inline void Move
		(
			const Float Distance
		)
		{
			CameraPosition += CameraRotation.GetVector() * Distance;
		}

		inline void MoveDirection
		(
			const Vector3f &	Direction,
			const Float			Distance
		)
		{
			CameraPosition += CameraRotation.GetVector() * Distance * Direction;
		}

		inline void MoveDirection
		(
			const Vector3f &	Direction,
			const Vector3f &	Distance
		)
		{
			CameraPosition += CameraRotation.GetVector() * Distance * Direction;
		}

		inline void MoveRight
		(
			const Float Distance
		)
		{
			Rotation Rot = Rotation(0, CameraRotation.Yaw, 0);
			{
				Rot.Yaw += 90.0f;

				if (Rot.Yaw > 360.0f)
				{
					Rot.Yaw -= 360.0f;
				}
				else if (Rot.Yaw < 0.0f)
				{
					Rot.Yaw += 360.0f;
				}
			}

			CameraPosition += Rot.GetVector() * Distance;
		}

		inline void MoveRotate3D
		(
			const Vector3f & Direction,
			const Float		 Distance
		)
		{
			CameraRotation = Direction.ToOrientationQuat();
			Move(Distance);
		}

		inline void MoveX
		(
			const Float Distance
		)
		{
			CameraPosition.X += Distance;
		}

		inline void MoveY
		(
			const Float Distance
		)
		{
			CameraPosition.Y += Distance;
		}

		inline void MoveZ
		(
			const Float Distance
		)
		{
			CameraPosition.Z += Distance;
		}
	};
}