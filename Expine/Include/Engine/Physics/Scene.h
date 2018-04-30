#pragma once

#include "PhysX.h"
#include "../VibeD3D/Scene.h"

namespace Vibe
{
	class VScene : public D3D::CScene
	{
	private:

		UniquePointer<PhysX::XScene> PhysxScene;
		 		
	public:

		VScene(const D3D::CScreen * pScreen) : D3D::CScene(pScreen)
		{
			PhysxScene = PhysX::CPhysX::Instance().CreateScene();
		}

		void LoadArea
		(
			WString& AreaProperties
		);
	};
}