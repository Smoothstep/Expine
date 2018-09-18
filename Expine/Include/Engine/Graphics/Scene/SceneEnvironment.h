#pragma once

#include "Scene/SceneOutdoor.h"
#include "Scene/SceneIndoor.h"
#include "Scene/Object/StaticObject.h"

namespace D3D
{
	struct EnvironmentProperties
	{
		bool FogEnabled;
		bool ShadowsEnabled;
	};

	enum AreaFrustumIntersectionState
	{
		AreaOutsideFrustum,
		AreaInsideFrustum,
		AreaIntersectsFrustum
	};

	class CSceneEnvironment
	{
	private:

		EnvironmentProperties Properties;
		SharedPointer<CSceneArea> SceneArea;

	public:

		inline const EnvironmentProperties & GetProperties() const
		{
			return Properties;
		}

	public:

		inline void LoadArea
		(
			SharedPointer<CSceneArea> & Area
		)
		{
			SceneArea = Area;
		}

		AreaFrustumIntersectionState GetAreaIntersectionState
		(
			const Vector3f & Position
		)
		{
			return AreaIntersectsFrustum;
		}
	};
}