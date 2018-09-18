#include "Hyper/Quaternion.h"

namespace Hyper
{
	const Quaternion Quaternion::Identity = Quaternion(0, 0, 0, 1);

	Quaternion Quaternion::Slerp_NotNormalized
	(
		const	Quaternion & Quat1, 
		const	Quaternion & Quat2, 
		const	float		 Slerp
	)
	{
		const float RawCosom =
			Quat1.X * Quat2.X +
			Quat1.Y * Quat2.Y +
			Quat1.Z * Quat2.Z +
			Quat1.W * Quat2.W;

		const float Cosom = Math::FloatSelect(RawCosom, RawCosom, -RawCosom);

		float Scale0, Scale1;

		if (Cosom < 0.9999f)
		{
			const float Omega = Math::Acos(Cosom);
			const float InvSin = 1.f / Math::Sin(Omega);
			Scale0 = Math::Sin((1.f - Slerp) * Omega) * InvSin;
			Scale1 = Math::Sin(Slerp * Omega) * InvSin;
		}
		else
		{
			Scale0 = 1.0f - Slerp;
			Scale1 = Slerp;
		}

		Scale1 = Math::FloatSelect(RawCosom, Scale1, -Scale1);

		Quaternion Result;

		Result.X = Scale0 * Quat1.X + Scale1 * Quat2.X;
		Result.Y = Scale0 * Quat1.Y + Scale1 * Quat2.Y;
		Result.Z = Scale0 * Quat1.Z + Scale1 * Quat2.Z;
		Result.W = Scale0 * Quat1.W + Scale1 * Quat2.W;

		return Result;
	}

	Quaternion Quaternion::SlerpFullPath_NotNormalized
	(
		const	Quaternion & quat1, 
		const	Quaternion & quat2, 
		const	float		 Alpha
	)
	{
		const float CosAngle = Math::Clamp(quat1 | quat2, -1.f, 1.f);
		const float Angle = Math::Acos(CosAngle);

		if (Math::Abs(Angle) < KINDA_SMALL_NUMBER)
		{
			return quat1;
		}

		const float SinAngle = Math::Sin(Angle);
		const float InvSinAngle = 1.f / SinAngle;

		const float Scale0 = Math::Sin((1.0f - Alpha)*Angle)*InvSinAngle;
		const float Scale1 = Math::Sin(Alpha*Angle)*InvSinAngle;

		return quat1*Scale0 + quat2*Scale1;
	}

	Quaternion Quaternion::Squad
	(
		const	Quaternion & quat1, 
		const	Quaternion & tang1, 
		const	Quaternion & quat2, 
		const	Quaternion & tang2, 
		const	float		 Alpha
	)
	{
		const Quaternion Q1 = Quaternion::Slerp_NotNormalized(quat1, quat2, Alpha);
		const Quaternion Q2 = Quaternion::SlerpFullPath_NotNormalized(tang1, tang2, Alpha);
		const Quaternion Result = Quaternion::SlerpFullPath(Q1, Q2, 2.f * Alpha * (1.f - Alpha));

		return Result;
	}

	Quaternion Quaternion::SquadFullPath
	(
		const	Quaternion & quat1, 
		const	Quaternion & tang1, 
		const	Quaternion & quat2, 
		const	Quaternion & tang2, 
		const	float		 Alpha
	)
	{
		const Quaternion Q1 = Quaternion::SlerpFullPath_NotNormalized(quat1, quat2, Alpha);
		const Quaternion Q2 = Quaternion::SlerpFullPath_NotNormalized(tang1, tang2, Alpha);
		const Quaternion Result = Quaternion::SlerpFullPath(Q1, Q2, 2.f * Alpha * (1.f - Alpha));

		return Result;
	}

	void Quaternion::CalcTangents
	(
		const	Quaternion & PrevP, 
		const	Quaternion & P, 
		const	Quaternion & NextP, 
		const	float		 Tension,
				Quaternion & OutTan
	)
	{
		const Quaternion InvP = P.Inverse();
		const Quaternion Part1 = (InvP * PrevP).Log();
		const Quaternion Part2 = (InvP * NextP).Log();

		const Quaternion PreExp = (Part1 + Part2) * -0.5f;

		OutTan = P * PreExp.Exp();
	}

	FORCEINLINE Quaternion FindBetween_Helper
	(
		const	Vector3f & A, 
		const	Vector3f & B, 
		const	float	   NormAB)
	{
		float W = NormAB + Vector3f::DotProduct(A, B);
		Quaternion Result;

		if (W >= 1e-6f * NormAB)
		{
			//Axis = Vector3f::CrossProduct(A, B);
			Result = Quaternion(A.Y * B.Z - A.Z * B.Y,
				A.Z * B.X - A.X * B.Z,
				A.X * B.Y - A.Y * B.X,
				W);
		}
		else
		{
			// A and B point in opposite directions
			W = 0.f;
			Result = Math::Abs(A.X) > Math::Abs(A.Y)
				? Quaternion(-A.Z, 0.f, A.X, W)
				: Quaternion(0.f, -A.Z, A.Y, W);
		}

		Result.Normalize();
		return Result;
	}

	Quaternion Quaternion::FindBetweenNormals(const Vector3f& A, const Vector3f& B)
	{
		const float NormAB = 1.f;
		return FindBetween_Helper(A, B, NormAB);
	}

	Quaternion Quaternion::FindBetweenVectors(const Vector3f& A, const Vector3f& B)
	{
		const float NormAB = Math::Sqrt(A.SizeSquared() * B.SizeSquared());
		return FindBetween_Helper(A, B, NormAB);
	}

	Quaternion Quaternion::Log() const
	{
		Quaternion Result;
		Result.W = 0.f;

		if (Math::Abs(W) < 1.f)
		{
			const float Angle = Math::Acos(W);
			const float SinAngle = Math::Sin(Angle);

			if (Math::Abs(SinAngle) >= SMALL_NUMBER)
			{
				const float Scale = Angle / SinAngle;
				Result.X = Scale*X;
				Result.Y = Scale*Y;
				Result.Z = Scale*Z;

				return Result;
			}
		}

		Result.X = X;
		Result.Y = Y;
		Result.Z = Z;

		return Result;
	}

	Quaternion Quaternion::Exp() const
	{
		const float Angle = Math::Sqrt(X*X + Y*Y + Z*Z);
		const float SinAngle = Math::Sin(Angle);

		Quaternion Result;
		Result.W = Math::Cos(Angle);

		if (Math::Abs(SinAngle) >= SMALL_NUMBER)
		{
			const float Scale = SinAngle / Angle;
			Result.X = Scale*X;
			Result.Y = Scale*Y;
			Result.Z = Scale*Z;
		}
		else
		{
			Result.X = X;
			Result.Y = Y;
			Result.Z = Z;
		}

		return Result;
	}

	Rotation Quaternion::GetRotation() const
	{
		DiagnosticCheckNaN();

		const float SingularityTest = Z*X - W*Y;
		const float YawY = 2.f*(W*Z + X*Y);
		const float YawX = (1.f - 2.f*(Math::Square(Y) + Math::Square(Z)));

		// reference 
		// http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
		// http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/

		// this value was found from experience, the above websites recommend different values
		// but that isn't the case for us, so I went through different testing, and finally found the case 
		// where both of world lives happily. 
		const float SINGULARITY_THRESHOLD = 0.4999995f;
		const float RAD_TO_DEG = (180.f) / PI;
		Rotation RotatorFromQuat;

		if (SingularityTest < -SINGULARITY_THRESHOLD)
		{
			RotatorFromQuat.Pitch = -90.f;
			RotatorFromQuat.Yaw = Math::Atan2(YawY, YawX) * RAD_TO_DEG;
			RotatorFromQuat.Roll = Rotation::NormalizeAxis(-RotatorFromQuat.Yaw - (2.f * Math::Atan2(X, W) * RAD_TO_DEG));
		}
		else if (SingularityTest > SINGULARITY_THRESHOLD)
		{
			RotatorFromQuat.Pitch = 90.f;
			RotatorFromQuat.Yaw = Math::Atan2(YawY, YawX) * RAD_TO_DEG;
			RotatorFromQuat.Roll = Rotation::NormalizeAxis(RotatorFromQuat.Yaw - (2.f * Math::Atan2(X, W) * RAD_TO_DEG));
		}
		else
		{
			RotatorFromQuat.Pitch = Math::FastAsin(2.f*(SingularityTest)) * RAD_TO_DEG;
			RotatorFromQuat.Yaw = Math::Atan2(YawY, YawX) * RAD_TO_DEG;
			RotatorFromQuat.Roll = Math::Atan2(-2.f*(W*X + Y*Z), (1.f - 2.f*(Math::Square(X) + Math::Square(Y)))) * RAD_TO_DEG;
		}

#if ENABLE_NAN_DIAGNOSTIC
		if (RotatorFromQuat.ContainsNaN())
		{
			logOrEnsureNanError(TEXT("Quaternion::Rotator(): Rotator result %s contains NaN! Quat = %s, YawY = %.9f, YawX = %.9f"), *RotatorFromQuat.ToString(), *this->ToString(), YawY, YawX);
			RotatorFromQuat = Rotation::ZeroRotator;
		}
#endif

		return RotatorFromQuat;
	}

	Quaternion Quaternion::MakeFromEuler
	(
		const Vector3f& Euler
	)
	{
		return Rotation::MakeFromEuler(Euler).GetQuaternion();
	}

	void Quaternion::ToSwingTwist
	(
		const	Vector3f	& InTwistAxis, 
				Quaternion	& OutSwing, 
				Quaternion	& OutTwist
	)	const
	{
		// Vector part projected onto twist axis
		Vector3f Projection = Vector3f::DotProduct(InTwistAxis, Vector3f(X, Y, Z)) * InTwistAxis;

		// Twist quaternion
		OutTwist = Quaternion(Projection.X, Projection.Y, Projection.Z, W);

		// Singularity close to 180deg
		if (OutTwist.SizeSquared() == 0.0f)
		{
			OutTwist = Quaternion::Identity;
		}
		else
		{
			OutTwist.Normalize();
		}

		// Set swing
		OutSwing = *this * OutTwist.Inverse();
	}
}