
#include "Actor.h"

#include "../VibeD3D/ViewFrustum.h"

bool VActorObject::InsideFrustum(const D3D::ViewFrustum & Frustum) const
{
	return Frustum.IntersectsBox(
		PhysX::ToHx(Actor->PxGetBounds().getCenter()), 
		PhysX::ToHx(Actor->PxGetBounds().getExtents()));
}

bool VActorObject::IntersectsFrustum(const D3D::ViewFrustum & Frustum) const
{
	return Frustum.IntersectsBox(
		PhysX::ToHx(Actor->PxGetBounds().getCenter()),
		PhysX::ToHx(Actor->PxGetBounds().getExtents()));
}

const Vector3f & VActorObject::GetCenterPosition() const 
{
	return Actor->GetPosition();
}

const Vector3f & VActorObject::GetRotation() const
{
	return Actor->GetRotation();
}
