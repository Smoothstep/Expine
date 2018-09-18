#pragma once

#include "DirectX/D3D.h"

namespace D3D
{
#define MINIMUM_FEATURE_LEVEL D3D_FEATURE_LEVEL_11_0

	class RDevice : public CSingleton<RDevice>
	{
	private:

		ComPointer<IDevice> Device;

	public:

		inline IDevice * Get() const
		{
			return Device.Get();
		}

		inline operator IDevice*() const
		{
			return Device.Get();
		}

		inline IDevice* operator->() const
		{
			return Device.Get();
		}

	public:

		static ErrorCode CreateDevice
		(
			IUnknown * const pAdapter = NULL
		);
	};

#define DEVICE (RDevice::Instance())
}