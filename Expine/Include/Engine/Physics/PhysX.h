#pragma once

#include <Hyper.h>
#include <Singleton.h>
#include <File\File.h>
#include <ThirdParty\PhysXShared\foundation\PxErrors.h>
#include <ThirdParty\PhysX\PxPhysicsAPI.h>

namespace PhysX
{
	static inline const physx::PxVec3 & ToPx(const Vector3f& V)
	{
		return *reinterpret_cast<const physx::PxVec3*>(&V);
	}

	static inline const physx::PxVec4 & ToPx(const Vector4f& V)
	{
		return *reinterpret_cast<const physx::PxVec4*>(&V);
	}

	static inline const Vector3f & ToHx(const physx::PxVec3& V)
	{
		return *reinterpret_cast<const Vector3f*>(&V);
	}

	static inline const Vector4f & ToHx(const physx::PxVec4& V)
	{
		return *reinterpret_cast<const Vector4f*>(&V);
	}

	static inline const Vector3f & ToHx(const physx::PxExtendedVec3& V)
	{
		return *reinterpret_cast<const Vector3f*>(&V);
	}

	struct Default {};
	constexpr static Default GlobalDefault;

	enum EErrorType
	{
		PhysicsInitFailed,
		FoundationInitFailed,
		ExtensionInitFailed,
		CookingInitFailed
	};

	class ExPhysX
	{
	private:

		EErrorType ErrorType;

	public:

		inline EErrorType GetErrorType() const
		{
			return ErrorType;
		}

		ExPhysX
		(
			EErrorType ErrorType
		) :
			ErrorType(ErrorType)
		{}
	};

	class XActor
	{
		friend class CPhysX;
		friend class XScene;

	protected:

		Vector3f PhysicalPosition  = Vector3f::ZeroVector;
		Vector3f MassLocalPosition = Vector3f::ZeroVector;

	protected:

		physx::PxRigidDynamic	* Actor				= 0;
		physx::PxShape			* ActorShape		= 0;
		physx::PxController		* ActorController	= 0;

		physx::PxControllerFilters CollisionFilters;

	public:

		inline physx::PxActor & Px()
		{
			return *Actor;
		}

		inline physx::PxTransform PxGetTransform() const
		{
			return ActorShape->getLocalPose();
		}

		inline physx::PxGeometryHolder PxGetGeometryHolder() const
		{
			return ActorShape->getGeometry();
		}

		inline physx::PxGeometry PxGetGeometry() const
		{
			return ActorShape->getGeometry().any();
		}

		inline physx::PxBounds3 PxGetBounds() const
		{
			return Actor->getWorldBounds();
		}

		inline physx::PxTransform PxGetGlobalPose() const
		{
			return Actor->getGlobalPose();
		}

		inline Vector3f GetPosition() const
		{
			return ToHx(ActorController->getPosition());
		}

		inline const Vector3f GetPositionFoot() const
		{
			return ToHx(ActorController->getFootPosition());
		}

	public:

		XActor
		(
			const Vector3f & Position
		);
		XActor
		(
			physx::PxRigidDynamic * Actor
		);

		inline void SetPhysicalPosition
		(
			const Vector3f & Position
		) 
		{
			PhysicalPosition = Position;
		};

		inline void Teleport
		(
			const Vector3f & Position
		)
		{
			ActorController->setPosition(physx::PxExtendedVec3(Position.X, Position.Y, Position.Z));
			SetPhysicalPosition(Position);
		}

		inline void SetPhysicalPositionTarget
		(
			const Vector3f & Position
		)
		{
			Actor->setGlobalPose(physx::PxTransform(ToPx(Position)));
		}

		inline void SetPhysicalLinearVelocity
		(
			const Vector3f & Velocity
		)
		{
			Actor->setLinearVelocity(ToPx(Velocity));
		}

		inline void SetPhysicalAngularVelocity
		(
			const Vector3f & Velocity
		)
		{
			Actor->setAngularVelocity(ToPx(Velocity));
		}

		inline void Move
		(
			const Vector3f& Destination,
			const float		ElapsedTime
		)
		{
			ActorController->move(ToPx(Destination), 0, ElapsedTime, CollisionFilters);
		}

		inline void SetStepOffset
		(
			const float StepOffset
		)
		{
			ActorController->setStepOffset(StepOffset);
		}

		inline void SetMass
		(
			const float Mass
		)
		{
			Actor->setMass(Mass);
		}

		void CreateCapsuleShape
		(
			const float Radius,
			const float HalfHeight
		);

		void CreateBoxShape
		(
			const float X,
			const float Y,
			const float Z
		);

		void CreateShape
		(
			physx::PxGeometry & Geometry,
			physx::PxMaterial * const * Materials,
			Uint16				NumMaterials
		);

		void Resize
		(
			const float NewHeight
		)
		{
			ActorController->resize(NewHeight);
		}

		void UpdateVolumeCapsule
		(
			const float Radius,
			const float Height
		)
		{
			static_cast<physx::PxCapsuleController*>(ActorController)->setRadius(Radius);
			static_cast<physx::PxCapsuleController*>(ActorController)->setHeight(Height);
		}

		void UpdateVolumeBox
		(
			const float HalfHeight,
			const float HalfSideExtent,
			const float HalfForwardExtent
		)
		{
			static_cast<physx::PxBoxController*>(ActorController)->setHalfHeight(HalfHeight);
			static_cast<physx::PxBoxController*>(ActorController)->setHalfSideExtent(HalfSideExtent);
			static_cast<physx::PxBoxController*>(ActorController)->setHalfForwardExtent(HalfForwardExtent);
		}

		void SetDensity
		(
			const float Density
		);
	};

	class XScene
	{
	private:

		physx::PxScene				*	Scene;
		physx::PxControllerManager	*	ControllerManager;
		physx::PxU32					LastError;

	public:

		inline physx::PxScene & Px()
		{
			return *Scene;
		}

		inline physx::PxControllerManager & GetControllerManager()
		{
			return *ControllerManager;
		}

		inline uint32_t GetLastError()
		{
			return LastError;
		}

	public:

		~XScene();
		XScene
		(
			physx::PxScene * Scene
		);

		void AddActor
		(
			XActor * Actor
		);

		void Simulate
		(
			float TimeElapsed
		);

		bool ProcessSimulationResult();

		physx::PxRaycastBuffer CheckSceneRayHit
		(
			const Vector3f& Origin,
			const Vector3f& Direction,
			const float		Distance
		)	const;

		physx::PxOverlapBuffer CheckSceneRayIntersection
		(
			const physx::PxGeometry	 & Geometry,
			const physx::PxTransform & Transform
		)	const;
	};

	class CPhysX : public CSingleton<CPhysX>
	{
	public:

		class XAllocatorCallback : public physx::PxAllocatorCallback
		{
			virtual void * allocate
			(
						size_t	Size, 
				const	char *	TypeName, 
				const	char *	Filename, 
						int		Line
			) override;

			virtual void deallocate
			(
				void * Memory
			) override;
		};

		class XErrorCallback : public physx::PxErrorCallback
		{
			virtual void reportError
			(
						physx::PxErrorCode::Enum	Code, 
				const	char					*	Message, 
				const	char					*	File, 
						int							Line
			) override;
		};

		class XInputStream : public physx::PxInputStream
		{
		private:

			TVector<Byte> * Input;

			uint32_t OffsetMax;
			uint32_t OffsetMin = 0;
			uint32_t OffsetCur = 0;

		public:

			XInputStream
			(
				File::CFile * File
			)
			{
				Input = &File->GetContentRef();

				OffsetMax = Input->size();
			}

			XInputStream
			(
				TVector<Byte> * Data
			)
			{
				Input = Data;

				OffsetMax = Input->size();
			}

			XInputStream
			(
						Byte * Data, 
				const	size_t Size
			)
			{
				Input = new TVector<Byte>(Size);
				{
					CopyMemory(Input->data(), Data, Size);
				}

				OffsetMax = Input->size();
			}

			void SetStartOffset
			(
				const size_t Offset
			)
			{
				OffsetMin = Math::Max(Offset, Input->size());
			}

			void SetEndOffset
			(
				const size_t Offset
			)
			{
				OffsetMax = Math::Min(Offset, Input->size());
			}

			virtual uint32_t read
			(
				void	*	Data, 
				uint32_t	Count
			) override
			{
				if (OffsetCur + Count >= OffsetMax)
				{
					size_t Count = OffsetMax - OffsetCur;

					if (Count == 0)
					{
						return 0;
					}
				}

				CopyMemory(reinterpret_cast<Byte*>(Data), Input->data(), Count);
				{
					OffsetCur += Count;
				}

				return Count;
			}
		};

		struct XToleranceScale : public physx::PxTolerancesScale
		{
			static constexpr float DefaultMass = 100.0f;
			static constexpr float DefaultSpeed = 981.0f;

			inline XToleranceScale() = default;
			inline XToleranceScale
			(
				const Default&
			)
			{
				mass  = DefaultMass;
				speed = DefaultSpeed;
			}
		};

	private:

		physx::PxPhysics	* Physics;
		physx::PxFoundation * PhysicsFoundation;
		physx::PxCooking	* PhysicsCooking;
		physx::PxPvd		* PhysicsPvd;
		physx::PxMaterial	* PhysicsDefaultMaterial;

		XAllocatorCallback	CBAllocator;
		XErrorCallback		CBErrors;
		XToleranceScale		ToleranceScale = XToleranceScale(GlobalDefault);

	public:

		inline const physx::PxMaterial & GetDefaultMaterial() const
		{
			return *PhysicsDefaultMaterial;
		}

	public:

		~CPhysX();
		CPhysX();

		XScene * CreateScene();

		void CreateHeightField
		(
			XInputStream & InputStream
		);

		void CreateHeightField
		(
			XActor	* Actor,
			Uint16	* Data,
			int32_t Width,
			int32_t Height,
			Float	HeightScale
		);

		void CreateTriangleMesh
		(
			XInputStream & InputStream
		);

		physx::PxTriangleMesh * CreateTriangleMesh
		(
			uint32_t *	Indices,
			Vector3f *	Vertices,
			uint32_t	NumIndices,
			uint32_t	NumVertices
		);

		physx::PxTriangleMesh * CreateTriangleMesh
		(
			uint16_t *	Indices,
			Vector3f *	Vertices,
			uint16_t	NumIndices,
			uint32_t	NumVertices
		);

		XActor * CreateActor
		(
			const Vector3f & Position
		);

		void InitializeActor
		(
			XActor * Actor
		);
	};
}