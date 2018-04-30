#pragma once

#include "RawResource.h"

namespace D3D
{
	class CResourceCache : public CSingleton<CResourceCache>
	{
	private:

		THashMap<DXGI_FORMAT, TVector<SharedPointer<RResource> > > ResourceMap;


	};
}