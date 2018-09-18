#pragma once

#include "Raw/RawResource.h"

namespace D3D
{
	class CResourceCache : public CSingleton<CResourceCache>
	{
	private:

		THashMap<DXGI_FORMAT, TVector<SharedPointer<RResource> > > ResourceMap;


	};
}