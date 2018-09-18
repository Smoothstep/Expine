#pragma once

#include "DirectX/D3D.h"

namespace D3D
{
	struct SimplePositionVertex
	{
		Vector3f Position;
	};

	struct SimpleColorVertex
	{
		Vector4f	Position;
		RGBAColor	Color;
	};

	struct SimpleTextureVertex
	{
		Vector3f Position;
		Vector2f Texture;
	};
}