#pragma once

#include "Object.h"

namespace D3D
{
	class CSpeedTreeObject;

	class CStaticObject : public CSceneObject
	{
	public:

		struct InitializeOptionsSpeedTree : public InitializeOptions
		{

		};

	public:
		
		virtual inline EObjectType ObjectType() const = 0;
	};

	class CStaticObjectFactory
	{
	public:

		virtual CStaticObject * Create
		(
			CStaticObject::InitializeOptions& Options
		) = 0;
	};
}