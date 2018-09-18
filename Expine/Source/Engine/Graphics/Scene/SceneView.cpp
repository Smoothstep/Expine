#include "Precompiled.h"

#include "Scene/SceneView.h"

#include <DXTK\SimpleMath.h>

namespace D3D
{
	void CSceneView::SetViewportShadowMap
	(
		const ScreenViewport & Viewport
	)
	{
		this->ViewportShadowMap = Viewport;
		this->ViewportShadowMap.MaxDepth				= Math::Clamp(Viewport.MaxDepth,			0.0f,	1.0f);
		this->ViewportShadowMap.MaxDepth				= Math::Clamp(Viewport.MaxDepth,			0.0f,	1.0f);
		this->ViewportShadowMap.TopLeftX				= Math::Clamp(Viewport.TopLeftX,			0.0f,	ShadowSizeX);
		this->ViewportShadowMap.TopLeftY				= Math::Clamp(Viewport.TopLeftY,			0.0f,	ShadowSizeY);
		this->ViewportShadowMap.Width					= Math::Clamp(Viewport.Width,				0.0f,	ShadowSizeX);
		this->ViewportShadowMap.Height					= Math::Clamp(Viewport.Height,				0.0f,	ShadowSizeY);
		this->ViewportShadowMap.ScissorRect.bottom		= Math::Clamp(Viewport.ScissorRect.bottom,	0L,		ShadowSize.Y);
		this->ViewportShadowMap.ScissorRect.right		= Math::Clamp(Viewport.ScissorRect.right,	0L,		ShadowSize.X);
		this->ViewportShadowMap.ScissorRect.left		= Math::Clamp(Viewport.ScissorRect.left,	0L,		ShadowSize.X);
		this->ViewportShadowMap.ScissorRect.top			= Math::Clamp(Viewport.ScissorRect.top,		0L,		ShadowSize.Y);
	}

	void CSceneView::SetViewport
	(
		const ScreenViewport & Viewport
	)
	{
		this->Viewport = Viewport;
		this->Viewport.MaxDepth				= Math::Clamp(Viewport.MaxDepth,			0.0f,	1.0f);
		this->Viewport.MaxDepth				= Math::Clamp(Viewport.MaxDepth,			0.0f,	1.0f);
		this->Viewport.TopLeftX				= Math::Clamp(Viewport.TopLeftX,			0.0f,	ViewSizeX);
		this->Viewport.TopLeftY				= Math::Clamp(Viewport.TopLeftY,			0.0f,	ViewSizeY);
		this->Viewport.Width				= Math::Clamp(Viewport.Width,				0.0f,	ViewSizeX);
		this->Viewport.Height				= Math::Clamp(Viewport.Height,				0.0f,	ViewSizeY);
		this->Viewport.ScissorRect.bottom	= Math::Clamp(Viewport.ScissorRect.bottom,	0L,		ViewSize.Y);
		this->Viewport.ScissorRect.right	= Math::Clamp(Viewport.ScissorRect.right,	0L,		ViewSize.X);
		this->Viewport.ScissorRect.left		= Math::Clamp(Viewport.ScissorRect.left,	0L,		ViewSize.X);
		this->Viewport.ScissorRect.top		= Math::Clamp(Viewport.ScissorRect.top,		0L,		ViewSize.Y);
	}

	ViewSetup::ViewSetup(const CCamera * pCamera)
	{
		Update(pCamera);
	}

	void ViewSetup::Update(const CCamera * pCamera)
	{
		Ensure(pCamera != NULL);

		Vector3f LocalViewOrigin = pCamera->GetPosition();
		
		Matrix4x4 ViewRotationMatrix = CreateInversedRotationMatrix(pCamera->GetRotation()) *
			Matrix4x4(
				Vector4f(0, 0, 1, 0),
				Vector4f(1, 0, 0, 0),
				Vector4f(0, 1, 0, 0),
				Vector4f(0, 0, 0, 1));

		Vector3f Origin = ViewRotationMatrix.GetOrigin();

		if (Origin != Vector3f::ZeroVector)
		{
			LocalViewOrigin += ViewRotationMatrix.InverseTransformPosition(Vector3f::ZeroVector);
			ViewRotationMatrix.RemoveTranslation();
		}

		ViewMatrix = Matrix4x4::Translation(-LocalViewOrigin) * ViewRotationMatrix;
		ViewInverseMatrix = ViewRotationMatrix.GetTransposed() * Matrix4x4::Translation(LocalViewOrigin);

		// Projection

		ProjectionMatrix			= pCamera->GetProjectionAdjusted().GetProjectionMatrix();
		ProjectionInverseMatrix		= pCamera->GetProjectionAdjusted().GetProjectionInverseMatrix();
		ProjectionUnadjustedMatrix	= pCamera->GetProjection().GetProjectionMatrix();
		
		// ViewProjection

		ViewProjectionMatrix		= ViewMatrix * ProjectionMatrix;
		ViewProjectionInverseMatrix = ProjectionInverseMatrix * ViewInverseMatrix;
		ViewProjectionFrustumMatrix = ViewMatrix * pCamera->GetProjection().GetProjectionFarMatrix();

		Matrix4x4 LocalTranslatedViewMatrix		= ViewRotationMatrix;
		Matrix4x4 LocalInvTranslatedViewMatrix	= ViewRotationMatrix.GetTransposed();

		ViewOrigin = LocalViewOrigin;

		ViewFocus = Vector3f
		(
			0.0f,
			0.0f,
			Origin.Z
		) + ViewOrigin;

		ViewUp = Vector3f
		(
			0.0f,
			Origin.Y,
			0.0f
		);

		TranslatedViewMatrix = LocalTranslatedViewMatrix;
		TranslatedViewInverseMatrix = LocalInvTranslatedViewMatrix;

		TranslatedViewProjectionMatrix = LocalTranslatedViewMatrix * ProjectionMatrix;
		TranslatedViewProjectionInverseMatrix = ProjectionInverseMatrix * LocalInvTranslatedViewMatrix;

		ScreenToTranslatedWorldMatrix = Matrix4x4
		(
			Vector4f(1, 0, 0, 0),
			Vector4f(0, 1, 0, 0),
			Vector4f(0, 0, ProjectionUnadjustedMatrix.MatrixArray[2][2], 1),
			Vector4f(0, 0, ProjectionUnadjustedMatrix.MatrixArray[3][2], 0)
		) * TranslatedViewProjectionInverseMatrix;

		DeviceZToWorld = pCamera->GetProjection().GetDeviceZToWorldTransform();
	}
}
