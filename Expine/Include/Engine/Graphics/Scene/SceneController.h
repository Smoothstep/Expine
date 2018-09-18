#pragma once

#include "Engine/IO/Input.h"

namespace D3D
{
	struct SceneCameraKeyAssignments
	{
		KeyboardKey MoveForwards;
		KeyboardKey MoveBackwards;
		KeyboardKey MoveLeft;
		KeyboardKey MoveRight;
		KeyboardKey ZoomIn;
		KeyboardKey ZoomOut;
		KeyboardKey RotateRight;
		KeyboardKey RotateLeft;
	};

	class _EX_ CSceneController
	{
	private:

		SharedPointer<CCamera> Camera;

	private:

		ConstPointer<CScene> Scene;

	private:

		bool IsDraggingLocked	= false;
		bool IsDragging			= false;
		
		Vector3f MovingDirection;
		Vector3f MovingAmplitude = 10.0f;

		int DragX = 0;
		int DragY = 0;

		Vector3f RotationVector = Vector3f::ZeroVector;

	public:

		CSceneController
		(
			const CScene * pScene
		);

		void SetPosition
		(
			const Vector3f & Position
		);

		void SetRotation
		(
			const Rotation & Rotation
		);

		inline void LockDrag()
		{
			IsDraggingLocked = true;
		}

		inline void UnlockDrag()
		{
			IsDraggingLocked = false;
		}

		inline void EndDrag()
		{
			DragX = 0;
			DragY = 0;

			IsDragging = false;
		}

		inline void StopMove()
		{
			MovingDirection = Vector3f::ZeroVector;
		}

		inline void MoveForward()
		{
			MovingDirection += Vector3f::ForwardVector;
		}

		inline void MoveBackward()
		{
			MovingDirection += Vector3f::BackwardVector;
		}

		inline void MoveRight()
		{
			MovingDirection += Vector3f::RightVector;
		}

		inline void MoveLeft()
		{
			MovingDirection += Vector3f::LeftVector;
		}

		inline void MoveUp()
		{
			MovingDirection += Vector3f::UpVector;
		}

		inline void MoveDown()
		{
			MovingDirection += Vector3f::DownVector;
		}

		void Drag
		(
			const int ScreenX,
			const int ScreenY
		);

		void Update();

		void BeginDrag
		(
			const int ScreenX,
			const int ScreenY
		);
	};
}