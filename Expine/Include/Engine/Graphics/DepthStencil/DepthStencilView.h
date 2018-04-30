#pragma once

#include "RawDepthStencilView.h"

namespace D3D
{
	class CDepthStencilView
	{
	private:

		UniquePointer<RDepthStencilView> DepthStencilView;
		UniquePointer<RDepthStencilView> DepthStencilViewReadOnly;

		UINT ReadOnlySubresource;

		D3D12_RESOURCE_STATES ResourceStates[2];

	public:

		CDepthStencilView
		(
			RDepthStencilView * pDepthStencilView
		)
		{
			DepthStencilView = pDepthStencilView;
		}

		CDepthStencilView
		(
			RDepthStencilView * pDepthStencilView,
			RDepthStencilView * pDepthStencilViewReadOnly
		)
		{
			DepthStencilView			= pDepthStencilView;
			DepthStencilViewReadOnly	= pDepthStencilViewReadOnly;
		}

		void AsRead()
		{
			ResourceStates[ReadOnlySubresource] = D3D12_RESOURCE_STATE_DEPTH_READ;
			ReadOnlySubresource = (ReadOnlySubresource + 1) % 2;
		}

		void AsWrite()
		{
			if (DepthStencilViewReadOnly)
			{

			}
		}
	};
}