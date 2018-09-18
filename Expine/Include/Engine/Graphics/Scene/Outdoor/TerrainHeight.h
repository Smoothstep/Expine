#pragma once

#include "DirectX/D3D.h"

namespace D3D
{
	namespace Terrain
	{
		enum EHeightMapDataType
		{
			UNKNOWN_FORMAT,
			RAW_8_BIT,
			RAW_16_BIT,
			RAW_32_BIT,
			RAW_16_BIT_FLOAT,
			RAW_32_BIT_FLOAT,
			RAW_64_BIT_FLOAT
		};

		static inline EHeightMapDataType GetHeightMapFormat
		(
			const String & Format
		)
		{
			if (Format == "U8")
			{
				return EHeightMapDataType::RAW_8_BIT;
			}
			if (Format == "U16")
			{
				return EHeightMapDataType::RAW_16_BIT;
			}
			if (Format == "U32")
			{
				return EHeightMapDataType::RAW_32_BIT;
			}
			if (Format == "F16")
			{
				return EHeightMapDataType::RAW_16_BIT_FLOAT;
			}
			if (Format == "F32")
			{
				return EHeightMapDataType::RAW_32_BIT_FLOAT;
			}
			if (Format == "F64")
			{
				return EHeightMapDataType::RAW_64_BIT_FLOAT;
			}

			return EHeightMapDataType::UNKNOWN_FORMAT;
		}

		class CHeight
		{
		private:

			Float	 *			HeightMap;
			Vector3f *			NormalMap;
			Vector3f *			TangentMap;

			TVector<Uint8>		Data;

			Int					SizeX;
			Int					SizeZ;

			Float				HeightScale;
			Float				MaxHeight;
			Float				MinHeight;
			EHeightMapDataType	HeightMapDataType;

			static constexpr Float DefaultScaling = 0.01f;

		protected:

			Float Filter = 0.5f;

		public:

			inline void SetHeightScale
			(
				const Float Scale
			)
			{
				HeightScale = Scale;
			}

			inline Int GetSizeX() const
			{
				return SizeX;
			}

			inline Int GetSizeZ() const
			{
				return SizeZ;
			}

			inline const Float * GetData() const
			{
				return HeightMap;
			}

			inline const Vector3f * GetNormals() const
			{
				return NormalMap;
			}

			inline const Vector3f * GetTangents() const
			{
				return TangentMap;
			}

			inline Float GetActualHeight
			(
				const Int X,
				const Int Z
			)	const
			{
				return HeightMap[X + Z * SizeX];
			}

			inline Float GetScaledHeight
			(
				const Int X,
				const Int Z
			)	const
			{
				return GetActualHeight(X, Z) * HeightScale;
			}

			inline Float GetInterpolatedHeight
			(
				const Float X,
				const Float Z
			)	const
			{
				Int iX = Math::TruncateToInt(X);
				Int iZ = Math::TruncateToInt(Z);

				Float Low = GetScaledHeight(iX, iZ);

				if (iX + 1 >= SizeX ||
					iZ + 1 >= SizeZ)
				{
					return Low;
				}

				Float HighX = GetScaledHeight(iX + 1, iZ + 0);
				Float HighZ = GetScaledHeight(iX + 0, iZ + 1);

				Int nX = ((HighX - Low) * (X - iX)) + Low;
				Int nZ = ((HighZ - Low) * (Z - iZ)) + Low;

				return (nX + nZ) / 2;
			}

		public:

			CHeight();
			CHeight
			(
				const IntPoint Size,
				const Float Scaling = DefaultScaling
			);
			CHeight
			(
				const Int SizeX,
				const Int SizeZ,
				const Float Scaling = DefaultScaling
			);

			~CHeight();

			template
			<
				typename T
			>
			bool ReadFromData
			(
				const TVector<T> & Data
			);

			void Resize
			(
				const UINT SizeX,
				const UINT SizeZ
			);

			ErrorCode LoadRAWHeightmap
			(
				const EHeightMapDataType  DataType,
				const WString			& Path
			);

			ErrorCode LoadRAWHeightmapShort
			(
				const WString & Path
			);

			ErrorCode LoadRAWHeightmapChar
			(
				const WString & Path
			);

			void Clamp
			(
				const Float Min,
				const Float Max
			);

			void Normalize
			(
				const Float Value
			);

			void Erode();

			Vector3f * GetPositions()
			{
				Vector3f * Positions = new Vector3f[SizeX * SizeZ];

				UINT O = 0;

				for (UINT Z = 0; Z < SizeZ; ++Z)
				{
					for (UINT X = 0; X < SizeX; ++X)
					{
						Positions[O++] = Vector3f(X, Z, GetScaledHeight(X, Z));
					}
				}

				return Positions;
			}

			void SetHeightMap(float * HeightMapArray)
			{
				if (HeightMap)
				{
					delete[] HeightMap;
				}

				HeightMap = HeightMapArray;
			}

			void SetNormalMap(Vector3f * NormalMapArray)
			{
				if (NormalMap)
				{
					delete[] NormalMap;
				}

				NormalMap = NormalMapArray;
			}

			void SetTangentMap(Vector3f * TangentMapArray)
			{
				if (NormalMap)
				{
					delete[] TangentMap;
				}

				TangentMap = TangentMapArray;
			}

			Vector3f * GetNormals
			(
				const Vector3f * Positions
			)
			{
				Vector3f * Normals = new Vector3f[SizeX * SizeZ];
				{
					std::fill_n(Normals, SizeX * SizeZ, Vector3f::ZeroVector);
				}

				UINT O = 0;

				for (UINT Z = 0; Z < SizeZ - 1; ++Z)
				{
					for (UINT X = 0; X < SizeX - 1; ++X)
					{
						const Vector3f & P0 = Positions[O];
						const Vector3f & P1 = Positions[O + 1];
						const Vector3f & P2 = Positions[O + SizeX];

						Vector3f D0 = P1 - P0;
						Vector3f D1 = P2 - P0;

						Vector3f Normal = Vector3f::CrossProduct(D0, D1);
						{
							Normal.Normalize();
						}

						Normals[O] += Normal;
						Normals[O + 1] += Normal;
						Normals[O + SizeX] += Normal;

						Normals[O++].Normalize();
					}
				}

				return Normals;
			}
		};

		template
		<
			typename T
		>
		inline bool CHeight::ReadFromData
		(
			const TVector<T> & Data
		)
		{
			if (Data.size() != SizeX * SizeZ)
			{
				return false;
			}

			for (UINT N = 0; N < Data.size(); ++N)
			{
				HeightMap[N] = static_cast<Float>(Data[N]) * HeightScale;
			}

			return true;
		}
	}
}