#include "Precompiled.h"

#include "Scene/Outdoor/OcclusionMap.h"

#include <Utils/File/File.h>

namespace D3D
{
	CSceneOcclusion::CSceneOcclusion()
	{}

	CSceneOcclusion::~CSceneOcclusion()
	{}

	ErrorCode CSceneOcclusion::Create(const IntPoint & Size)
	{
		ErrorCode Error;

		OcclusionResource = new RResource();
		
		if ((Error = OcclusionResource->Create(RResource::InitializeOptions::Texture2D(
			Size.X,
			Size.Y,
			DXGI_FORMAT_R32_FLOAT, 1, 0,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			1))))
		{
			return Error;
		}

		return S_OK;
	}

	ErrorCode CSceneOcclusion::SaveOcclusionMap()
	{
		ErrorCode Error;

		UINT64 Size = OcclusionResource->DetermineSize();

		File::CFile File(L"Vibe_OcclusionMap");

		if (!File.OpenFileWrite())
		{
			return ERROR_FILE_INVALID;
		}

		auto Content = File.GetContentRef();

		Content.resize(Size);

		if ((Error = OcclusionResource->Map(0, 0, 0, reinterpret_cast<void**>(&Content[0]))))
		{
			return Error;
		}

		if (File.WriteFileContent() != File::ErrorNone)
		{
			return ERROR_FILE_INVALID;
		}

		return S_OK;
	}
}
