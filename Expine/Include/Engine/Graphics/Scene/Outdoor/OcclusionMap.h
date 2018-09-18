#pragma once

#include "Raw/RawUnorderedAccessView.h"

namespace D3D
{
	class CSceneOcclusion
	{
	private:

		SharedPointer<RResource> OcclusionResource;

	public:

		inline const SharedPointer<RResource> & GetBufferResource() const
		{
			return OcclusionResource;
		}

	public:

		CSceneOcclusion();
		~CSceneOcclusion();

		ErrorCode Create
		(
			const IntPoint& Size
		);

		ErrorCode SaveOcclusionMap();
	};
}