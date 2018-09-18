#include "Precompiled.h"

#include "Raw/RawCommandSignature.h"
#include "Raw/RawDevice.h"

namespace D3D
{
	ErrorCode RCommandSignature::Create(const SharedPointer<RRootSignature> & pRootSignature, const D3D12_COMMAND_SIGNATURE_DESC & CommandSignatureDesc)
	{
		ErrorCode Error = DEVICE->CreateCommandSignature(&CommandSignatureDesc, pRootSignature.GetRef(), IID_PPV_ARGS(&CommandSignature));

		if (Error)
		{
			return Error;
		}

		RootSignature = pRootSignature;

		return S_OK;
	}

	ErrorCode RCommandSignature::Create(const D3D12_COMMAND_SIGNATURE_DESC & CommandSignatureDesc)
	{
		ErrorCode Error = DEVICE->CreateCommandSignature(&CommandSignatureDesc, NULL, IID_PPV_ARGS(&CommandSignature));

		if (Error)
		{
			return Error;
		}

		return S_OK;
	}
}
