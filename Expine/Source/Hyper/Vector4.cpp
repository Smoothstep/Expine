#include "../Precompiled.h"
#include "Vector4.h"
#include "Quaternion.h"

namespace Hyper
{
	Vector4f Vector4f::ZeroVector = Vector4f(0.0f);

	Rotation Vector4f::ToOrientationRotation() const
	{
		Rotation R;

		// Find yaw.
		R.Yaw = Math::Atan2(Y, X) * (180.f / PI);

		// Find pitch.
		R.Pitch = Math::Atan2(Z, Math::Sqrt(X * X + Y * Y)) * (180.f / PI);

		// Find roll.
		R.Roll = 0;

		return R;
	}

	Rotation Vector4f::GetRotation() const
	{
		return ToOrientationRotation();
	}

	Quaternion Vector4f::ToOrientationQuat() const
	{
		// Essentially an optimized Vector->Rotator->Quat made possible by knowing Roll == 0, and avoiding radians->degrees->radians.
		// This is done to avoid adding any roll (which our API states as a constraint).
		const float YawRad = Math::Atan2(Y, X);
		const float PitchRad = Math::Atan2(Z, Math::Sqrt(X*X + Y*Y));

		const float DIVIDE_BY_2 = 0.5f;
		float SP, SY;
		float CP, CY;

		Math::SinCos(&SP, &CP, PitchRad * DIVIDE_BY_2);
		Math::SinCos(&SY, &CY, YawRad * DIVIDE_BY_2);

		Quaternion RotationQuat;
		RotationQuat.X = SP*SY;
		RotationQuat.Y = -SP*CY;
		RotationQuat.Z = CP*SY;
		RotationQuat.W = CP*CY;
		return RotationQuat;
	}
}