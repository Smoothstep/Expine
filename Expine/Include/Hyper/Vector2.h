#pragma once

#include "Math.h"

namespace Hyper
{
	struct Vector2f
	{
		float X;
		float Y;

	public:
		static const Vector2f ZeroVector;
		static const Vector2f UnitVector;

	public:
		FORCEINLINE Vector2f() { }
		FORCEINLINE Vector2f
		(
			float InX, 
			float InY
		);

		FORCEINLINE Vector2f
		(
			IntPoint InPos
		);

		FORCEINLINE Vector2f
		(
			const Vector3f & V
		);

	public:
		FORCEINLINE Vector2f operator+
		(
			const Vector2f & V
		)	const;

		FORCEINLINE Vector2f operator-
		(
			const Vector2f & V
		)	const;

		FORCEINLINE Vector2f operator*
		(
			float Scale
		)	const;

		FORCEINLINE Vector2f operator/
		(
			float Scale
		)	const;

		FORCEINLINE Vector2f operator+
		(
			float A
		)	const;

		FORCEINLINE Vector2f operator-
		(
			float A
		)	const;

		FORCEINLINE Vector2f operator*
		(
			const Vector2f& V
		)	const;


		FORCEINLINE Vector2f operator/
		(
			const Vector2f& V
		)	const;

		FORCEINLINE float operator|
		(
			const Vector2f& V
		)	const;


		FORCEINLINE float operator^
		(
			const Vector2f& V
		)	const;

	public:
		FORCEINLINE bool operator==
		(
			const Vector2f& V
		)	const;

		FORCEINLINE bool operator!=
		(
			const Vector2f& V
		)	const;

		FORCEINLINE bool operator<
		(
			const Vector2f& Other
		)	const;

		FORCEINLINE bool operator>
		(
			const Vector2f& Other
		)	const;

		FORCEINLINE bool operator<=
		(
			const Vector2f& Other
		)	const;

		FORCEINLINE bool operator>=
		(	
			const Vector2f& Other
		)	const;

		FORCEINLINE Vector2f operator-() const;

		FORCEINLINE Vector2f operator+=
		(
			const Vector2f& V
		);

		FORCEINLINE Vector2f operator-=
		(
			const Vector2f& V
		);

		FORCEINLINE Vector2f operator*=
		(
			float Scale
		);

		FORCEINLINE Vector2f operator/=
		(
			float V
		);

		FORCEINLINE Vector2f operator*=
		(
			const Vector2f& V
		);

		FORCEINLINE Vector2f operator/=
		(
			const Vector2f& V
		);

		FORCEINLINE float& operator[]
		(
			int32 Index
		);

		FORCEINLINE float operator[]
		(
			int32 Index
		)	const;

		FORCEINLINE float& Component
		(
			int32 Index
		);

		FORCEINLINE float Component
		(
			int32 Index
		)	const;

	public:
		FORCEINLINE static float DotProduct
		(
			const Vector2f& A, 
			const Vector2f& B
		);

		FORCEINLINE static float DistSquared
		(
			const Vector2f& V1, 
			const Vector2f& V2
		);

		FORCEINLINE static float Distance
		(
			const Vector2f& V1, 
			const Vector2f& V2
		);

		FORCEINLINE static float CrossProduct
		(
			const Vector2f& A, 
			const Vector2f& B
		);

		FORCEINLINE bool Equals
		(
			const Vector2f& V, 
			float Tolerance = FLT_EPSILON
		)	const;

		FORCEINLINE void Set
		(
			float InX, 
			float InY
		);

		FORCEINLINE float GetMax()		const;
		FORCEINLINE float GetAbsMax()	const;
		FORCEINLINE float GetMin()		const;
		FORCEINLINE float Size()		const;
		FORCEINLINE float SizeSquared() const;

		FORCEINLINE Vector2f GetRotated
		(
			float AngleDeg
		)	const;

		FORCEINLINE Vector2f GetSafeNormal
		(
			float Tolerance = FLT_EPSILON
		)	const;

		FORCEINLINE void Normalize
		(
			float Tolerance = FLT_EPSILON
		);

		FORCEINLINE bool IsNearlyZero
		(
			float Tolerance = FLT_EPSILON
		)	const;

		FORCEINLINE void ToDirectionAndLength
		(
			Vector2f	&OutDir, 
			float		&OutLength
		)	const;

		FORCEINLINE bool IsZero() const;

		FORCEINLINE IntPoint AsIntPoint() const;

		FORCEINLINE Vector2f ClampAxes
		(
			float MinAxisVal, 
			float MaxAxisVal
		)	const;


		FORCEINLINE Vector2f GetSignVector() const;
		FORCEINLINE Vector2f GetAbs() const;

		FORCEINLINE void DiagnosticCheckNaN() {}
		FORCEINLINE Vector3f SphericalToUnitCartesian() const;
	};

	FORCEINLINE Vector2f operator*
	(
		const float		 Scale,
		const Vector2f & V
	)
	{
		return V * Scale;
	}

	FORCEINLINE Vector2f::Vector2f
	(
		const float InX, 
		const float InY
	)
		: X(InX), Y(InY)
	{ }


	FORCEINLINE Vector2f::Vector2f
	(
		IntPoint InPos
	)
	{
		X = (float)InPos.X;
		Y = (float)InPos.Y;
	}

	FORCEINLINE Vector2f Vector2f::operator+
	(
		const Vector2f & V
	)	const
	{
		return Vector2f
		(
			X + V.X, 
			Y + V.Y
		);
	}


	FORCEINLINE Vector2f Vector2f::operator-
	(
		const Vector2f & V
	)	const
	{
		return Vector2f
		(
			X - V.X, 
			Y - V.Y
		);
	}


	FORCEINLINE Vector2f Vector2f::operator*
	(
		const float Scale
	)	const
	{
		return Vector2f
		(
			X * Scale, 
			Y * Scale
		);
	}


	FORCEINLINE Vector2f Vector2f::operator/
	(
		const float Scale
	)	const
	{
		const float RScale = 1.f / Scale;

		return Vector2f
		(
			X * RScale, 
			Y * RScale
		);
	}


	FORCEINLINE Vector2f Vector2f::operator+
	(
		const float A
	)	const
	{
		return Vector2f
		(
			X + A, 
			Y + A
		);
	}


	FORCEINLINE Vector2f Vector2f::operator-
	(
		const float A
	)	const
	{
		return Vector2f
		(
			X - A, 
			Y - A
		);
	}


	FORCEINLINE Vector2f Vector2f::operator*
	(
		const Vector2f & V
	)	const
	{
		return Vector2f
		(
			X * V.X, 
			Y * V.Y
		);
	}


	FORCEINLINE Vector2f Vector2f::operator/
	(
		const Vector2f & V
	)	const
	{
		return Vector2f
		(
			X / V.X, 
			Y / V.Y
		);
	}


	FORCEINLINE float Vector2f::operator|
	(
		const Vector2f & V
	)	const
	{
		return 
			X * V.X + 
			Y * V.Y;
	}


	FORCEINLINE float Vector2f::operator^
	(
		const Vector2f & V
	)	const
	{
		return 
			X * V.Y - 
			Y * V.X;
	}


	FORCEINLINE float Vector2f::DotProduct
	(
		const Vector2f & A, 
		const Vector2f & B
	)
	{
		return A | B;
	}


	FORCEINLINE float Vector2f::DistSquared
	(
		const Vector2f & V1, 
		const Vector2f & V2
	)
	{
		return 
			Math::Square(V2.X - V1.X) + 
			Math::Square(V2.Y - V1.Y);
	}


	FORCEINLINE float Vector2f::Distance
	(
		const Vector2f & V1, 
		const Vector2f & V2
	)
	{
		return Math::Sqrt(Vector2f::DistSquared(V1, V2));
	}


	FORCEINLINE float Vector2f::CrossProduct
	(
		const Vector2f & A, 
		const Vector2f & B
	)
	{
		return A ^ B;
	}


	FORCEINLINE bool Vector2f::operator==
	(
		const Vector2f & V
	)	const
	{
		return 
			X == V.X && 
			Y == V.Y;
	}


	FORCEINLINE bool Vector2f::operator!=
	(
		const Vector2f & V
	)	const
	{
		return 
			X != V.X || 
			Y != V.Y;
	}


	FORCEINLINE bool Vector2f::operator<
	(
		const Vector2f & Other
	)	const
	{
		return 
			X < Other.X && 
			Y < Other.Y;
	}


	FORCEINLINE bool Vector2f::operator>
	(
		const Vector2f & Other
	)	const
	{
		return 
			X > Other.X && 
			Y > Other.Y;
	}


	FORCEINLINE bool Vector2f::operator<=
	(
		const Vector2f & Other
	)	const
	{
		return 
			X <= Other.X && 
			Y <= Other.Y;
	}


	FORCEINLINE bool Vector2f::operator>=
	(
		const Vector2f & Other
	)	const
	{
		return 
			X >= Other.X && 
			Y >= Other.Y;
	}


	FORCEINLINE bool Vector2f::Equals
	(
		const	Vector2f & V, 
		const	float	   Tolerance
	)	const
	{
		return 
			Math::Abs(X - V.X) <= Tolerance && 
			Math::Abs(Y - V.Y) <= Tolerance;
	}


	FORCEINLINE Vector2f Vector2f::operator-() const
	{
		return Vector2f
		(
			-X, 
			-Y
		);
	}


	FORCEINLINE Vector2f Vector2f::operator+=
	(
		const Vector2f & V
	)
	{
		X += V.X; 
		Y += V.Y;
		return *this;
	}


	FORCEINLINE Vector2f Vector2f::operator-=
	(
		const Vector2f & V
	)
	{
		X -= V.X; 
		Y -= V.Y;
		return *this;
	}


	FORCEINLINE Vector2f Vector2f::operator*=
	(
		const float Scale
	)
	{
		X *= Scale; 
		Y *= Scale;
		return *this;
	}


	FORCEINLINE Vector2f Vector2f::operator/=
	(
		const float V
	)
	{
		const float RV = 1.f / V;
		X *= RV; 
		Y *= RV;
		return *this;
	}


	FORCEINLINE Vector2f Vector2f::operator*=
	(
		const Vector2f & V
	)
	{
		X *= V.X; 
		Y *= V.Y;
		return *this;
	}


	FORCEINLINE Vector2f Vector2f::operator/=
	(
		const Vector2f & V
	)
	{
		X /= V.X; 
		Y /= V.Y;
		return *this;
	}


	FORCEINLINE float& Vector2f::operator[]
	(
		const int32 Index
	)
	{
		return ((Index == 0) ? X : Y);
	}


	FORCEINLINE float Vector2f::operator[]
	(
		const int32 Index
	)	const
	{
		return ((Index == 0) ? X : Y);
	}


	FORCEINLINE void Vector2f::Set
	(
		const float InX,
		const float InY
	)
	{
		X = InX;
		Y = InY;
	}


	FORCEINLINE float Vector2f::GetMax() const
	{
		return Math::Max(X, Y);
	}


	FORCEINLINE float Vector2f::GetAbsMax() const
	{
		return Math::Max(
			Math::Abs(X), 
			Math::Abs(Y));
	}


	FORCEINLINE float Vector2f::GetMin() const
	{
		return Math::Min(X, Y);
	}


	FORCEINLINE float Vector2f::Size() const
	{
		return Math::Sqrt(
			X * X + 
			Y * Y);
	}


	FORCEINLINE float Vector2f::SizeSquared() const
	{
		return 
			X * X + 
			Y * Y;
	}


	FORCEINLINE Vector2f Vector2f::GetRotated
	(
		const float AngleDeg
	)	const
	{
		float S, C;
		{
			Math::SinCos(&S, &C, Math::DegreesToRadians(AngleDeg));
		}

		const float OMC = 1.0f - C;

		return Vector2f
		(
			C * X - S * Y,
			S * X + C * Y
		);
	}


	FORCEINLINE Vector2f Vector2f::GetSafeNormal
	(
		const float Tolerance
	)	const
	{
		const float SquareSum = X * X + Y * Y;

		if (SquareSum > Tolerance)
		{
			const float Scale = Math::InvSqrt(SquareSum);

			return Vector2f
			(
				X * Scale, 
				Y * Scale
			);
		}

		return Vector2f(0.f, 0.f);
	}

	FORCEINLINE void Vector2f::Normalize
	(
		const float Tolerance
	)
	{
		const float SquareSum = X * X + Y * Y;

		if (SquareSum > Tolerance)
		{
			const float Scale = Math::InvSqrt(SquareSum);

			X *= Scale;
			Y *= Scale;
		}
		else
		{
			X = 0.0f;
			Y = 0.0f;
		}
	}

	FORCEINLINE void Vector2f::ToDirectionAndLength
	(
		Vector2f & OutDir, 
		float	 & OutLength
	)	const
	{
		OutLength = Size();

		if (OutLength > FLT_EPSILON)
		{
			float OneOverLength = 1.0f / OutLength;

			OutDir = Vector2f
			( 
				X * OneOverLength, 
				Y * OneOverLength
			);
		}
		else
		{
			OutDir = Vector2f::ZeroVector;
		}
	}


	FORCEINLINE bool Vector2f::IsNearlyZero
	(
		const float Tolerance
	)	const
	{
		return	Math::Abs(X) <= Tolerance
			&&	Math::Abs(Y) <= Tolerance;
	}


	FORCEINLINE bool Vector2f::IsZero() const
	{
		return 
			X == 0.f && 
			Y == 0.f;
	}


	FORCEINLINE float& Vector2f::Component
	(
		const int32 Index
	)
	{
		return (&X)[Index];
	}


	FORCEINLINE float Vector2f::Component
	(
		const int32 Index
	)	const
	{
		return (&X)[Index];
	}


	FORCEINLINE Hyper::IntPoint Vector2f::AsIntPoint() const
	{
		return Hyper::IntPoint
		(
			Math::RoundToInt(X), 
			Math::RoundToInt(Y)
		);
	}


	FORCEINLINE Vector2f Vector2f::ClampAxes
	(
		const float MinAxisVal, 
		const float MaxAxisVal
	)	const
	{
		return Vector2f
		(
			Math::Clamp(X, MinAxisVal, MaxAxisVal), 
			Math::Clamp(Y, MinAxisVal, MaxAxisVal)
		);
	}


	FORCEINLINE Vector2f Vector2f::GetSignVector() const
	{
		return Vector2f
		(
			Math::FloatSelect(X, 1.f, -1.f),
			Math::FloatSelect(Y, 1.f, -1.f)
		);
	}

	FORCEINLINE Vector2f Vector2f::GetAbs() const
	{
		return Vector2f
		(
			Math::Abs(X), 
			Math::Abs(Y)
		);
	}
}