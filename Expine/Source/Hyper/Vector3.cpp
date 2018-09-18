#include "Hyper/Vector3.h"
#include "Hyper/Quaternion.h"

namespace Hyper
{
	const Vector3f Vector3f::ZeroVector		= Vector3f(0.0f, 0.0f, 0.0f);
	const Vector3f Vector3f::UpVector		= Vector3f(0.0f, 0.0f, 1.0f);
	const Vector3f Vector3f::DownVector		= Vector3f(0.0f, 0.0f, -1.0f);
	const Vector3f Vector3f::ForwardVector	= Vector3f(1.0f, 0.0f, 0.0f);
	const Vector3f Vector3f::BackwardVector	= Vector3f(-1.0f, 0.0f, 0.0f);
	const Vector3f Vector3f::RightVector	= Vector3f(0.0f, 1.0f, 0.0f);
	const Vector3f Vector3f::LeftVector		= Vector3f(0.0f, -1.0f, 0.0f);

	void Vector3f::UnwindEuler()
	{
		X = Math::UnwindDegrees(X);
		Y = Math::UnwindDegrees(Y);
		Z = Math::UnwindDegrees(Z);
	}

	void Vector3f::FindBestAxisVectors
	(
				Vector3f & Axis1, 
				Vector3f & Axis2
	)	const
	{
		const float NX = Math::Abs(X);
		const float NY = Math::Abs(Y);
		const float NZ = Math::Abs(Z);

		if (NZ > NX && NZ > NY)
		{
			Axis1 = Vector3f(1, 0, 0);
		}
		else
		{
			Axis1 = Vector3f(0, 0, 1);
		}

		Axis1 = (Axis1 - *this * (Axis1 | *this)).GetSafeNormal();
		Axis2 = (Axis1 ^ *this);
	}

	Quaternion Vector3f::ToOrientationQuat() const
	{
		const float YawRad = Math::Atan2(Y, X);
		const float PitchRad = Math::Atan2(Z, Math::Sqrt(X*X + Y*Y));

		const float DIVIDE_BY_2 = 0.5f;
		float SP, SY;
		float CP, CY;

		Math::SinCos(&SP, &CP, PitchRad * DIVIDE_BY_2);
		Math::SinCos(&SY, &CY, YawRad * DIVIDE_BY_2);

		Quaternion RotationQuat;

		RotationQuat.X = SP * SY;
		RotationQuat.Y =-SP * CY;
		RotationQuat.Z = CP * SY;
		RotationQuat.W = CP * CY;

		return RotationQuat;
	}

	void Vector3f::CreateOrthonormalBasis
	(
		Vector3f & XAxis, 
		Vector3f & YAxis, 
		Vector3f & ZAxis
	)
	{
		// Project the X and Y axes onto the plane perpendicular to the Z axis.
		XAxis -= (XAxis | ZAxis) / (ZAxis | ZAxis) * ZAxis;
		YAxis -= (YAxis | ZAxis) / (ZAxis | ZAxis) * ZAxis;

		// If the X axis was parallel to the Z axis, choose a vector which is orthogonal to the Y and Z axes.
		if (XAxis.SizeSquared() < DELTA*DELTA)
		{
			XAxis = YAxis ^ ZAxis;
		}

		// If the Y axis was parallel to the Z axis, choose a vector which is orthogonal to the X and Z axes.
		if (YAxis.SizeSquared() < DELTA*DELTA)
		{
			YAxis = XAxis ^ ZAxis;
		}

		// Normalize the basis vectors.
		XAxis.Normalize();
		YAxis.Normalize();
		ZAxis.Normalize();
	}

	Rotation Vector3f::ToOrientationRotation() const
	{
		Rotation R;

		// Find yaw.
		R.Yaw = Math::Atan2(Y, X) * (180.f / PI);

		// Find pitch.
		R.Pitch = Math::Atan2(Z, Math::Sqrt(X*X + Y*Y)) * (180.f / PI);

		// Find roll.
		R.Roll = 0;

		return R;
	}

	Vector3f Math::ClosestPointOnLine
	(
		const Vector3f & LineStart,
		const Vector3f & LineEnd,
		const Vector3f & Point
	)
	{
		// Solve to find alpha along line that is closest point
		// Weisstein, Eric W. "Point-Line Distance--3-Dimensional." From MathWorld--A Switchram Web Resource. http://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html 
		const float A = (LineStart - Point) | (LineEnd - LineStart);
		const float B = (LineEnd - LineStart).SizeSquared();
		// This should be robust to B == 0 (resulting in NaN) because clamp should return 1.
		const float T = Math::Clamp(-A / B, 0.f, 1.f);

		// Generate closest point
		Vector3f ClosestPoint = LineStart + (T * (LineEnd - LineStart));

		return ClosestPoint;
	}

	Vector3f ClosestPointOnInfiniteLine
	(
		const Vector3f & LineStart,
		const Vector3f & LineEnd,
		const Vector3f & Point
	)
	{
		const float A = (LineStart - Point) | (LineEnd - LineStart);
		const float B = (LineEnd - LineStart).SizeSquared();

		if (B < SMALL_NUMBER)
		{
			return LineStart;
		}

		const float T = -A / B;

		// Generate closest point
		const Vector3f ClosestPoint = LineStart + (T * (LineEnd - LineStart));
		return ClosestPoint;
	}
}