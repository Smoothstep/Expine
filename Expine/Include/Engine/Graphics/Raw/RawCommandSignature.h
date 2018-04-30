#pragma once

#include "RawRootSignature.h"

namespace D3D
{
	class RCommandSignature
	{
	private:

		ComPointer<ICommandSignature> CommandSignature;

	protected:

		SharedPointer<RRootSignature> RootSignature;

	public:

		inline operator ICommandSignature * () const
		{
			return CommandSignature.Get();
		}

		inline ICommandSignature * operator->() const
		{
			return CommandSignature.Get();
		}

	public:

		ErrorCode Create
		(
			const SharedPointer<RRootSignature> & pRootSignature,
			const D3D12_COMMAND_SIGNATURE_DESC	& CommandSignatureDesc
		);

		ErrorCode Create
		(
			const D3D12_COMMAND_SIGNATURE_DESC	& CommandSignatureDesc
		);
	};
}