#pragma once

#include "Scene.h"
#include "Input.h"

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

	class CSceneController
	{
		enum EMovingDirection
		{
			None,
			Forwards,
			Backwards,
			Right,
			Left
		};

	private:

		SharedPointer<CCamera> Camera;

	private:

		ConstPointer<CScene> Scene;

	private:

		bool IsDraggingLocked	= false;
		bool IsDragging			= false;
		
		EMovingDirection MovingDirection = None;

		int DragX = 0;
		int DragY = 0;

		Vector3f RotationVector = Vector3f::ZeroVector;

	public:

		CSceneController
		(
			const CScene * pScene
		);

		inline void SetPosition
		(
			const Vector3f & Position
		)
		{
			Camera->SetPosition(Position);
		}

		inline void SetRotation
		(
			const Rotation & Rotation
		)
		{
			Camera->SetRotation(Rotation);
		}

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
			MovingDirection = None;
		}

		inline void MoveForward()
		{
			MovingDirection = Forwards;
		}

		inline void MoveBackward()
		{
			MovingDirection = Backwards;
		}

		inline void MoveRight()
		{
			MovingDirection = Right;
		}

		inline void MoveLeft()
		{
			MovingDirection = Left;
		}

		inline void BeginDrag
		(
			const int ScreenX,
			const int ScreenY
		)
		{
			if (IsDraggingLocked)
			{
				return;
			}

			IsDragging = true;

			DragX = ScreenX;
			DragY = ScreenY;
		}

		inline void Drag
		(
			const int ScreenX,
			const int ScreenY
		)
		{
			static constexpr float ResistModificier = 1.0 / 3.0;

			if (!IsDragging)
			{
				BeginDrag(ScreenX, ScreenY);
				return;
			}

			RotationVector.X = (ScreenX - DragX) * ResistModificier;
			RotationVector.Y = (ScreenY - DragY) * ResistModificier;

			if (RotationVector.X != 0 || RotationVector.Y != 0)
			{
				Camera->Rotate(RotationVector);
			}

			DragX = ScreenX;
			DragY = ScreenY;
		}

		inline void Update()
		{
			switch (MovingDirection)
			{
			case Forwards:

				Camera->Move(10.0);

				break;

			case Backwards:

				Camera->Move(-10.0);

				break;

			case Right:

				Camera->MoveRight(10.0);

				break;

			case Left:

				Camera->MoveRight(-10.0);

				break;
			}
		}
	};
}