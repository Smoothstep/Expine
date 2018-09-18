#include <Types.h>
#include <Defines.h>
#include <WindowsH.h>
#include <DX12Helper.h>

#undef min
#undef max

using namespace Hyper;

class TextureLoaderException
{
private:
	String Message;
	Uint ErrorCode;

public:
	TextureLoaderException(StringView What, Uint ErrorCode = GetLastError()) : Message(What), ErrorCode(HRESULT_FROM_WIN32(ErrorCode)) {}
	TextureLoaderException(StringView What, HRESULT ErrorCode) : Message(What), ErrorCode(ErrorCode) {}
	TextureLoaderException() : ErrorCode(GetLastError()) {}

	const String& GetErrorMessage() const { return Message; }
	const Uint GetErrorCode() const { return ErrorCode; }
};

class TextureLoader
{
private:
	ScopedHandle FileMappingHandle;
	ScopedHandle FileHandle;

	size_t MappingOffset = 0;
	size_t MappingMax = 0;

protected:

	ID3D12Device * Device;

	static inline const size_t Granularity = GetSysInfo().dwAllocationGranularity;

	void Open(const WStringView& File)
	{
		FileHandle = CreateFile2(File.data(), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, NULL);

		if (!FileHandle.IsValid())
		{
			throw TextureLoaderException(String::MakeString("Unable to open file: ", File));
		}

		FileMappingHandle = CreateFileMapping(FileHandle, NULL, FILE_READ_ACCESS, -1, -1, NULL);

		if (!FileMappingHandle.IsValid())
		{
			throw TextureLoaderException(String::MakeString("Unable to create file mapping.", File));
		}

		FILE_STANDARD_INFO FileInfo;

		if (!GetFileInformationByHandleEx(FileMappingHandle, FileStandardInfo, &FileInfo, sizeof(FileInfo)))
		{
			throw TextureLoaderException(String::MakeString("Unable to retrieve file information: ", File));
		}

		MappingMax = static_cast<size_t>(FileInfo.EndOfFile.QuadPart);

	}

	inline Byte * Read(size_t Count)
	{
		auto Off = ExtractLoHi(MappingOffset);

		if (Count < Granularity)
		{
			Count = Granularity;
		}
		else
		{
			Count = MakeAlign(Count, Granularity);
		}

		Byte * Mapping = reinterpret_cast<Byte*>(MapViewOfFile(FileMappingHandle, PAGE_EXECUTE_READ, Off.second, Off.first, Count));
		MappingOffset += Count;
		return Mapping;
	}

	inline Byte * Read(size_t Offset, size_t Count)
	{
		auto Off = ExtractLoHi(Offset);

		if (Count < Granularity)
		{
			Count = Granularity;
		}
		else
		{
			Count = MakeAlign(Count, Granularity);
		}

		Byte * Mapping = reinterpret_cast<Byte*>(MapViewOfFile(FileMappingHandle, PAGE_EXECUTE_READ, Off.second, Off.first, Count));
		size_t AlignedOffset = MakeAlign(Offset, Granularity);
		MappingOffset = Offset + Count;

		return Mapping;
	}

	inline size_t GetSize() const
	{
		return MappingMax;
	}

public:

	TextureLoader(ID3D12Device * Device)
		: Device(Device)
	{}
};

namespace DDS
{
	inline constexpr bool IsDepthStencil(DXGI_FORMAT Format)
	{
		switch (Format)
		{
		case DXGI_FORMAT_R32G8X24_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		case DXGI_FORMAT_D16_UNORM:
			return true;

		default:
			return false;
		}
	}

	inline constexpr size_t BitsPerPixel(DXGI_FORMAT Format)
	{
		switch (Format)
		{
			case DXGI_FORMAT_R32G32B32A32_TYPELESS:
			case DXGI_FORMAT_R32G32B32A32_FLOAT:
			case DXGI_FORMAT_R32G32B32A32_UINT:
			case DXGI_FORMAT_R32G32B32A32_SINT:
				return 128;

			case DXGI_FORMAT_R32G32B32_TYPELESS:
			case DXGI_FORMAT_R32G32B32_FLOAT:
			case DXGI_FORMAT_R32G32B32_UINT:
			case DXGI_FORMAT_R32G32B32_SINT:
				return 96;

			case DXGI_FORMAT_R16G16B16A16_TYPELESS:
			case DXGI_FORMAT_R16G16B16A16_FLOAT:
			case DXGI_FORMAT_R16G16B16A16_UNORM:
			case DXGI_FORMAT_R16G16B16A16_UINT:
			case DXGI_FORMAT_R16G16B16A16_SNORM:
			case DXGI_FORMAT_R16G16B16A16_SINT:
			case DXGI_FORMAT_R32G32_TYPELESS:
			case DXGI_FORMAT_R32G32_FLOAT:
			case DXGI_FORMAT_R32G32_UINT:
			case DXGI_FORMAT_R32G32_SINT:
			case DXGI_FORMAT_R32G8X24_TYPELESS:
			case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
			case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
			case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
			case DXGI_FORMAT_Y416:
			case DXGI_FORMAT_Y210:
			case DXGI_FORMAT_Y216:
				return 64;

			case DXGI_FORMAT_R10G10B10A2_TYPELESS:
			case DXGI_FORMAT_R10G10B10A2_UNORM:
			case DXGI_FORMAT_R10G10B10A2_UINT:
			case DXGI_FORMAT_R11G11B10_FLOAT:
			case DXGI_FORMAT_R8G8B8A8_TYPELESS:
			case DXGI_FORMAT_R8G8B8A8_UNORM:
			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			case DXGI_FORMAT_R8G8B8A8_UINT:
			case DXGI_FORMAT_R8G8B8A8_SNORM:
			case DXGI_FORMAT_R8G8B8A8_SINT:
			case DXGI_FORMAT_R16G16_TYPELESS:
			case DXGI_FORMAT_R16G16_FLOAT:
			case DXGI_FORMAT_R16G16_UNORM:
			case DXGI_FORMAT_R16G16_UINT:
			case DXGI_FORMAT_R16G16_SNORM:
			case DXGI_FORMAT_R16G16_SINT:
			case DXGI_FORMAT_R32_TYPELESS:
			case DXGI_FORMAT_D32_FLOAT:
			case DXGI_FORMAT_R32_FLOAT:
			case DXGI_FORMAT_R32_UINT:
			case DXGI_FORMAT_R32_SINT:
			case DXGI_FORMAT_R24G8_TYPELESS:
			case DXGI_FORMAT_D24_UNORM_S8_UINT:
			case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
			case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
			case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
			case DXGI_FORMAT_R8G8_B8G8_UNORM:
			case DXGI_FORMAT_G8R8_G8B8_UNORM:
			case DXGI_FORMAT_B8G8R8A8_UNORM:
			case DXGI_FORMAT_B8G8R8X8_UNORM:
			case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
			case DXGI_FORMAT_B8G8R8A8_TYPELESS:
			case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			case DXGI_FORMAT_B8G8R8X8_TYPELESS:
			case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			case DXGI_FORMAT_AYUV:
			case DXGI_FORMAT_Y410:
			case DXGI_FORMAT_YUY2:
				return 32;

			case DXGI_FORMAT_P010:
			case DXGI_FORMAT_P016:
				return 24;

			case DXGI_FORMAT_R8G8_TYPELESS:
			case DXGI_FORMAT_R8G8_UNORM:
			case DXGI_FORMAT_R8G8_UINT:
			case DXGI_FORMAT_R8G8_SNORM:
			case DXGI_FORMAT_R8G8_SINT:
			case DXGI_FORMAT_R16_TYPELESS:
			case DXGI_FORMAT_R16_FLOAT:
			case DXGI_FORMAT_D16_UNORM:
			case DXGI_FORMAT_R16_UNORM:
			case DXGI_FORMAT_R16_UINT:
			case DXGI_FORMAT_R16_SNORM:
			case DXGI_FORMAT_R16_SINT:
			case DXGI_FORMAT_B5G6R5_UNORM:
			case DXGI_FORMAT_B5G5R5A1_UNORM:
			case DXGI_FORMAT_A8P8:
			case DXGI_FORMAT_B4G4R4A4_UNORM:
				return 16;

			case DXGI_FORMAT_NV12:
			case DXGI_FORMAT_420_OPAQUE:
			case DXGI_FORMAT_NV11:
				return 12;

			case DXGI_FORMAT_R8_TYPELESS:
			case DXGI_FORMAT_R8_UNORM:
			case DXGI_FORMAT_R8_UINT:
			case DXGI_FORMAT_R8_SNORM:
			case DXGI_FORMAT_R8_SINT:
			case DXGI_FORMAT_A8_UNORM:
			case DXGI_FORMAT_AI44:
			case DXGI_FORMAT_IA44:
			case DXGI_FORMAT_P8:
				return 8;

			case DXGI_FORMAT_R1_UNORM:
				return 1;

			case DXGI_FORMAT_BC1_TYPELESS:
			case DXGI_FORMAT_BC1_UNORM:
			case DXGI_FORMAT_BC1_UNORM_SRGB:
			case DXGI_FORMAT_BC4_TYPELESS:
			case DXGI_FORMAT_BC4_UNORM:
			case DXGI_FORMAT_BC4_SNORM:
				return 4;

			case DXGI_FORMAT_BC2_TYPELESS:
			case DXGI_FORMAT_BC2_UNORM:
			case DXGI_FORMAT_BC2_UNORM_SRGB:
			case DXGI_FORMAT_BC3_TYPELESS:
			case DXGI_FORMAT_BC3_UNORM:
			case DXGI_FORMAT_BC3_UNORM_SRGB:
			case DXGI_FORMAT_BC5_TYPELESS:
			case DXGI_FORMAT_BC5_UNORM:
			case DXGI_FORMAT_BC5_SNORM:
			case DXGI_FORMAT_BC6H_TYPELESS:
			case DXGI_FORMAT_BC6H_UF16:
			case DXGI_FORMAT_BC6H_SF16:
			case DXGI_FORMAT_BC7_TYPELESS:
			case DXGI_FORMAT_BC7_UNORM:
			case DXGI_FORMAT_BC7_UNORM_SRGB:
				return 8;

	#if defined(_XBOX_ONE) && defined(_TITLE)

			case DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT:
			case DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT:
			case DXGI_FORMAT_R10G10B10_SNORM_A2_UNORM:
				return 32;

			case DXGI_FORMAT_D16_UNORM_S8_UINT:
			case DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
			case DXGI_FORMAT_X16_TYPELESS_G8_UINT:
				return 24;

			case DXGI_FORMAT_R4G4_UNORM:
				return 8;

	#endif // _XBOX_ONE && _TITLE

			default:
				return 0;
		}
	}

	constexpr DXGI_FORMAT MakeSRGB(DXGI_FORMAT Format)
	{
		switch (Format)
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM:
			return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

		case DXGI_FORMAT_BC1_UNORM:
			return DXGI_FORMAT_BC1_UNORM_SRGB;

		case DXGI_FORMAT_BC2_UNORM:
			return DXGI_FORMAT_BC2_UNORM_SRGB;

		case DXGI_FORMAT_BC3_UNORM:
			return DXGI_FORMAT_BC3_UNORM_SRGB;

		case DXGI_FORMAT_B8G8R8A8_UNORM:
			return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

		case DXGI_FORMAT_B8G8R8X8_UNORM:
			return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;

		case DXGI_FORMAT_BC7_UNORM:
			return DXGI_FORMAT_BC7_UNORM_SRGB;

		default:
			return Format;
		}
	}

	constexpr void AdjustPlaneResource(
		DXGI_FORMAT Format,
		size_t Height,
		size_t SlicePlane,
		D3D12_SUBRESOURCE_DATA& Result)
	{
		switch (Format)
		{
		case DXGI_FORMAT_NV12:
		case DXGI_FORMAT_P010:
		case DXGI_FORMAT_P016:

#if defined(_XBOX_ONE) && defined(_TITLE)
		case DXGI_FORMAT_D16_UNORM_S8_UINT:
		case DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_X16_TYPELESS_G8_UINT:
#endif
			if (!SlicePlane)
			{
				Result.SlicePitch = Result.RowPitch * Height;
			}
			else
			{
				Result.pData = reinterpret_cast<const uint8_t*>(Result.pData) + Result.RowPitch * Height;
				Result.SlicePitch = Result.RowPitch * ((Height + 1) >> 1);
			}
			break;

		case DXGI_FORMAT_NV11:
			if (!SlicePlane)
			{
				Result.SlicePitch = Result.RowPitch * Height;
			}
			else
			{
				Result.pData = reinterpret_cast<const uint8_t*>(Result.pData) + Result.RowPitch * Height;
				Result.RowPitch = (Result.RowPitch >> 1);
				Result.SlicePitch = Result.RowPitch * Height;
			}
			break;
		}
	}

	constexpr void GetSurfaceInfo(
		size_t Width,
		size_t Height,
		DXGI_FORMAT Format,
		size_t& NumBytes,
		size_t& RowBytes,
		size_t& NumRows)
	{
		NumBytes = 0;
		RowBytes = 0;
		NumRows = 0;

		bool BC = false;
		bool Packed = false;
		bool Planar = false;

		size_t Bits = 0;

		switch (Format)
		{
		case DXGI_FORMAT_BC1_TYPELESS:
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGI_FORMAT_BC4_TYPELESS:
		case DXGI_FORMAT_BC4_UNORM:
		case DXGI_FORMAT_BC4_SNORM:
			BC = true;
			Bits = 8;
			break;

		case DXGI_FORMAT_BC2_TYPELESS:
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGI_FORMAT_BC3_TYPELESS:
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGI_FORMAT_BC5_TYPELESS:
		case DXGI_FORMAT_BC5_UNORM:
		case DXGI_FORMAT_BC5_SNORM:
		case DXGI_FORMAT_BC6H_TYPELESS:
		case DXGI_FORMAT_BC6H_UF16:
		case DXGI_FORMAT_BC6H_SF16:
		case DXGI_FORMAT_BC7_TYPELESS:
		case DXGI_FORMAT_BC7_UNORM:
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			BC = true;
			Bits = 16;
			break;

		case DXGI_FORMAT_R8G8_B8G8_UNORM:
		case DXGI_FORMAT_G8R8_G8B8_UNORM:
		case DXGI_FORMAT_YUY2:
			Packed = true;
			Bits = 4;
			break;

		case DXGI_FORMAT_Y210:
		case DXGI_FORMAT_Y216:
			Packed = true;
			Bits = 8;
			break;

		case DXGI_FORMAT_NV12:
		case DXGI_FORMAT_420_OPAQUE:
			Planar = true;
			Bits = 2;
			break;

		case DXGI_FORMAT_P010:
		case DXGI_FORMAT_P016:
			Planar = true;
			Bits = 4;
			break;
		}

		if (BC)
		{
			size_t NumBlocksWide = 0;

			if (Width > 0)
			{
				NumBlocksWide = std::max<size_t>(1, (Width + 3) / 4);
			}

			size_t NumBlocksHigh = 0;

			if (Height > 0)
			{
				NumBlocksHigh = std::max<size_t>(1, (Height + 3) / 4);
			}

			RowBytes = NumBlocksWide * Bits;
			NumRows = NumBlocksHigh;
			NumBytes = RowBytes * NumBlocksHigh;
		}
		else if (Packed)
		{
			RowBytes = ((Width + 1) >> 1) * Bits;
			NumRows = Height;
			NumBytes = RowBytes * Height;
		}
		else if (Format == DXGI_FORMAT_NV11)
		{
			RowBytes = ((Width + 3) >> 2) * 4;
			NumRows = Height * 2;
			NumBytes = RowBytes * NumRows;
		}
		else if (Planar)
		{
			RowBytes = ((Width + 1) >> 1) * Bits;
			NumBytes = (RowBytes * Height) + ((RowBytes * Height + 1) >> 1);
			NumRows = Height + ((Height + 1) >> 1);
		}
		else
		{
			RowBytes = (Width * BitsPerPixel(Format) + 7) / 8;
			NumRows = Height;
			NumBytes = RowBytes * Height;
		}
	}

	constexpr size_t CountMips(size_t W, size_t H)
	{
		if (W * H == 0)
		{
			return 0;
		}

		size_t C = 1;

		while (W > 1 || H > 1)
		{
			W >>= 1;
			H >>= 1;
			++C;
		}

		return C;
	}

	template<class ArrayType, size_t... S>
	static constexpr uint32_t MakeUint32(std::index_sequence<S...>& Seq, const ArrayType& Characters)
	{
		return ((Characters[S] << (S * 8)) + ...);
	}

	template<size_t Size = 4>
	static constexpr uint32_t MakeUint32(const char Characters[Size])
	{
		return MakeUint32(std::make_index_sequence<Size>(), Characters);
	}

	enum HeaderFlags
	{
		HF_Texture		= 0x00001007,
		HF_MipMap		= 0x00020000,
		HF_Volume		= 0x00800000,
		HF_Pitch		= 0x00000008,
		HF_Linearsize	= 0x00080000,
		HF_Height		= 0x00000002,
		HF_Width		= 0x00000004
	};

	enum MiscFlags
	{
		MF1_TextureCube = 0x00000004
	};

	enum MiscFlags2
	{
		MF2_AlphaNodeMask = 0x0000000L,
	};

	enum SurfaceFlags
	{
		SF_Texture	= 0x00001000,
		SF_MipMap	= 0x00400008,
		SF_CubeMap	= 0x00000008
	};

	enum CubeMapFaces
	{
		Cube_PosX = 0x00000600,
		Cube_NegX = 0x00000A00,
		Cube_PosY = 0x00001200,
		Cube_NegY = 0x00002200,
		Cube_PosZ = 0x00004200,
		Cube_NegZ = 0x00008200,
		Cube_AllF = Cube_PosX | 
		Cube_NegX | Cube_PosY | 
		Cube_NegY | Cube_PosZ | 
		Cube_NegZ
	};

	enum PixelformatType
	{
		PF_FourCC		= 0x00000004,
		PF_RGB			= 0x00000040,
		PF_RGBA			= 0x00000041,
		PF_Luminance	= 0x00020000,
		PF_LuminanceA	= 0x00020000,
		PF_Alphapixels	= 0x00000001,
		PF_Alpha		= 0x00000002,
		PF_PAL8			= 0x00000020,
		PF_PAL8A		= 0x00000021,
		PF_BumpDuDv		= 0x00080000
	};

	enum AlphaMode
	{
		AM_Unknown = 0,
		AM_Straight = 1,
		AM_Premultiplied = 2,
		AM_Opaque = 3,
		AM_Custom = 4,
	};

	enum LoaderFlags
	{
		LF_Default = 0,
		LF_ForceSRGB = 0x1,
		LF_MipReserve = 0x8,
	};
	
#pragma pack(push, 1)
	struct Pixelformat
	{
		constexpr Pixelformat(Uint32 Size = 0, Uint32 Flags = 0, Uint32 FourCC = 0, Uint32 RGBBitCount = 0, Uint32 RBitMask = 0, Uint32 GBitMask = 0, Uint32 BBitMask = 0, Uint32 ABitMask = 0) 
			: Size(Size), Flags(Flags), FourCC(FourCC), RGBBitCount(RGBBitCount), RBitMask(RBitMask), GBitMask(GBitMask), BBitMask(BBitMask), ABitMask(ABitMask)
		{}
		
		Uint32 Size;
		Uint32 Flags;
		Uint32 FourCC;
		Uint32 RGBBitCount;
		Uint32 RBitMask;
		Uint32 GBitMask;
		Uint32 BBitMask;
		Uint32 ABitMask;
	};

	struct Header
	{
		Uint32			Size;
		Uint32			Flags;
		Uint32			Height;
		Uint32			Width;
		Uint32			PitchOrLinearSize;
		Uint32			Depth;
		Uint32			MipMapCount;
		Uint32			Reserved1[11];
		Pixelformat		Format;
		Uint32			Caps1;
		Uint32			Caps2;
		Uint32			Caps3;
		Uint32			Caps4;
		Uint32			Reserved2;

		virtual AlphaMode GetAlphaMode()
		{
			if (Format.Flags & PF_FourCC)
			{
				if (Format.FourCC == MakeUint32("DXT2") || Format.FourCC == MakeUint32("DXT4"))
				{
					return AM_Premultiplied;
				}
			}

			return AM_Unknown;
		}
	};

	struct Header_DXT10 : public Header
	{
		DXGI_FORMAT		DXGIFormat;
		uint32_t		ResourceDimension;
		uint32_t		MiscFlag1;
		uint32_t		ArraySize;
		uint32_t		MiscFlags2;

		virtual AlphaMode GetAlphaMode() final
		{
			if (Format.Flags & PF_FourCC)
			{
				auto Mode = static_cast<AlphaMode>(MiscFlags2 & MF2_AlphaNodeMask);

				switch (Mode)
				{
				case AM_Straight:
				case AM_Premultiplied:
				case AM_Opaque:
				case AM_Custom:
					return Mode;
				}
			}

			return AM_Unknown;
		}
	};

	struct FileStructure
	{
		Uint32			Magic;
		Header			Header;
	};

	struct FileStructure10
	{
		Uint32			Magic;
		Header_DXT10	Header10;
	};
#pragma pack(pop)

	template<Uint32 FourCC>
	static constexpr Pixelformat MakeFormat()
	{
		return { sizeof(Pixelformat), PixelformatType::PF_FourCC, FourCC, 0, 0, 0, 0, 0 };
	}

	DXGI_FORMAT GetDXGIFormat(const Pixelformat& Format)
	{
#define ISBITMASK( r,g,b,a ) (Format.RBitMask == r && Format.GBitMask == g && Format.BBitMask == b && Format.ABitMask == a)

		if (Format.Flags & PF_RGB)
		{
			switch (Format.RGBBitCount)
			{
			case 32:
				if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
				{
					return DXGI_FORMAT_R8G8B8A8_UNORM;
				}

				if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
				{
					return DXGI_FORMAT_B8G8R8A8_UNORM;
				}

				if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000))
				{
					return DXGI_FORMAT_B8G8R8X8_UNORM;
				}

				if (ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000))
				{
					return DXGI_FORMAT_R10G10B10A2_UNORM;
				}

				if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
				{
					return DXGI_FORMAT_R16G16_UNORM;
				}

				if (ISBITMASK(0xffffffff, 0x00000000, 0x00000000, 0x00000000))
				{
					return DXGI_FORMAT_R32_FLOAT;
				}
				break;

			case 24:
				break;

			case 16:
				if (ISBITMASK(0x7c00, 0x03e0, 0x001f, 0x8000))
				{
					return DXGI_FORMAT_B5G5R5A1_UNORM;
				}
				if (ISBITMASK(0xf800, 0x07e0, 0x001f, 0x0000))
				{
					return DXGI_FORMAT_B5G6R5_UNORM;
				}

				if (ISBITMASK(0x0f00, 0x00f0, 0x000f, 0xf000))
				{
					return DXGI_FORMAT_B4G4R4A4_UNORM;
				}
				break;
			}
		}
		else if (Format.RGBBitCount & PF_Luminance)
		{
			if (8 == Format.RGBBitCount)
			{
				if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x00000000))
				{
					return DXGI_FORMAT_R8_UNORM;
				}

				if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00))
				{
					return DXGI_FORMAT_R8G8_UNORM;
				}
			}

			if (16 == Format.RGBBitCount)
			{
				if (ISBITMASK(0x0000ffff, 0x00000000, 0x00000000, 0x00000000))
				{
					return DXGI_FORMAT_R16_UNORM;
				}
				if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00))
				{
					return DXGI_FORMAT_R8G8_UNORM;
				}
			}
		}
		else if (Format.Flags & PF_Alpha)
		{
			if (8 == Format.RGBBitCount)
			{
				return DXGI_FORMAT_A8_UNORM;
			}
		}
		else if (Format.Flags & PF_BumpDuDv)
		{
			if (16 == Format.RGBBitCount)
			{
				if (ISBITMASK(0x00ff, 0xff00, 0x0000, 0x0000))
				{
					return DXGI_FORMAT_R8G8_SNORM;
				}
			}

			if (32 == Format.RGBBitCount)
			{
				if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
				{
					return DXGI_FORMAT_R8G8B8A8_SNORM;
				}
				if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
				{
					return DXGI_FORMAT_R16G16_SNORM;
				}
			}
		}
		else if (Format.Flags & PF_FourCC)
		{
			if (MakeUint32("DXT1") == Format.FourCC)
			{
				return DXGI_FORMAT_BC1_UNORM;
			}
			if (MakeUint32("DXT3") == Format.FourCC)
			{
				return DXGI_FORMAT_BC2_UNORM;
			}
			if (MakeUint32("DXT5") == Format.FourCC)
			{
				return DXGI_FORMAT_BC3_UNORM;
			}

			if (MakeUint32("DXT2") == Format.FourCC)
			{
				return DXGI_FORMAT_BC2_UNORM;
			}
			if (MakeUint32("DXT4") == Format.FourCC)
			{
				return DXGI_FORMAT_BC3_UNORM;
			}

			if (MakeUint32("ATI1") == Format.FourCC)
			{
				return DXGI_FORMAT_BC4_UNORM;
			}
			if (MakeUint32("BC4U") == Format.FourCC)
			{
				return DXGI_FORMAT_BC4_UNORM;
			}
			if (MakeUint32("BC4S") == Format.FourCC)
			{
				return DXGI_FORMAT_BC4_SNORM;
			}

			if (MakeUint32("ATI2") == Format.FourCC)
			{
				return DXGI_FORMAT_BC5_UNORM;
			}
			if (MakeUint32("BC5U") == Format.FourCC)
			{
				return DXGI_FORMAT_BC5_UNORM;
			}
			if (MakeUint32("BC5S") == Format.FourCC)
			{
				return DXGI_FORMAT_BC5_SNORM;
			}

			if (MakeUint32("RGBG") == Format.FourCC)
			{
				return DXGI_FORMAT_R8G8_B8G8_UNORM;
			}
			if (MakeUint32("GRGB") == Format.FourCC)
			{
				return DXGI_FORMAT_G8R8_G8B8_UNORM;
			}

			if (MakeUint32("YUY2") == Format.FourCC)
			{
				return DXGI_FORMAT_YUY2;
			}

			switch (Format.FourCC)
			{
			case 36: // D3DFMT_A16B16G16R16
				return DXGI_FORMAT_R16G16B16A16_UNORM;

			case 110: // D3DFMT_Q16W16V16U16
				return DXGI_FORMAT_R16G16B16A16_SNORM;

			case 111: // D3DFMT_R16F
				return DXGI_FORMAT_R16_FLOAT;

			case 112: // D3DFMT_G16R16F
				return DXGI_FORMAT_R16G16_FLOAT;

			case 113: // D3DFMT_A16B16G16R16F
				return DXGI_FORMAT_R16G16B16A16_FLOAT;

			case 114: // D3DFMT_R32F
				return DXGI_FORMAT_R32_FLOAT;

			case 115: // D3DFMT_G32R32F
				return DXGI_FORMAT_R32G32_FLOAT;

			case 116: // D3DFMT_A32B32G32R32F
				return DXGI_FORMAT_R32G32B32A32_FLOAT;
			}
		}

		return DXGI_FORMAT_UNKNOWN;
	}

	template
	<
		PixelformatType Type,
		Uint32 Bits,
		Uint32 MaskR,
		Uint32 MaskG,
		Uint32 MaskB,
		Uint32 MaskA
	>
		static constexpr Pixelformat MakeFormat()
	{
		return { sizeof(Pixelformat), Type, 0, Bits, MaskR, MaskG, MaskB, MaskA };
	}

	constexpr Uint32 DXT1S = MakeUint32("DXT1");

	static constexpr Pixelformat PF_DXT1		= MakeFormat<MakeUint32("DXT1")>();
	static constexpr Pixelformat Pf_DXT2		= MakeFormat<MakeUint32("DXT2")>();
	static constexpr Pixelformat Pf_DXT3		= MakeFormat<MakeUint32("DXT2")>();
	static constexpr Pixelformat Pf_DXT4		= MakeFormat<MakeUint32("DXT2")>();
	static constexpr Pixelformat Pf_DXT5		= MakeFormat<MakeUint32("DXT2")>();
	static constexpr Pixelformat Pf_BC4U		= MakeFormat<MakeUint32("BC4U")>();
	static constexpr Pixelformat Pf_BC4S		= MakeFormat<MakeUint32("BC4S")>();
	static constexpr Pixelformat Pf_BC5U		= MakeFormat<MakeUint32("BC5U")>();
	static constexpr Pixelformat Pf_BC5S		= MakeFormat<MakeUint32("BC5S")>();
	static constexpr Pixelformat Pf_RGBG		= MakeFormat<MakeUint32("RGBG")>();
	static constexpr Pixelformat Pf_GRGB		= MakeFormat<MakeUint32("GRGB")>();
	static constexpr Pixelformat Pf_YUY2		= MakeFormat<MakeUint32("YUY2")>();
	static constexpr Pixelformat Pf_YUV2		= MakeFormat<MakeUint32("YUV2")>();

	static constexpr Pixelformat Pf_A8R8G8B8	= MakeFormat<PixelformatType::PF_RGBA,	32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000>();
	static constexpr Pixelformat Pf_A8B8G8R8	= MakeFormat<PixelformatType::PF_RGBA,	32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000>();
	static constexpr Pixelformat Pf_A1R5G5B5	= MakeFormat<PixelformatType::PF_RGBA,	16, 0x00007C00, 0x000003E0, 0x0000001F, 0x00008000>();
	static constexpr Pixelformat Pf_A4R4G4B4	= MakeFormat<PixelformatType::PF_RGBA,	16, 0x00000F00, 0x000000F0, 0x0000000F, 0x0000F000>();

	static constexpr Pixelformat Pf_X8R8G8B8	= MakeFormat<PixelformatType::PF_RGB,	32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0x00000000>();
	static constexpr Pixelformat Pf_X8B8G8R8	= MakeFormat<PixelformatType::PF_RGB,	32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0x00000000>();
	static constexpr Pixelformat Pf_G16R16		= MakeFormat<PixelformatType::PF_RGB,	16, 0x0000FFFF, 0xFFFF0000, 0x00000000, 0x00000000>();
	static constexpr Pixelformat Pf_R5G6B5		= MakeFormat<PixelformatType::PF_RGB,	16, 0x0000F800, 0x000007E0, 0x0000001F, 0x00000000>();
	static constexpr Pixelformat Pf_R8G8B8		= MakeFormat<PixelformatType::PF_RGB,	24, 0x00FF0000, 0x0000FF00, 0x000000FF, 0x00000000>();

	static constexpr Pixelformat Pf_L8			= MakeFormat<PixelformatType::PF_Luminance, 8,	0xFFFF, 0x0000, 0x0000, 0x0000>();
	static constexpr Pixelformat Pf_L16			= MakeFormat<PixelformatType::PF_Luminance, 16, 0xFFFF, 0x0000, 0x0000, 0x0000>();
	static constexpr Pixelformat Pf_A8L8		= MakeFormat<PixelformatType::PF_Luminance, 8,	0xFF00, 0x0000, 0x0000, 0x00FF>();
	static constexpr Pixelformat Pf_A8L8ALT		= MakeFormat<PixelformatType::PF_Luminance, 8,	0x00FF, 0x0000, 0x0000, 0xFF00>();

	static constexpr Pixelformat Pf_A8			= MakeFormat<PixelformatType::PF_Alpha, 8, 0x0000, 0x0000, 0x0000, 0xFFFF>();

	static constexpr Pixelformat Pf_V8U8		= MakeFormat<PixelformatType::PF_BumpDuDv, 16, 0x00FF, 0xFF00, 0x0000, 0x0000>();
	static constexpr Pixelformat Pf_Q8W8V8U8	= MakeFormat<PixelformatType::PF_BumpDuDv, 32, 0x000000FF, 0x00FF0000, 0x0000FF00, 0xFF000000>();
	static constexpr Pixelformat Pf_V16U16		= MakeFormat<PixelformatType::PF_BumpDuDv, 32, 0x0000FFFF, 0xFFFF0000, 0x00000000, 0x00000000>();


	class TextureLoader : public ::TextureLoader
	{
	private:

		bool FillData(
			size_t Width,
			size_t Height,
			size_t Depth,
			size_t MipCount,
			size_t ArraySize,
			size_t NumberOfPlanes,
			DXGI_FORMAT Format,
			size_t MaxSize,
			size_t BitSize,
			const Byte* Data,
			size_t& RWidth,
			size_t& RHeight,
			size_t& RDepth,
			size_t& RSkipMip,
			std::vector<D3D12_SUBRESOURCE_DATA>& RInitData)
		{
			size_t NumBytes;
			size_t RowBytes;
			size_t NumRows;

			const Byte * EndMemory = Data + BitSize;

			for (size_t Plane = 0; Plane < NumberOfPlanes; ++Plane)
			{
				const Byte * CurrentMemory = Data;

				for (size_t Arr = 0; Arr < ArraySize; ++Arr)
				{
					size_t W = Width;
					size_t H = Height;
					size_t D = Depth;

					for (size_t Mip = 0; Mip < MipCount; ++Mip)
					{
						GetSurfaceInfo(W, H, Format, NumBytes, RowBytes, NumRows);

						if (MipCount <= 1 || MaxSize == 0 || (W <= MaxSize || H <= MaxSize || D <= MaxSize))
						{
							if (RWidth == 0)
							{
								RWidth = W;
								RHeight = H;
								RDepth = D;
							}

							D3D12_SUBRESOURCE_DATA SubresourceData =
							{
								reinterpret_cast<const void*>(CurrentMemory),
								static_cast<LONG_PTR>(RowBytes),
								static_cast<LONG_PTR>(NumBytes)
							};

							AdjustPlaneResource(Format, H, Plane, SubresourceData);

							RInitData.emplace_back(SubresourceData);
						}
						else if (!Arr)
						{
							++RSkipMip;
						}

						if (CurrentMemory + (NumBytes * D) > EndMemory)
						{
							throw TextureLoaderException("End of file reached.", ERROR_HANDLE_EOF);
						}

						CurrentMemory += NumBytes * D;
						
						W >>= 1;
						H >>= 1;
						D >>= 1;

						if (W == 0) W = 1;
						if (H == 0) H = 1;
						if (D == 0) D = 1;
					}
				}
			}

			return !RInitData.empty();
		}

		HRESULT CreateTextureResource(
			const Uint32 Width,
			const Uint32 Height,
			const Uint32 Depth,
			const Uint32 MipCount,
			const Uint32 ArraySize,
			DXGI_FORMAT Format,
			const D3D12_RESOURCE_FLAGS ResourceFlags,
			const D3D12_RESOURCE_DIMENSION Dimension,
			ID3D12Resource * Result)
		{
#pragma warning(disable:1563)
			return Device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), 
				D3D12_HEAP_FLAG_NONE, 
				&CD3DX12_RESOURCE_DESC(Dimension, 16, Width, Height, std::max(Depth, ArraySize), MipCount, Format, 1, 0, 
					D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_UNKNOWN, ResourceFlags), D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&Result));
		}

		ID3D12Resource * Process(
			const Byte * Data, 
			const size_t Size, 
			size_t MaxSize,
			const Uint32 ArraySize,
			const Uint32 Depth,
			const Uint32 Width, 
			const Uint32 Height,
			const size_t MipCount,
			const Uint32 LoadFlags,
			const D3D12_RESOURCE_FLAGS ResourceFlags,
			const D3D12_RESOURCE_DIMENSION Dimension, 
			const bool IsCubeMap,
			DXGI_FORMAT Format,
			std::vector<D3D12_SUBRESOURCE_DATA>& RData)
		{
			ID3D12Resource * Resource = nullptr;

			if (LoadFlags & LF_ForceSRGB)
			{
				Format = MakeSRGB(Format);
			}

			switch (Dimension)
			{
			case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
				if ((ArraySize > D3D12_REQ_TEXTURE1D_ARRAY_AXIS_DIMENSION) || (Width > D3D12_REQ_TEXTURE1D_U_DIMENSION))
				{
					throw TextureLoaderException("Texture1D width or array size exceeded maximum.", ERROR_NOT_SUPPORTED);
				}

				break;

			case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
				if (IsCubeMap)
				{
					if ((ArraySize > D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION) || (Width > D3D12_REQ_TEXTURECUBE_DIMENSION) || (Height > D3D12_REQ_TEXTURECUBE_DIMENSION))
					{
						throw TextureLoaderException("Texture1D width, height or array size exceeded maximum.", ERROR_NOT_SUPPORTED);
					}
				}
				else if ((ArraySize > D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION) || (Width > D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION) || (Height > D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION))
				{
					throw TextureLoaderException("Texture1D width, height or array size exceeded maximum.", ERROR_NOT_SUPPORTED);
				}

				break;

			case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
				if ((ArraySize > 1) || (Width > D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION) || (Height > D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION) || (Depth > D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION))
				{
					throw TextureLoaderException("Texture3D array size must be equal to 1. Width, height and depth may not exceed their maximum.", ERROR_NOT_SUPPORTED);
				}

				break;

			default:
				throw TextureLoaderException("Unsupported dimension.", ERROR_NOT_SUPPORTED);
			}

			const Uint8 NumPlanes = D3D12GetFormatPlaneCount(Device, Format);

			if (NumPlanes > 1 && IsDepthStencil(Format))
			{
				throw TextureLoaderException("Format is type of depth stencil, but uses multiple planes.", ERROR_NOT_SUPPORTED);
			}

			const size_t NumResources = ArraySize * MipCount * NumPlanes;

			RData.reserve(NumResources);

			size_t SkipMip = 0;
			size_t W = 0;
			size_t H = 0;
			size_t D = 0;

			if (FillData(W, H, D, MipCount, ArraySize, NumPlanes, Format, Size, Size, Data, W, H, D, SkipMip, RData))
			{
				size_t ReservedMips;

				if (LoadFlags & LF_MipReserve)
				{
					ReservedMips = std::min<size_t>(D3D12_REQ_MIP_LEVELS, CountMips(W, H));
				}
				else
				{
					ReservedMips = 0;
				}

				if (FAILED(CreateTextureResource(W, H, D, MipCount, ArraySize, Format, ResourceFlags, Dimension, Resource)) && !MaxSize && MipCount > 1)
				{
					RData.clear();

					MaxSize = (Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
						? D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION
						: D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;

					if (FillData(Width, Height, Depth, MipCount, ArraySize, NumPlanes, Format, MaxSize, Size, Data, W, H, D, SkipMip, RData))
					{
						HRESULT Hr = CreateTextureResource(W, H, D, MipCount - SkipMip, ArraySize, Format, ResourceFlags, Dimension, Resource);

						if (FAILED(Hr))
						{
							throw TextureLoaderException("Unable to create texture resource.", Hr);
						}
					}
				}

			}

			return Resource;
		}

		ID3D12Resource * Process(const Byte * Data, const size_t Size, const size_t MaxSize, const D3D12_RESOURCE_FLAGS ResourceFlags, const Header& Header, bool& IsCubeMap, std::vector<D3D12_SUBRESOURCE_DATA>& RData)
		{
			DXGI_FORMAT Format = GetDXGIFormat(Header.Format);
			D3D12_RESOURCE_DIMENSION Dimension;
			Uint32 ArraySize = 1;
			Uint32 Depth = Header.Depth;

			if (Format == DXGI_FORMAT_UNKNOWN)
			{
				throw TextureLoaderException("Texture format not recognized", ERROR_NOT_SUPPORTED);
			}

			if (Header.Flags & HF_Volume)
			{
				Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
			}
			else
			{
				if (Header.Caps2 & SF_CubeMap)
				{
					if ((Header.Caps2 & Cube_AllF) != Cube_AllF)
					{
						throw TextureLoaderException("Not all 6 faces are defined.", ERROR_NOT_SUPPORTED);
					}

					ArraySize = 6;
					IsCubeMap = true;
				}

				Depth = 1;
				Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			}

			assert(BitsPerPixel(Format) != 0);

			return TextureLoader::Process(Data, Size, MaxSize, ArraySize, Depth, Header.Width, Header.Height, 
				Header.MipMapCount, Header.Flags, ResourceFlags, Dimension, IsCubeMap, Format, RData);
		}

		ID3D12Resource * Process(
			const Byte * Data, 
			const size_t Size, 
			const size_t MaxSize,
			const Uint32 LoadFlags,
			const D3D12_RESOURCE_FLAGS ResourceFlags, 
			const Header_DXT10& Header, 
			bool& IsCubeMap, 
			std::vector<D3D12_SUBRESOURCE_DATA>& RData)
		{
			constexpr size_t MaxSupportedMipMaps = D3D12_REQ_MIP_LEVELS;

			if (Header.MipMapCount > MaxSupportedMipMaps)
			{
				throw TextureLoaderException("Amount of mip maps.", ERROR_NOT_SUPPORTED);
			}

			Uint32 ArraySize = Header.ArraySize;
			Uint32 Depth = Header.Depth;

			switch (Header.DXGIFormat)
			{
			case DXGI_FORMAT_AI44:
			case DXGI_FORMAT_IA44:
			case DXGI_FORMAT_P8:
			case DXGI_FORMAT_A8P8:
				throw TextureLoaderException("Unsupported format.", ERROR_NOT_SUPPORTED);

			default:
				if (BitsPerPixel(Header.DXGIFormat) == 0)
				{
					throw TextureLoaderException("Unsupported format.", ERROR_NOT_SUPPORTED);
				}
			}

			switch (Header.ResourceDimension)
			{
			case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
				if ((Header.Flags & HeaderFlags::HF_Height) && Header.Height != 1)
				{
					throw TextureLoaderException("Texture 1D height not 1.", ERROR_INVALID_DATA);
				}

				Depth = 1;

				break;

			case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
				if (Header.MiscFlag1 & MF1_TextureCube)
				{
					ArraySize *= 6;
					IsCubeMap = true;
				}

				Depth = 1;

				break;

			case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
				if (!(Header.Flags & HeaderFlags::HF_Volume))
				{
					throw TextureLoaderException("Texture 3D is not marked as volume.", ERROR_INVALID_DATA);
				}

				if (Header.ArraySize > 1)
				{
					throw TextureLoaderException("Texture 3D is marked as texture array.", ERROR_NOT_SUPPORTED);
				}

				break;

			default:
				throw TextureLoaderException("Unsupported resource dimension.", ERROR_NOT_SUPPORTED);
			}

			return TextureLoader::Process(Data, Size, MaxSize, ArraySize, Depth, Header.Width, Header.Height,
				Header.MipMapCount, LoadFlags, ResourceFlags, static_cast<D3D12_RESOURCE_DIMENSION>(Header.ResourceDimension), IsCubeMap, Header.DXGIFormat, RData);
		}

	public:

		using ::TextureLoader::TextureLoader;

		ID3D12Resource * Load(
			const WStringView& File,
			const D3D12_RESOURCE_FLAGS ResourceFlags, 
			const Uint32 LoadFlags, 
			const size_t MaxSize, 
			bool& IsCubeMap,
			std::vector<D3D12_SUBRESOURCE_DATA>& SubResourceData)
		{
			TextureLoaderException InvalidFormat("Invalid format.");

			::TextureLoader::Open(File);

			const size_t Size = TextureLoader::GetSize();

			if (Size < sizeof(Uint32))
			{
				throw InvalidFormat;
			}

			Byte * Bytes = TextureLoader::Read(sizeof(Uint32));

			if (!Bytes)
			{
				throw InvalidFormat;
			}

			const FileStructure10 * Struct = reinterpret_cast<FileStructure10*>(Bytes);

			if (Struct->Magic != MakeUint32("DDS "))
			{
				throw InvalidFormat;
			}

			const Header_DXT10 & Header = Struct->Header10;
			size_t HeaderSize;

			if ((Header.Flags & PixelformatType::PF_FourCC) && Header.Format.FourCC == MakeUint32("DXT1"))
			{
				if (Size < sizeof(FileStructure10))
				{
					throw InvalidFormat;
				}

				HeaderSize = sizeof(FileStructure10);
			}
			else
			{
				if (Size < sizeof(FileStructure))
				{
					throw InvalidFormat;
				}

				HeaderSize = sizeof(FileStructure);
			}

			ID3D12Resource * Resource = Process(TextureLoader::Read(HeaderSize, Size - HeaderSize),
				Size - HeaderSize, MaxSize, ResourceFlags, Header, IsCubeMap, SubResourceData);

			if (!Resource)
			{
				throw TextureLoaderException("Unable to fill initial data.", E_FAIL);
			}

			return Resource;
		}
	};
}