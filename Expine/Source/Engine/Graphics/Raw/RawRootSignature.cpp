#include "Precompiled.h"

#include "Raw/RawRootSignature.h"
#include "Raw/RawDevice.h"

namespace D3D
{
	ErrorCode RRootSignature::Create(const RRootSignature::InitializeOptions & Options)
	{
		IBlob * pError;
		IBlob * pSignature;

		ErrorCode Error = D3D12SerializeRootSignature
		(
			&Options,
			D3D_ROOT_SIGNATURE_VERSION_1,
			&pSignature,
			&pError
		);

		if (Error)
		{
			return Error;
		}

		Ensure(pSignature != NULL);

		Error = DEVICE->CreateRootSignature
		(
			0,
			pSignature->GetBufferPointer(),
			pSignature->GetBufferSize(),
			IID_PPV_ARGS(&RootSignature)
		);

		if (Error)
		{
			return Error;
		}

		return S_OK;
	}
}
