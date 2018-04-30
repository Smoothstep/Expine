#pragma once

#include "D3D.h"

#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <dxgi1_5.h>

namespace D3D
{
	struct ScreenWindow
	{
		HWND WindowHandle;
		BOOL WindowIsFullscreen;
		UINT WindowWidth;
		UINT WindowHeight;

		struct
		{
			DXGI_FORMAT					Format;
			DXGI_RATIONAL				RefreshRate;
			DXGI_MODE_SCALING			ScalingMode;
			DXGI_MODE_SCANLINE_ORDER	ScanlineOrderMode;
		}	Output;
	};

	struct ScreenViewport : public D3D12_VIEWPORT 
	{
		D3D12_RECT ScissorRect;
	};

	static IDXGIOutput * GetBestOutput
	(
		IDXGIAdapter * 	pAdapter,
		ScreenWindow &	Window
	)
	{
		if (!pAdapter)
		{
			return NULL;
		}

		DXGI_MODE_DESC ModeDesc = {};
		{
			ModeDesc.RefreshRate	= Window.Output.RefreshRate;
			ModeDesc.Format			= Window.Output.Format;
			ModeDesc.Height			= Window.WindowHeight;
			ModeDesc.Width			= Window.WindowWidth;
		}

		DXGI_MODE_DESC ClosestMatching;

		ComPointer<IDXGIOutput> pOutput;

		for (UINT iOutputIndex = 0; DXGI_ERROR_NOT_FOUND != pAdapter->EnumOutputs(iOutputIndex, &pOutput); ++iOutputIndex)
		{
			ErrorCode Error = pOutput->FindClosestMatchingMode(
				&ModeDesc, 
				&ClosestMatching, 
				0);

			if (Error)
			{
				continue;
			}

			Window.WindowHeight				= ClosestMatching.Height;
			Window.WindowWidth				= ClosestMatching.Width;
			Window.Output.RefreshRate		= ClosestMatching.RefreshRate;
			Window.Output.ScalingMode		= ClosestMatching.Scaling;
			Window.Output.Format			= ClosestMatching.Format;
			Window.Output.ScanlineOrderMode = ClosestMatching.ScanlineOrdering;

			break;
		}

		return pOutput.Detach();
	}

	static IDXGIAdapter1 * GetHardwareAdapter
	(
		IDXGIFactory1 * const pFactory
	)
	{
		ComPointer<IDXGIAdapter1> pAdapter;

		if (!pFactory)
		{
			return NULL;
		}
		
		DXGI_ADAPTER_DESC1 AdapterDesc;

		for (UINT iAdapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(iAdapterIndex, &pAdapter); ++iAdapterIndex)
		{
			ErrorCode Error = pAdapter->GetDesc1(&AdapterDesc);

			if (Error)
			{
				continue;
			}

			if (AdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				continue;
			}

			Error = D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), NULL);

			if (!Error)
			{
				break;
			}

			pAdapter = nullptr;
		}

		return pAdapter.Detach();
	}
}