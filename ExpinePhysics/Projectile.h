#pragma once

#include <Hyper/Math.h>
#include <Hyper/Vector3.h>

namespace PHX
{
	using namespace Hyper;

	constexpr size_t ProjectileBatchSize = 128;

	enum ProjectileTargetingType
	{
		Target_Direct,
		Target_Indirect,
		Target_Tracing
	};

	enum ProjectileState
	{
		State_Idle,
		State_Flying,
		State_Triggered,
		State_Disarmed
	};

	struct ProjectileAttributes
	{
		Uint32 Index;
		float Weight;
		float Velocity;
		float Acceleration;
		float Accuracy;
		float Angle;
		float FlightControl;
		float TTL;
		float Time;
		float MinHeight;
		ProjectileTargetingType TargetingType;
		Vector3f Size;
	};

	struct ProjectileData
	{
		Uint16 Index;
		Uint16 Next;
		Vector3f Angle;
		Vector3f Origin;
		Vector3f Position;
		Vector3f Target;
		ProjectileState State;
		ProjectileData() = default;
		ProjectileData(const Uint16 Index, const Vector3f& Origin, const Vector3f& Target)
			: Index(Index)
			, Next(Index + 1)
			, Origin(Origin)
			, Position(Origin)
			, Target(Target)
		{}
	};

	constexpr int sizeS = sizeof(ProjectileData);

	struct ProjectileBatch
	{
		ProjectileData BulkData[ProjectileBatchSize];

		Uint16 CurrentOffset = 0;

		template<class... Args>
		ProjectileData * AddProjectile(Args&&... Arguments)
		{
			return new (&BulkData[CurrentOffset]) ProjectileData(CurrentOffset++, std::forward<Args>(Arguments)...);
		}
	};

	class GlobalObject
	{
	private:
		Uint64 ObjectId;
	};

	class Projectile : public GlobalObject
	{
	private:
		ProjectileAttributes * Attr;
		ProjectileBatch & Batch;
		ProjectileData * Data;

	public:
		Projectile(ProjectileAttributes * Attr, ProjectileBatch& Batch)
			: Attr(Attr)
			, Batch(Batch)
		{
			Data = Batch.AddProjectile();
		}
	};

	class ProjectileManager
	{
	private:
		std::deque<Projectile> Projectiles;
		std::vector<ProjectileAttributes> AttrLut;

	public:
		ProjectileManager()
		{

		}

		Projectile & SpawnProjectile(Uint32 AttrId)
		{

		}

		template<class... Args>
		ProjectileAttributes & AddAttributeEntry(Args&&... Arguments)
		{
			return AttrLut.emplace_back(AttrLut.size(), std::forward<Args>(Arguments)...);
		}
	};
}