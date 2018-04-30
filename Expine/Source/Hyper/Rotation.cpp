#include "../Precompiled.h"
#include "Quaternion.h"
#include "Rotation.h"
#include "Matrix.h"

namespace Hyper
{
	const Rotation Rotation::ZeroRotation = Rotation(0.0, 0.0, 0.0);

	Vector3f Rotation::GetVector() const
	{
		float CP, SP, CY, SY;
		Math::SinCos(&SP, &CP, Math::DegreesToRadians(Pitch));
		Math::SinCos(&SY, &CY, Math::DegreesToRadians(Yaw));
		Vector3f V = Vector3f(CP*CY, CP*SY, SP);

		return V;
	}

	Rotation Rotation::GetInverse() const
	{
		return Quaternion().Inverse().GetRotation();
	}

	Quaternion Rotation::GetQuaternion() const
	{
		DiagnosticCheckNaN();

#if PLATFORM_ENABLE_VECTORINTRINSICS
		const M128 Angles = MakeVectorRegister(Pitch, Yaw, Roll, 0.0f);
		const M128 HalfAngles = VectorMultiply(Angles, GlobalVectorConstants::DEG_TO_RAD_HALF);

		M128 SinAngles, CosAngles;
		VectorSinCos(&SinAngles, &CosAngles, &HalfAngles);

		// Vectorized conversion, measured 20% faster than using scalar version after VectorSinCos.
		// Indices within M128 (for shuffles): P=0, Y=1, R=2
		const M128 SR = VectorReplicate(SinAngles, 2);
		const M128 CR = VectorReplicate(CosAngles, 2);

		const M128 SY_SY_CY_CY_Temp = VectorShuffle(SinAngles, CosAngles, 1, 1, 1, 1);

		const M128 SP_SP_CP_CP = VectorShuffle(SinAngles, CosAngles, 0, 0, 0, 0);
		const M128 SY_CY_SY_CY = VectorShuffle(SY_SY_CY_CY_Temp, SY_SY_CY_CY_Temp, 0, 2, 0, 2);

		const M128 CP_CP_SP_SP = VectorShuffle(CosAngles, SinAngles, 0, 0, 0, 0);
		const M128 CY_SY_CY_SY = VectorShuffle(SY_SY_CY_CY_Temp, SY_SY_CY_CY_Temp, 2, 0, 2, 0);

		const uint32 Neg = uint32(1 << 31);
		const uint32 Pos = uint32(0);
		const M128 SignBitsLeft = MakeVectorRegister(Pos, Neg, Pos, Pos);
		const M128 SignBitsRight = MakeVectorRegister(Neg, Neg, Neg, Pos);
		const M128 LeftTerm = VectorBitwiseXor(SignBitsLeft, VectorMultiply(CR, VectorMultiply(SP_SP_CP_CP, SY_CY_SY_CY)));
		const M128 RightTerm = VectorBitwiseXor(SignBitsRight, VectorMultiply(SR, VectorMultiply(CP_CP_SP_SP, CY_SY_CY_SY)));

		Quaternion RotationQuat;
		const M128 Result = VectorAdd(LeftTerm, RightTerm);
		VectorStoreAligned(Result, &RotationQuat);
#else
		const float DEG_TO_RAD = PI / (180.f);
		const float DIVIDE_BY_2 = DEG_TO_RAD / 2.f;
		float SP, SY, SR;
		float CP, CY, CR;

		Math::SinCos(&SP, &CP, Pitch*DIVIDE_BY_2);
		Math::SinCos(&SY, &CY, Yaw*DIVIDE_BY_2);
		Math::SinCos(&SR, &CR, Roll*DIVIDE_BY_2);

		Quaternion RotationQuat;
		RotationQuat.X = CR*SP*SY - SR*CP*CY;
		RotationQuat.Y =-CR*SP*CY - SR*CP*SY;
		RotationQuat.Z = CR*CP*SY - SR*SP*CY;
		RotationQuat.W = CR*CP*CY + SR*SP*SY;
#endif

		RotationQuat.DiagnosticCheckNaN();

		return RotationQuat;
	}

	Vector3f Rotation::GetEuler() const
	{
		return Vector3f(Roll, Pitch, Yaw);
	}

	Rotation Rotation::MakeFromEuler
	(
		const Vector3f & Euler
	)
	{
		return Rotation(Euler.Y, Euler.Z, Euler.X);
	}

	Vector3f Rotation::UnrotateVector
	(
		const Vector3f & V
	)	const
	{
		return CreateInversedRotationMatrix(*this).GetTransposed().TransformVector(V);
	}

	Vector3f Rotation::RotateVector
	(
		const Vector3f & V
	)	const
	{
		return CreateInversedRotationMatrix(*this).TransformVector(V);
	}

	void Rotation::GetWindingAndRemainder
	(
				Rotation & Winding, 
				Rotation & Remainder
	)	const
	{
		//// YAW
		Remainder.Yaw = NormalizeAxis(Yaw);
		Winding.Yaw = Yaw - Remainder.Yaw;

		//// PITCH
		Remainder.Pitch = NormalizeAxis(Pitch);
		Winding.Pitch = Pitch - Remainder.Pitch;

		//// ROLL
		Remainder.Roll = NormalizeAxis(Roll);
		Winding.Roll = Roll - Remainder.Roll;
	}
}