#pragma once

#include "PhysX.h"
#include "../VibeD3D/Object.h"

struct ActorInitializeProperties
{
	UINT64 ActorId;
	Vector3f Position;
	Vector3f Direction;
};

class VActor;
class VActorObject : public D3D::ISceneObjectController
{
private:

	VActor * Actor;

public:

	virtual bool InsideFrustum
	(
		const D3D::ViewFrustum & Frustum
	)	const override;

	virtual bool IntersectsFrustum
	(
		const D3D::ViewFrustum & Frustum
	)	const override;

	virtual const Vector3f & GetCenterPosition() const override;
	virtual const Vector3f & GetRotation() const override;
};

class VActor : public PhysX::XActor
{
public:

	inline const Vector3f & GetRotation() const
	{
		return Rotation;
	}

public:
	
	VActor
	(
		ActorInitializeProperties & Properties
	) :
		PhysX::XActor(Properties.Position)
	{
		ActorId = Properties.ActorId;
	}

private:

	UINT64 ActorId;

private:

	Vector3f Rotation;
};