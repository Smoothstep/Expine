#pragma once

#include "DirectX/D3D.h"

#include "Scene/Object/Intersection.h"
#include "Scene/Object/ObjectTable.h"

namespace D3D
{
	static constexpr UINT NumAreaSubNodes = 4;

	class CAreaNode : public IBoxBehavior<CAreaNode>
	{
	private:

		Vector3f Size;
		Vector3f Position;

		CAreaNode * SubNodes[NumAreaSubNodes];

	public:

		inline const Vector3f & GetPosition() const
		{
			return Position;
		}

		inline const Vector3f & GetExtent() const
		{
			return Size;
		}

	public:

		struct InitializeOptions
		{

		};

	public:

		CAreaNode
		(
			const InitializeOptions & Options
		)
		{

		}
	};

	class CAreaPatch : public CAreaNode
	{
	private:

	public:

	};

	struct AreaProperties
	{
		UINT AreaId;
	};

	struct AreaLoadParameters
	{
		UINT AreaId;

		inline bool Valid() const
		{
			return true;
		}

		inline AreaLoadParameters() {}
		inline AreaLoadParameters
		(
			UINT AreaId
		) : AreaId(AreaId)
		{}
	};

	enum EAreaType
	{
		AreaIndoor,
		AreaOutdoor
	};
	
	class CSceneArea
	{
	private:

		ObjectTable StaticObjectTable;

	protected:

		AreaProperties Properties;

		ErrorCode LoadStaticObjects();
		ErrorCode LoadDynamicObjects();

	public:

		virtual ErrorCode Load
		(
			const AreaLoadParameters & Parameters
		);

		virtual EAreaType GetAreaType() const = 0;

		ErrorCode LoadArea
		(
			const UINT AreaId
		);
	};
}