#include "Precompiled.h"

#include "Raw/RawDevice.h"
#include "Command/CommandQueue.h"
#include "Pipeline/Pipelines.h"
#include "Resource/Texture/TextureManager.h"
#include "Buffer/BufferCache.h"

namespace D3D
{
	static ErrorCode SetupPipelines()
	{
		ErrorCode Error;

		if ((Error = Pipelines::InitializePipelines()))
		{
			return Error;
		}

		return S_OK;
	}

	static ErrorCode SetupCommandQueues()
	{
		ErrorCode Error;

		if ((Error = CCommandQueueDirect::New()->Create()))
		{
			return Error;
		}

		if ((Error = CCommandQueueCompute::New()->Create()))
		{
			return Error;
		}

		if ((Error = CCommandQueueCopy::New()->Create()))
		{
			return Error;
		}

		return S_OK;
	}

	static ErrorCode EnableDebugLayer()
	{
		ErrorCode Error;

		ComPointer<ID3D12Debug> pDebugController;

		if (!(Error = D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugController))))
		{
			pDebugController->EnableDebugLayer();
		}

		return Error;
	}

	ErrorCode RDevice::CreateDevice(IUnknown * const pAdapter)
	{
		static RDevice g_GrpDevice;

		if (g_GrpDevice.Device == NULL)
		{
			ErrorCode Error;

#ifdef _DEBUG
			ComPointer<ID3D12Debug> pDebugController;

			if (!(Error = D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugController))))
			{
				pDebugController->EnableDebugLayer();
			}

			GShaderManager.EnableDevelopmentMode();
#endif

			if ((Error = D3D12CreateDevice(pAdapter, MINIMUM_FEATURE_LEVEL, IID_PPV_ARGS(&g_GrpDevice.Device))))
			{
				return Error;
			}

			if ((Error = SetupPipelines()))
			{
				return Error;
			}

			if ((Error = SetupCommandQueues()))
			{
				return Error;
			}

			CBufferCache::New();
			CTextureManager::New();
		}

		return S_OK;
	}
}
