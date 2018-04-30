#pragma once

#include "ViewFrustum.h"

namespace D3D
{
	enum class IntersectionState
	{
		MightIntersect,
		NoIntersection,
		Intersects
	};

	class IIntersectible
	{
	public:
		virtual IntersectionState CheckIntersection
		(
			const Vector3f& Ray
		)	const = 0;


	};

	template<typename Geometry> class IBoxBehavior : public IIntersectible
	{
	public:
		virtual IntersectionState CheckIntersection
		(
			const Vector3f& Ray
		)	const override
		{
			const Vector3f& Position = reinterpret_cast<const Geometry*>(this)->GetPosition();
			const Vector3f& Extent = reinterpret_cast<const Geometry*>(this)->GetExtent();

			if (Math::Abs(Position.X - Ray.X) < Extent.X &&
				Math::Abs(Position.Y - Ray.Y) < Extent.Y)
			{
				return IntersectionState::Intersects;
			}
			
			return IntersectionState::NoIntersection;
		}
	};

	class BoxIteration
	{
	private:

		Int32 CurrentIndex = 1;
		Int32 Stage = 2;
		Int32 StageIter = 0;
		Int32 Width;

	public:

		inline void Next(Int32& X, Int32& Y)
		{
			if (CurrentIndex % Stage == 0)
			{
				if (StageIter == 2)
				{
					Stage *= Stage;
					StageIter = 0;
				}
				else
				{
					++StageIter;
				}
			}

			X = (-Stage + CurrentIndex++);
		}

		inline Int32 Last() const
		{
			return Width * Width;
		}
	};
}