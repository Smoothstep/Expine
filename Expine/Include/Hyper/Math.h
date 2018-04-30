#pragma once

#include <d3d12.h>

#include "Defines.h"
#include "MathDefines.h"
#include "Point.h"
#include "Types.h"
#include "SSE.h"

namespace Hyper
{
	struct RGColor;
	struct RGColor16;
	struct RGBColor;
	struct RGBColor16;
	struct RGBAColor;
	struct RGBAColor16;

	struct Vector2f;
	struct Vector3f;
	struct Vector4f;
	struct Plane;
	struct Quaternion;
	struct Matrix4x4;
	struct Rotation;

	namespace Math
	{
		static constexpr int Power10LookupTable[10] =
		{
			1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000
		};

		static constexpr double FPower10LookupTable[10] =
		{
			1, 0.1, 0.01, 0.001, 0.0001, 0.00001, 0.000001, 0.0000001, 0.00000001, 0.000000001
		};

		namespace SSE
		{
			static FORCEINLINE M128 MakeVectorRegister
			(
				const float X,
				const float Y,
				const float Z,
				const float W
			)
			{
				return _mm_setr_ps(X, Y, Z, W);
			}

			static FORCEINLINE M128i MakeVectorRegisterInt
			(
				const int32 X,
				const int32 Y,
				const int32 Z,
				const int32 W
			)
			{
				return _mm_setr_epi32(X, Y, Z, W);
			}

			static FORCEINLINE M128 MakeVectorRegister
			(
				uint32 X,
				uint32 Y,
				uint32 Z,
				uint32 W
			)
			{
				union
				{
					M128  v;
					M128i i;
				} Tmp;

				Tmp.i = _mm_setr_epi32(X, Y, Z, W);

				return Tmp.v;
			}

			static const M128 QMULTI_SIGN_MASK0 = MakeVectorRegister( 1.f,-1.f, 1.f,-1.f);
			static const M128 QMULTI_SIGN_MASK1 = MakeVectorRegister( 1.f, 1.f,-1.f,-1.f);
			static const M128 QMULTI_SIGN_MASK2 = MakeVectorRegister(-1.f, 1.f, 1.f,-1.f);

			static FORCEINLINE M128 VectorQuaternionMultiply2
			(
				const M128 & Quat1, 
				const M128 & Quat2
			)
			{
				M128 Result = VectorMultiply(VectorReplicate(Quat1, 3), Quat2);

				Result = VectorMultiplyAdd(VectorMultiply(VectorReplicate(Quat1, 0), VectorSwizzle(Quat2, 3, 2, 1, 0)), QMULTI_SIGN_MASK0, Result);
				Result = VectorMultiplyAdd(VectorMultiply(VectorReplicate(Quat1, 1), VectorSwizzle(Quat2, 2, 3, 0, 1)), QMULTI_SIGN_MASK1, Result);
				Result = VectorMultiplyAdd(VectorMultiply(VectorReplicate(Quat1, 2), VectorSwizzle(Quat2, 1, 0, 3, 2)), QMULTI_SIGN_MASK2, Result);

				return Result;
			}

			static FORCEINLINE void VectorQuaternionMultiply
			(
						void * RESTRICT Result, 
				const	void * RESTRICT Quat1, 
				const	void * RESTRICT Quat2)
			{
				*reinterpret_cast<M128*>(Result) = VectorQuaternionMultiply2
				(
					*reinterpret_cast<const M128 *>(Quat1),
					*reinterpret_cast<const M128 *>(Quat2)
				);
			}

			static FORCEINLINE M128 VectorSelect
			(
				const M128 & Mask, 
				const M128 & Vec1,
				const M128 & Vec2
			)
			{
				return _mm_xor_ps(Vec2, _mm_and_ps(Mask, _mm_xor_ps(Vec1, Vec2)));
			}

			static FORCEINLINE M128 VectorDot3
			(
				const M128 & Vec1, 
				const M128 & Vec2
			)
			{
				M128 Temp = VectorMultiply(Vec1, Vec2);
				return VectorAdd(VectorReplicate(Temp, 0), VectorAdd(VectorReplicate(Temp, 1), VectorReplicate(Temp, 2)));
			}

			static FORCEINLINE M128 VectorDot4
			(
				const M128 & Vec1, 
				const M128 & Vec2
			)
			{
				M128 Temp1, Temp2;
				Temp1 = VectorMultiply(Vec1, Vec2);
				Temp2 = _mm_shuffle_ps(Temp1, Temp1, SHUFFLEMASK(2, 3, 0, 1));
				Temp1 = VectorAdd(Temp1, Temp2);
				Temp2 = _mm_shuffle_ps(Temp1, Temp1, SHUFFLEMASK(1, 2, 3, 0));
				return VectorAdd(Temp1, Temp2);
			}

			static FORCEINLINE M128 VectorReciprocalLen
			(
				const M128 & Vector
			)
			{
				M128 RecipLen = VectorDot4(Vector, Vector);
				return VectorReciprocalSqrt(RecipLen);
			}

			static FORCEINLINE M128 VectorReciprocalSqrtAccurate
			(
				const M128 & Vec
			)
			{
				const M128 OneHalf = ::SSE::FloatOneHalf;
				const M128 VecDivBy2 = VectorMultiply(Vec, OneHalf);

				const M128 x0 = VectorReciprocalSqrt(Vec);

				M128 x1 = VectorMultiply(x0, x0);
				x1 = VectorSubtract(OneHalf, VectorMultiply(VecDivBy2, x1));
				x1 = VectorMultiplyAdd(x0, x1, x0);

				M128 x2 = VectorMultiply(x1, x1);
				x2 = VectorSubtract(OneHalf, VectorMultiply(VecDivBy2, x2));
				x2 = VectorMultiplyAdd(x1, x2, x1);

				return x2;
			}

			static FORCEINLINE M128 VectorReciprocalAccurate
			(
				const M128 & Vec
			)
			{
				const M128 x0 = VectorReciprocal(Vec);

				const M128 x0Squared = VectorMultiply(x0, x0);
				const M128 x0Times2 = VectorAdd(x0, x0);
				const M128 x1 = VectorSubtract(x0Times2, VectorMultiply(Vec, x0Squared));

				const M128 x1Squared = VectorMultiply(x1, x1);
				const M128 x1Times2 = VectorAdd(x1, x1);
				const M128 x2 = VectorSubtract(x1Times2, VectorMultiply(Vec, x1Squared));

				return x2;
			}

			static FORCEINLINE M128 VectorNormalizeSafe
			(
				const M128 & Vector,
				const M128 & VectorDefaultValue
			)
			{
				const M128 SquareSum = VectorDot4(Vector, Vector);
				const M128 NonZeroMask = VectorCompareGE(SquareSum, ::SSE::SmallLengthThreshold);
				const M128 InvLength = VectorReciprocalSqrtAccurate(SquareSum);
				const M128 NormalizedVector = VectorMultiply(InvLength, Vector);
				return VectorSelect(NonZeroMask, NormalizedVector, VectorDefaultValue);
			}

			static FORCEINLINE M128 VectorNormalizeQuaternion
			(
				const M128 & UnnormalizedQuat
			)
			{
				return VectorNormalizeSafe(UnnormalizedQuat, ::SSE::Float0001);
			}

			static FORCEINLINE M128 VectorCross
			(
				const M128 & Vec1,
				const M128 & Vec2
			)
			{
				M128 A_YZXW = _mm_shuffle_ps(Vec1, Vec1, SHUFFLEMASK(1, 2, 0, 3));
				M128 B_ZXYW = _mm_shuffle_ps(Vec2, Vec2, SHUFFLEMASK(2, 0, 1, 3));
				M128 A_ZXYW = _mm_shuffle_ps(Vec1, Vec1, SHUFFLEMASK(2, 0, 1, 3));
				M128 B_YZXW = _mm_shuffle_ps(Vec2, Vec2, SHUFFLEMASK(1, 2, 0, 3));
				return VectorSubtract(VectorMultiply(A_YZXW, B_ZXYW), VectorMultiply(A_ZXYW, B_YZXW));
			}

			static FORCEINLINE M128 VectorPow
			(
				const M128 & Base, 
				const M128 & Exponent
			)
			{
				union 
				{ 
					M128 v; 
					Float4 f; 
				} B, E;
				B.v = Base;
				E.v = Exponent;
				return _mm_setr_ps(powf(B.f[0], E.f[0]), powf(B.f[1], E.f[1]), powf(B.f[2], E.f[2]), powf(B.f[3], E.f[3]));
			}
		}

		template
		<
			typename IntegerType
		>
		static FORCEINLINE IntegerType PreviousPowerOfTwo
		(
			IN	IntegerType Integer
		)
		{
			Integer |= (Integer >> 1);
			Integer |= (Integer >> 2);
			Integer |= (Integer >> 4);
			Integer |= (Integer >> 8);
			Integer |= (Integer >> 16);
			return Integer - (Integer >> 1);
		}

		template
		<
			typename IntegerType
		>
		static FORCEINLINE IntegerType NextPowerOfTwo
		(
			IN	IntegerType Integer
		)
		{
			Integer--;
			Integer |= (Integer >> 1);
			Integer |= (Integer >> 2);
			Integer |= (Integer >> 4);
			Integer |= (Integer >> 8);
			Integer |= (Integer >> 16);
			Integer++;
			return Integer - (Integer >> 1);
		}

		static CONSTEXPR FORCEINLINE Float TruncateToFloat
		(
			IN	const int Integer
		)
		{
			return static_cast<float>(Integer);
		}

		static CONSTEXPR FORCEINLINE Int TruncateToInt
		(
			IN	const Float FloatingPoint
		)
		{
			return static_cast<int>(FloatingPoint);
		}

		static CONSTEXPR FORCEINLINE Float TruncateToFloat
		(
			IN	const Float FloatingPoint
		)
		{
			return
				static_cast<float>(
					static_cast<int>(FloatingPoint));
		}

		template<typename F> static FORCEINLINE F Floor
		(
			const F Value
		)
		{
			return std::floor(Value);
		}

		template<typename F> static FORCEINLINE F Ceil
		(
			const F Value
		)
		{
			return std::ceil(Value);
		}

		static FORCEINLINE Float GridSnap
		(
			IN	const Float Location,
			IN	const Float Grid
		)
		{
			if (Grid == 0.0f)
			{
				return Location;
			}

			return Floor(static_cast<float>((Location + 0.5 * Grid) / Grid)) * Grid;
		}

		static FORCEINLINE Float Log2
		(
			IN	const Float Value
		)
		{
			return log2f(Value);
		}

		template
		<
			class T
		>
		static CONSTEXPR FORCEINLINE T Square
		(
			IN	const T A
		)
		{
			return A * A;
		}

		template
		<
			class T
		>
		static CONSTEXPR FORCEINLINE T Clamp
		(
			IN	const T X,
			IN	const T Min,
			IN	const T Max
		)
		{
			return	X < Min ? Min : 
					X < Max ? X : Max;
		}

		template
		<
			typename T
		>
		static FORCEINLINE int RoundToInt
		(
			IN	const T FloatingPoint
		)
		{
			int Value;
#if defined(__ICC) || defined(__INTEL_COMPILER)
			RoundFP(FloatingPoint, Value);
#else
			Value = _mm_cvt_ss2si(_mm_set_ss(FloatingPoint + FloatingPoint + 0.5f)) >> 1;
#endif
			return Value;
		}

		static FORCEINLINE float Abs
		(
			IN	const Float FloatingPoint
		)
		{
			return fabsf(FloatingPoint);
		}

		static FORCEINLINE float Sqrt
		(
			IN	const Float FloatingPoint
		)
		{
			return sqrtf(FloatingPoint);
		}

		static FORCEINLINE float Exp2
		(
			IN	const Float FloatingPoint
		)
		{
			return powf(2.0f, FloatingPoint);
		}

		static FORCEINLINE float Exp
		(
			IN	const Float FloatingPoint
		)
		{
			return expf(FloatingPoint);
		}

		static CONSTEXPR FORCEINLINE float FloatSelect
		(
			IN	const Float Comparand,
			IN	const Float ValueGEZero,
			IN	const Float ValueLTZero
		)
		{
			return Comparand >= 0.f ? ValueGEZero : ValueLTZero;
		}

		template
		<
			typename T
		>
		static CONSTEXPR FORCEINLINE T Max
		(
			IN	const T L,
			IN	const T R
		)
		{
			return L < R ? R : L;
		}

		template
		<
			typename T
		>
		static CONSTEXPR FORCEINLINE T Min
		(
			IN	const T L,
			IN	const T R
		)
		{
			return L > R ? R : L;
		}

		template
		<
			class T
		>
		static FORCEINLINE auto RadiansToDegrees
		(
			IN	T const& RadVal
		) 
			-> decltype(RadVal * (180.0 / PI))
		{
			return RadVal * (180.0 / PI);
		}

		template
		<
			class T
		>
		static FORCEINLINE auto DegreesToRadians
		(
			IN	T const& DegVal
		) 
			-> decltype(DegVal * (PI / 180.0))
		{
			return DegVal * (PI / 180.0);
		}

		static FORCEINLINE void SinCos
		(
			OUT			Float *	ScalarSin,
			OUT			Float *	ScalarCos,
			IN	const	Float	Value)
		{
			float quotient = (INV_PI*0.5f)*Value;

			if (Value >= 0.0f)
			{
				quotient = (float)((int)(quotient + 0.5f));
			}
			else
			{
				quotient = (float)((int)(quotient - 0.5f));
			}

			float y = Value - (2.0f * PI) * quotient;
			float sign;

			if (y > HALF_PI)
			{
				y = PI - y;
				sign = -1.0f;
			}
			else if (y < -HALF_PI)
			{
				y = -PI - y;
				sign = -1.0f;
			}
			else
			{
				sign = +1.0f;
			}

			float y2 = y * y;
			*ScalarSin = (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 - 0.16666667f) * y2 + 1.0f) * y;

			float p = ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f;
			*ScalarCos = sign*p;
		}

		static FORCEINLINE Float InvSqrt
		(
			IN	const Float F
		)
		{
			const M128 fOneHalf = _mm_set_ss(0.5f);
			M128 Y0, X0, X1, X2, FOver2;
			float temp;

			Y0 = _mm_set_ss(F);
			X0 = _mm_rsqrt_ss(Y0);
			FOver2 = _mm_mul_ss(Y0, fOneHalf);

			X1 = _mm_mul_ss(X0, X0);
			X1 = _mm_sub_ss(fOneHalf, _mm_mul_ss(FOver2, X1));
			X1 = _mm_add_ss(X0, _mm_mul_ss(X0, X1));

			X2 = _mm_mul_ss(X1, X1);
			X2 = _mm_sub_ss(fOneHalf, _mm_mul_ss(FOver2, X2));
			X2 = _mm_add_ss(X1, _mm_mul_ss(X1, X2));

			_mm_store_ss(&temp, X2);
			return temp;
		}

		static FORCEINLINE Float InvSqrtEst
		(
			IN	const Float F
		)
		{
			const M128 fOneHalf = _mm_set_ss(0.5f);
			M128 Y0, X0, X1, FOver2;
			float temp;

			Y0 = _mm_set_ss(F);
			X0 = _mm_rsqrt_ss(Y0);

			FOver2 = _mm_mul_ss(Y0, fOneHalf);

			X1 = _mm_mul_ss(X0, X0);
			X1 = _mm_sub_ss(fOneHalf, _mm_mul_ss(FOver2, X1));
			X1 = _mm_add_ss(X0, _mm_mul_ss(X0, X1));

			_mm_store_ss(&temp, X1);
			return temp;
		}

		template
		<
			class MatrixType
		>
		FORCEINLINE M128 VectorTransformVector
		(
			IN	const M128			& VecP,
			IN	const MatrixType	& MatrixM
		)
		{
			M128 VTempX, VTempY, VTempZ, VTempW;

			const M128 *M = (const M128 *)&MatrixM;

			VTempX = VectorReplicate(VecP, 0);
			VTempY = VectorReplicate(VecP, 1);
			VTempZ = VectorReplicate(VecP, 2);
			VTempW = VectorReplicate(VecP, 3);

			VTempX = VectorMultiply(VTempX, M[0]);
			VTempY = VectorMultiply(VTempY, M[1]);
			VTempZ = VectorMultiply(VTempZ, M[2]);
			VTempW = VectorMultiply(VTempW, M[3]);

			VTempX = VectorAdd(VTempX, VTempY);
			VTempZ = VectorAdd(VTempZ, VTempW);
			VTempX = VectorAdd(VTempX, VTempZ);

			return VTempX;
		}

		FORCEINLINE M256 TwoLincomb_AVX_8
		(
			IN	const M128	 Source[4],
			IN	const M256 & A01
		)
		{
			M256 Result;

			Result = _mm256_mul_ps(_mm256_shuffle_ps(A01, A01, 0x00), _mm256_broadcast_ps(&Source[0]));
			{
				Result = _mm256_add_ps(Result, _mm256_mul_ps(_mm256_shuffle_ps(A01, A01, 0x55), _mm256_broadcast_ps(&Source[1])));
				Result = _mm256_add_ps(Result, _mm256_mul_ps(_mm256_shuffle_ps(A01, A01, 0xaa), _mm256_broadcast_ps(&Source[2])));
				Result = _mm256_add_ps(Result, _mm256_mul_ps(_mm256_shuffle_ps(A01, A01, 0xff), _mm256_broadcast_ps(&Source[3])));
			}

			return Result;
		}

		static FORCEINLINE Float FMod
		(
			IN	const Float X,
			IN	const Float Y
		)
		{
			return fmodf(X, Y);
		}

		static FORCEINLINE Float UnwindDegrees
		(
			IN	Float A
		)
		{
			A = FMod(A, 360.f);

			if (A > 180.f)
			{
				A -= 360.f;
			}

			if (A < -180.f)
			{
				A += 360.f;
			}

			return A;
		}

		static FORCEINLINE Float Cos
		(
			IN	const Float F
		)
		{
			return cosf(F);
		}

		static FORCEINLINE Float Sin
		(
			IN	const Float F
		)
		{
			return sinf(F);
		}

		static FORCEINLINE Float Acos
		(
			IN	const Float Value
		) 
		{ 
			return acosf((Value<-1.f) ? -1.f : ((Value<1.f) ? Value : 1.f)); 
		}

		static FORCEINLINE float Tan
		(
			IN	const float Value
		) 
		{ 
			return tanf(Value); 
		}

		static FORCEINLINE float Atan
		(
			IN	const float Value
		) 
		{ 
			return atanf(Value); 
		}

		static FORCEINLINE float Atan2
		(
			IN	const float Y,
			IN	const float X
		)
		{
			const float absX = Abs(X);
			const float absY = Abs(Y);

			const bool yAbsBigger = (absY > absX);

			float t0 = yAbsBigger ? absY : absX;
			float t1 = yAbsBigger ? absX : absY;

			if (t0 == 0.0f)
			{
				return 0.0f;
			}

			float t3 = t1 / t0;
			float t4 = t3 * t3;

			static constexpr float c[7] =
			{
				+7.2128853633444123e-03f,
				-3.5059680836411644e-02f,
				+8.1675882859940430e-02f,
				-1.3374657325451267e-01f,
				+1.9856563505717162e-01f,
				-3.3324998579202170e-01f,
				+1.0f
			};

			t0 = c[0];
			t0 = t0 * t4 + c[1];
			t0 = t0 * t4 + c[2];
			t0 = t0 * t4 + c[3];
			t0 = t0 * t4 + c[4];
			t0 = t0 * t4 + c[5];
			t0 = t0 * t4 + c[6];
			t3 = t0 * t3;

			t3 = yAbsBigger ? (0.5f * PI) - t3 : t3;
			t3 = (X < 0.0f) ? PI - t3 : t3;
			t3 = (Y < 0.0f) ? -t3 : t3;

			return t3;
		}

		static FORCEINLINE float FastAsin(float Value)
		{
			float x = Abs(Value);

			float omx = 1.0f - x;

			if (omx < 0.0f)
			{
				omx = 0.0f;
			}

			float result = ((((((
				-0.0012624911f * 
				x + 0.0066700901f) * 
					x - 0.0170881256f) * 
						x + 0.0308918810f) * 
							x - 0.0501743046f) * 
								x + 0.0889789874f) * 
									x - 0.2145988016f) * x + FASTASIN_HALF_PI;

			result *= Sqrt(omx);

			return (Value >= 0.0f ? FASTASIN_HALF_PI - result : result - FASTASIN_HALF_PI);
		}

		template 
		<
			class T
		>
		static FORCEINLINE T DivideAndRoundUp(T Dividend, T Divisor)
		{
			return (Dividend + Divisor - 1) / Divisor;
		}

		static FORCEINLINE Float FastPow
		(
			const Float A,
			const Float B
		)
		{
			union
			{
				Float F;
				int X[2];
			} U = { A };

			U.X[1] = static_cast<int>(B * (U.X[1] - 1072632447) + 1072632447);
			U.X[0] = 0;

			return U.F;
		}

		static FORCEINLINE Float Pow
		(
			IN	const Float A,
			IN	const Float B
		)
		{
#ifndef _DEBUG
			return FastPow(A, B);
#else
			return powf(A, B);
#endif
		}

		static FORCEINLINE bool IsNaN
		(
			IN	const Float F
		) 
		{ 
			return _isnan(F) != 0; 
		}

		static FORCEINLINE bool IsFinite
		(
			IN	const Float F
		) 
		{ 
			return _finite(F) != 0; 
		}

		static FORCEINLINE Float Frac
		(
			IN	const Float N
		)
		{
			return N - std::floor(N);
		}

		static FORCEINLINE Float Hash
		(
			IN	const Float N
		)
		{
			return Frac(sin(N) * 1e4);
		}

		static FORCEINLINE Float Lerp
		(
			IN	const Float Start,
			IN	const Float End,
			IN	const Float Percent
		)
		{
			return (Start + Percent * (End - Start));
		}

		static FORCEINLINE Vector3f Floor
		(
			IN	const Vector3f & V
		);

		static FORCEINLINE Vector3f Frac
		(
			IN	const Vector3f & V
		);

		static FORCEINLINE Float IqHash
		(
			IN	const Float n
		)
		{
			return Frac(sin(n) * 43758.5453);
		}

		Float GenerateNoise
		(
			IN	const Vector3f & Position
		);

		Float GenerateNoise2
		(
			IN	const Vector3f & Position
		);

		template<typename T> static FORCEINLINE T Random()
		{
			static size_t Seed = 1;
			T Value = static_cast<T>((Seed ^= 0x3F800000) +
				*reinterpret_cast<unsigned int*>(0x7FFE0008));

			return Value;
		}

		static FORCEINLINE Float RandomNoise()
		{
			static size_t Seed = 1;
			Seed ^= *reinterpret_cast<unsigned int*>(0x7FFE0008) ^ 0x3F800000 + (Seed << 6) + (Seed >> 2);

			Float Value;
			*reinterpret_cast<int32_t*>(&Value) = (0x7E << 23) | (Seed & 0x7FFFFF);
			
			return 2.0f * Value - 1.0f;
		}

		static FORCEINLINE Float RandomNoiseGtZero()
		{
			Float Noise = RandomNoise();

			if (Noise == 0)
			{
				return FLT_EPSILON;
			}

			return Noise;
		}

		template
		<
			typename T, std::enable_if_t<sizeof(T) == sizeof(float), size_t> = 0
		> 
		static inline T Rcp
		(
			const T F
		)
		{
			if (F == 0.0f)
			{
				return FLT_MAX;
			}
			else
			{
				if (F != 1.0)
				{
					return 1 / F;
				}
			}

			return F;
		}

		template
		<
			typename T, std::enable_if_t<sizeof(T) == sizeof(double), size_t> = 0
		> 
		static inline T Rcp
		(
			const T F
		)
		{
			if (F == 0.0f)
			{
				return FLT_MAX;
			}
			else
			{
				if (F != 1.0)
				{
					return 1 / F;
				}
			}

			return F;
		}

		static Vector3f ClosestPointOnLine
		(
			const Vector3f & LineStart, 
			const Vector3f & LineEnd, 
			const Vector3f & Point
		);

		static Vector3f ClosestPointOnInfiniteLine
		(
			const Vector3f & LineStart, 
			const Vector3f & LineEnd, 
			const Vector3f & Point
		);

		static Vector3f LinePlaneIntersection
		(
			const Vector3f	& Point1,
			const Vector3f	& Point2,
			const Plane		& Plane
		);

		static bool IntersectPlanes3
		(
					Vector3f& I,
			const	Plane	& P1,
			const	Plane	& P2,
			const	Plane	& P3
		);

		static bool IntersectPlanes2
		(
					Vector3f & I,
					Vector3f & D, 
			const	Plane	 & P1, 
			const	Plane	 & P2
		);

		static bool Intersects
		(
			const	Vector2f &	Object,
			const	Vector2f &	Source,
			const	Vector2f &	Destination,
			const	Float		LineSize,
			const	Float		ObjectSize,
					Vector2f *	Intersection0,
					Vector2f *	Intersection1
		);

		template
		<
			class U
		>
		FORCEINLINE Rotation Lerp
		(
			const	Rotation & A,
			const	Rotation & B,
			const	U		 & Alpha
		);

		template
		<
			class U
		>
		FORCEINLINE Rotation LerpRange
		(
			const	Rotation & A,
			const	Rotation & B,
			const	U		 & Alpha
		);

		template
		<
			class U
		>
		FORCEINLINE Quaternion Lerp
		(
			const	Quaternion  & A,
			const	Quaternion  & B,
			const	U			& Alpha
		);

		template
		<
			class U
		>
		FORCEINLINE Quaternion BiLerp
		(
			const	Quaternion & P00,
			const	Quaternion & P10,
			const	Quaternion & P01,
			const	Quaternion & P11,
			const	float		 FracX,
			const	float		 FracY
		);

		template
		<
			class U
		>
		FORCEINLINE Quaternion CubicInterp
		(
			const	Quaternion	& P0,
			const	Quaternion	& T0,
			const	Quaternion	& P1,
			const	Quaternion	& T1,
			const	U			& A
		);
	}
}