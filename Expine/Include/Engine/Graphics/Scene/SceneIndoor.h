#pragma once

#include "D3D.h"
#include "SceneArea.h"

namespace D3D
{
	class CSceneIndoor : public CSceneArea
	{
	public:

		inline virtual EAreaType GetAreaType() const
		{
			return AreaIndoor;
		}
	};
}