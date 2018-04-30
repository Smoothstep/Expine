#pragma once

#include "Vector4.h"
#include "Float16.h"

namespace Hyper
{
	struct RGColor
	{
		static const RGColor Black;
		static const RGColor White;

		union
		{
			Float2 ColorArray;

			struct
			{
				Float R, G;
			};

			Vector2f ColorVector;
		};

		RGColor() {}
		RGColor
		(
			const Float R,
			const Float G
		)
		{
			this->R = R;
			this->G = G;
		}
		RGColor
		(
			const Vector2f & V
		)
		{
			this->ColorVector = V;
		}
		RGColor
		(
			const Float F
		)
		{
			this->R = this->G = F;
		}
	};

	struct RGBColor
	{
		static const RGBColor Black;
		static const RGBColor White;

		union
		{
			Float3 ColorArray;

			struct
			{
				Float R, G, B;
			};

			Vector3f ColorVector;
		};

		RGBColor() {}
		RGBColor
		(
			const Float R,
			const Float G,
			const Float B
		)
		{
			this->R = R;
			this->G = G;
			this->B = B;
		}
		RGBColor
		(
			const Vector3f & V
		)
		{
			this->ColorVector = V;
		}
		RGBColor
		(
			const Float F
		)
		{
			this->R = this->G = this->B = F;
		}
	};

	struct RGBColor16
	{
		static const RGBColor Black;
		static const RGBColor White;

		union
		{
			Float16 ColorArray[3];

			struct
			{
				Float16 R, G, B;
			};
		};

		RGBColor16() {}
		RGBColor16
		(
			const Float16 R,
			const Float16 G,
			const Float16 B
		)
		{
			this->R = R;
			this->G = G;
			this->B = B;
		}

		RGBColor16
		(
			const Vector3f & V
		)
		{
			R = V.X;
			G = V.Y;
			B = V.Z;
		}

		RGBColor16
		(
			const Float16 F
		)
		{
			this->R = this->G = this->B = F;
		}
	};

	struct RGBAColor
	{
		static const RGBAColor Black;
		static const RGBAColor White;
		static const RGBAColor Transparent;

		union
		{
			Float4 ColorArray;

			struct
			{
				Float R, G, B, A;
			};

			Vector4f ColorVector;
		};

		inline RGBAColor() {}
		inline RGBAColor
		(
			const Float R,
			const Float G,
			const Float B,
			const Float A
		)
		{
			this->R = R;
			this->G = G;
			this->B = B;
			this->A = A;
		}
		inline RGBAColor
		(
			const Vector4f & V
		)
		{
			this->ColorVector = V;
		}
		inline RGBAColor
		(
			const Float F
		)
		{
			this->R = this->G = this->B = this->A = F;
		}
	}; 
	
	struct RGBAColor16
	{
		static const RGBAColor Black;
		static const RGBAColor White;
		static const RGBAColor Transparent;

		union
		{
			Float16 ColorArray[4];

			struct
			{
				Float16 R, G, B, A;
			};
		};

		inline RGBAColor16() {}
		inline RGBAColor16
		(
			const Float16 R,
			const Float16 G,
			const Float16 B,
			const Float16 A
		)
		{
			this->R = R;
			this->G = G;
			this->B = B;
			this->A = A;
		}

		inline RGBAColor16
		(
			const Float16 F
		)
		{
			this->R = this->G = this->B = this->A = F;
		}

		inline RGBAColor16
		(
			const Vector4f& V
		)
		{
			this->R = V.X;
			this->G = V.Y;
			this->B = V.Z;
			this->A = V.W;
		}
	};
}