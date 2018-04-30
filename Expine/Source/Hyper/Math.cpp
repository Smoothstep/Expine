#include "../Precompiled.h"
#include "Math.h"
#include "Vector4.h"

namespace Hyper
{
	namespace Math
	{
		Float GenerateNoise(const Vector3f& Position)
		{
			Vector3f P = Floor(Position);
			Vector3f F = Frac(Position);

			F = F * F * (Vector3f(3.0) - Vector3f(2.0) * F);

			Float N = P.X + P.Y * 57.0 + 113.0 * P.Z;

			return
				Lerp(Lerp(Lerp(IqHash(N + 0.0), IqHash(N + 1.0), F.X),
					Lerp(IqHash(N + 57.0), IqHash(N + 58.0), F.X), F.Y),
					Lerp(Lerp(IqHash(N + 113.0), IqHash(N + 114.0), F.X),
						Lerp(IqHash(N + 170.0), IqHash(N + 171.0), F.X), F.Y), F.Z);
		}

		Float GenerateNoise2(const Vector3f& Position)
		{
			static const Vector3f Step(Math::RandomNoiseGtZero() * 255.0, Math::RandomNoiseGtZero() * 255.0, Math::RandomNoiseGtZero() * 255.0);

			const Vector3f I = Floor(Position);
			const Vector3f F = Frac(Position);

			const Float N = I | F;;
			const Vector3f u = F * F * (Vector3f(3.0) - Vector3f(2.0) * F);
			return
				Lerp(Lerp(Lerp(Hash(N + Vector3f::DotProduct(Step, Vector3f(0, 0, 0))), Hash(N + Vector3f::DotProduct(Step, Vector3f(1, 0, 0))), u.X),
					Lerp(Hash(N + Vector3f::DotProduct(Step, Vector3f(0, 1, 0))), Hash(N + Vector3f::DotProduct(Step, Vector3f(1, 1, 0))), u.X), u.Y),
					Lerp(Lerp(Hash(N + Vector3f::DotProduct(Step, Vector3f(0, 0, 1))), Hash(N + Vector3f::DotProduct(Step, Vector3f(1, 0, 1))), u.X),
						Lerp(Hash(N + Vector3f::DotProduct(Step, Vector3f(0, 1, 1))), Hash(N + Vector3f::DotProduct(Step, Vector3f(1, 1, 1))), u.X), u.Y), u.Z);
		}
	}
}