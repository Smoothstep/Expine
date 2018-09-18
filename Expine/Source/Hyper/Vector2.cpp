#include "Hyper/Vector3.h"
#include "Hyper/Quaternion.h"

namespace Hyper
{
	const Vector2f Vector2f::ZeroVector = Vector2f(0.0, 0.0);

	static bool Math::Intersects
	(
		const	Vector2f &	Object,
		const	Vector2f &	Source,
		const	Vector2f &	Destination,
		const	Float		LineSize,
		const	Float		ObjectSize,
				Vector2f *	Intersection0,
				Vector2f *	Intersection1
	) 
	{
		Vector2f Delta = Source - Destination;
		Vector2f Normalized = Delta.GetSafeNormal();

		Float T = 
			Normalized.X * Delta.X + 
			Normalized.Y * Delta.Y;
		
		Vector2f E = T * Normalized + Source;
		Vector2f D = E - Object;

		Float Lec = D.SizeSquared();

		if (Lec < LineSize + ObjectSize)
		{
			Float DT = Math::Sqrt(ObjectSize * ObjectSize + Lec * Lec);

			if (Intersection0)
			{
				*Intersection0 = (T - DT) * Normalized + Source.X;
			}

			if (Intersection1)
			{
				*Intersection1 = (T + DT) * Normalized + Source.X;
			}

			return true;
		}

		return false;
	}
}