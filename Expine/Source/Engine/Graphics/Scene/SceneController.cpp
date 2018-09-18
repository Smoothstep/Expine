#include "Precompiled.h"

#include "Scene/Scene.h"
#include "Scene/SceneView.h"
#include "Scene/SceneController.h"

namespace D3D
{
	CSceneController::CSceneController(const CScene * pScene)
		: Scene(pScene)
	{
		ProjectionSetup Setup
		(
			Scene->GetProperties().FieldOfView,
			Scene->GetView().GetAspect(),
			Scene->GetProperties().ClipNear,
			Scene->GetProperties().ClipFar
		);

		Camera = new CCamera(Setup);
		{
			Scene->SetCamera(Camera);
		}
	}

	void CSceneController::SetPosition
	(
		const Vector3f & Position
	)
	{
		Camera->SetPosition(Position);
	}

	void CSceneController::SetRotation
	(
		const Rotation & Rotation
	)
	{
		Camera->SetRotation(Rotation);
	}

	void CSceneController::BeginDrag
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

	void CSceneController::Drag
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

	void CSceneController::Update()
	{
		Camera->MoveDirection(MovingDirection, MovingAmplitude);
	}
}
