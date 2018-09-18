#pragma once

#include "Utils/DXGIHelper.h"

#include "Scene/View/Camera.h"
#include "Scene/View/ViewFrustum.h"

namespace D3D
{
	struct ViewSetup
	{
		Matrix4x4 ViewMatrix;
		Matrix4x4 ViewInverseMatrix;

		Matrix4x4 ViewProjectionMatrix;
		Matrix4x4 ViewProjectionFrustumMatrix;
		Matrix4x4 ViewProjectionInverseMatrix;

		Matrix4x4 TranslatedViewMatrix;
		Matrix4x4 TranslatedViewInverseMatrix;

		Matrix4x4 TranslatedViewProjectionMatrix;
		Matrix4x4 TranslatedViewProjectionInverseMatrix;

		Matrix4x4 RotationMatrix;
		Matrix4x4 RotationInverseMatrix;

		Matrix4x4 ScreenToTranslatedWorldMatrix;
		Matrix4x4 ScreenToTranslatedWorldInverseMatrix;

		Matrix4x4 WorldMatrix;
		Matrix4x4 WorldInverseMatrix;

		Matrix4x4 ProjectionMatrix;
		Matrix4x4 ProjectionInverseMatrix;
		Matrix4x4 ProjectionFarMatrix;

		Matrix4x4 ProjectionUnadjustedMatrix;

		Vector3f ViewOrigin;
		Vector3f ViewFocus;
		Vector3f ViewUp;

		Rotation ViewRotation;

		Vector4f DeviceZToWorld;

		ViewSetup() = default;
		ViewSetup
		(
			const CCamera * pCamera
		);

		void Update
		(
			const CCamera * pCamera
		);
	};

	class CSceneView
	{
	private:

		ViewSetup Setup;
		ViewSetup SetupBefore;

		ViewFrustum Frustum;

	private:

		LongPoint ViewSize;

		Float ViewSizeX;
		Float ViewSizeY;
		Float ViewAspect;

		LongPoint ShadowSize;

		Float ShadowSizeX;
		Float ShadowSizeY;
		Float ShadowAspect;

	private:

		ScreenViewport Viewport;
		ScreenViewport ViewportShadowMap;

	public:

		void SetViewport
		(
			const ScreenViewport & Viewport
		);

		void SetViewportShadowMap
		(
			const ScreenViewport & Viewport
		);

		inline void SetSize
		(
			const LongPoint Size
		)
		{
			ViewSize	= Size;
			ViewSizeX	= Size.X;
			ViewSizeY	= Size.Y;
			
			ViewAspect = ViewSizeX / ViewSizeY;
		}

		inline void SetShadowMapSize
		(
			const LongPoint Size	
		)
		{
			ShadowSize	= Size;
			ShadowSizeX = Size.X;
			ShadowSizeY = Size.Y;

			ShadowAspect = ViewSizeX / ViewSizeY;
		}

		inline const ScreenViewport & GetViewport() const
		{
			return Viewport;
		}

		inline const ScreenViewport & GetViewportShadowMap() const
		{
			return ViewportShadowMap;
		}

		inline LongPoint GetSize() const
		{
			return ViewSize;
		}

		inline LongPoint GetSizeShadowMap() const
		{
			return ShadowSize;
		}

		inline Float GetAspect() const
		{
			return ViewAspect;
		}

		inline Float GetAspectShadowMap() const
		{
			return ShadowAspect;
		}

		inline const ViewSetup & GetViewSetup() const
		{
			return Setup;
		}

		inline const ViewSetup & GetViewSetupBefore() const
		{
			return SetupBefore;
		}

		inline const ViewFrustum & GetViewFrustum() const
		{
			return Frustum;
		}

	public:

		inline void UpdateSetup
		(
			const CCamera * pCamera
		)
		{
			SetupBefore = Setup;
			{
				Setup.Update(pCamera);
			}
		}

		inline void UpdateFrustum()
		{
			Frustum.Update(Setup.ViewProjectionFrustumMatrix, true, true);
		}

		inline void Update
		(
			const CCamera * pCamera
		)
		{
			UpdateSetup(pCamera);
			UpdateFrustum();
		}
	};
}