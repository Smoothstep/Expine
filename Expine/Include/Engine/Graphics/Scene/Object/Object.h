#pragma once

#include "D3D.h"
#include "ViewFrustum.h"

namespace D3D
{
	enum EObjectType
	{
		ObjectTypeNone = -1,
		ObjectTypeSpeedTree,
		ObjectTypeCount
	};

	class CScene;
	class CSceneRenderer;
	class CSceneEnvironment;
	
	class ISceneObjectController
	{
	protected:

		CSceneEnvironment * Environment;

	public:

		virtual const Vector3f & GetCenterPosition() const = 0;
		virtual const Vector3f & GetRotation() const = 0;

		virtual bool InsideFrustum
		(
			const ViewFrustum & Frustum
		)	const = 0;

		virtual bool IntersectsFrustum
		(
			const ViewFrustum & Frustum
		)	const = 0;
	};

	class CSceneObjectInterface
	{
	public:

		virtual void Render
		(
			const CSceneRenderer * Renderer
		)	const = 0;
	};

	class ISceneObjectComponents
	{
	public:

		virtual void Dispose() = 0;
	};

	class CSceneObject : public CSceneObjectInterface
	{
	public:

		struct InitializeOptions
		{
			EObjectType Type;
		};

	public:

		virtual ErrorCode LoadObject
		(
			InitializeOptions & Options
		)
		{
			return S_OK;
		}

		void SetObjectController(ISceneObjectController * ObjectController)
		{
			Controller = ObjectController;
		}

	protected:

		ISceneObjectController * Controller;
		ISceneObjectComponents * Components;
	};
}