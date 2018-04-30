#pragma once

#include "../VibeD3D/SceneEnvironment.h"

class ICollisionCheck
{
private:

	virtual float GetHeight(const Vector2f& Position) = 0;
	virtual float GetHeight(const float X, const float Y) = 0;

public:

	bool IntersectsTerrain(const Vector3f& Position)
	{

	}

	bool IntersectsTerrain(const Vector3f& Position, const float Radius)
	{
		
	}

	bool IntersectsTerrain(const Vector3f& Position, const Vector2f& Size)
	{

	}

	bool IntersectsObject(const Vector3f& Position, const float Radius)
	{}
};

class IEnvironment
{
public:


};

class VEnvironment : public IEnvironment
{
public:

	struct Properties
	{
		int32_t Width;
		int32_t Height;

		
	};

	void Create
	(

	)
	{

	}
};