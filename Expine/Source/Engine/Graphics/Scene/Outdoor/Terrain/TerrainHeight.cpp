#include "Precompiled.h"

#include "Scene/Outdoor/TerrainHeight.h"
#include "Utils/File/File.h"

namespace D3D
{
	namespace Terrain
	{
		CHeight::CHeight(const Int SizeX, const Int SizeZ, const Float Scaling)
			: SizeX(SizeX)
			, SizeZ(SizeZ)
			, HeightScale(Scaling)
		{
			Ensure
			(
				SizeX * SizeZ != 0
			);

			HeightMap = new Float[SizeX * SizeZ];
		}

		CHeight::CHeight() 
			: SizeX(0)
			, SizeZ(0)
			, HeightMap(NULL)
			, HeightScale(DefaultScaling)
		{}

		CHeight::CHeight(const IntPoint Size, const Float Scaling)
			: SizeX(Size.X)
			, SizeZ(Size.Y)
			, HeightScale(Scaling)
		{
			Ensure
			(
				SizeX * SizeZ != 0
			);

			HeightMap = new Float[SizeX * SizeZ];
			HeightScale = 0.01f;
		}

		CHeight::~CHeight()
		{
			if (HeightMap)
			{
				SafeReleaseArray(HeightMap);
			}
		}

		void CHeight::Resize(const UINT SizeX, const UINT SizeZ)
		{
			if (HeightMap)
			{
				if (SizeX * SizeZ != this->SizeX * this->SizeZ)
				{
					HeightMap = ReAllocate<Float>(HeightMap, SizeX * SizeZ);
				}
			}
			else
			{
				HeightMap = Allocate<Float>(SizeX * SizeZ);
			}

			this->SizeX = SizeX;
			this->SizeZ = SizeZ;
		}

		ErrorCode CHeight::LoadRAWHeightmap(const EHeightMapDataType DataType, const WString & Path)
		{
			HeightMapDataType = DataType;

			switch (DataType)
			{
				default: case RAW_16_BIT:
				{
					return LoadRAWHeightmapShort(Path);
				}
			}

			return E_INVALIDARG;
		}

		ErrorCode CHeight::LoadRAWHeightmapShort(const WString & Path)
		{
			File::CFile HeightMapFile(Path);
			{
				ErrorCode Error;

				if ((Error = HeightMapFile.ReadFileContent()))
				{
					return Error;
				}
			}

			if (!ReadFromData(HeightMapFile.GetContentRef<Uint16>()))
			{
				return E_FAIL;
			}

			HeightMapFile.DetachContent(Data);

			return S_OK;
		}

		ErrorCode CHeight::LoadRAWHeightmapChar(const WString & Path)
		{
			File::CFile HeightMapFile(Path);
			{
				ErrorCode Error;

				if ((Error = HeightMapFile.ReadFileContent()))
				{
					return Error;
				}
			}

			if (!ReadFromData(HeightMapFile.GetContentRef<Uint8>()))
			{
				return E_FAIL;
			}

			return S_OK;
		}

		void CHeight::Clamp(const Float Min, const Float Max)
		{
			for (int Z = 0; Z < SizeZ; ++Z)
			{
				int Offset = Z * SizeX;

				for (int X = 0; X < SizeX; ++X)
				{
					Float Value = HeightMap[Offset + X];
					{
						HeightMap[Offset + X] = Math::Clamp(Value, Min, Max);
					}
				}
			}
		}

		void CHeight::Normalize(const Float Value)
		{
			Float Min = HeightMap[0];
			Float Max = HeightMap[0];

			for (int Z = 0; Z < SizeZ; Z++)
			{
				int Offset = Z * SizeX;

				for (int X = 0; X < SizeX; X++)
				{
					Float F = HeightMap[Offset + X];

					if (F > Max)
					{
						Max = F;
					}
					else if (F < Min)
					{
						Min = F;
					}
				}
			}

			if (Max <= Min)
			{
				return;
			}

			Float DeltaHeight = Max - Min;

			for (int Z = 0; Z < SizeZ; Z++)
			{
				int Offset = Z * SizeX;

				for (int X = 0; X < SizeX; X++)
				{
					HeightMap[Offset + X] = ((HeightMap[Offset + X] - Min) / DeltaHeight) * Value;
				}
			}
		}

		void CHeight::Erode()
		{
			Float Last;

			for (int Z = 0; Z < SizeZ; Z++)
			{
				int Offset = Z * SizeX;
				{
					Last = HeightMap[Offset];
				}

				for (int X = 1; X < SizeX; X++)
				{
					HeightMap[Offset + X] = Filter * Last + (1 - Filter) * HeightMap[Offset + X];
					{
						Last = HeightMap[Offset + X];
					}
				}
			}

			for (int Z = SizeZ - 1; Z >= 0; Z--)
			{
				int Offset = Z * SizeX;
				{
					Last = HeightMap[Offset];
				}

				for (int X = 1; X < SizeX; X++)
				{
					HeightMap[Offset + X] = Filter * Last + (1 - Filter) * HeightMap[Offset + X];
					{
						Last = HeightMap[Offset + X];
					}
				}
			}

			for (int Z = 0; Z < SizeZ; Z++)
			{
				int Offset = Z * SizeX;
				{
					Last = HeightMap[Offset];
				}

				for (int X = 1; X < SizeX; X++)
				{
					HeightMap[Offset + X] = Filter * Last + (1 - Filter) * HeightMap[Offset + X];
					{
						Last = HeightMap[Offset + X];
					}
				}
			}

			for (int Z = SizeZ - 1; Z >= 0; Z--)
			{
				int Offset = Z * SizeX;
				{
					Last = HeightMap[Offset];
				}

				for (int X = 1; X < SizeX; X++)
				{
					HeightMap[Offset + X] = Filter * Last + (1 - Filter) * HeightMap[Offset + X];
					{
						Last = HeightMap[Offset + X];
					}
				}
			}
		}
	}
}
