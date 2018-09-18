#include "Precompiled.h"

#include "PostProcess/PostProcessDOF.h"

namespace D3D
{
	namespace PostProcess
	{
		ErrorCode CPostProcessDOF::Create(const IntPoint & BlurSize)
		{
			ErrorCode Error;

			SharedPointer<RResource> BlurResource = new RResource();

			if ((Error = BlurResource->Create(RResource::InitializeOptions::Texture2D(BlurSize.X, BlurSize.Y, CSceneComponents::CompositeSceneFormat, 1, 0, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE))))
			{
				return Error;
			}

			return S_OK;
		}
	}
}
