#pragma once

#include "Raw/RawRenderTarget.h"

namespace D3D
{
	struct ShadeRRenderTargetView
	{
		SharedPointer<RRenderTargetView> RenderTarget;

		D3D12_CPU_DESCRIPTOR_HANDLE CPUAddress; // RTV
		D3D12_GPU_DESCRIPTOR_HANDLE GPUAddress; // SRV
	};
}