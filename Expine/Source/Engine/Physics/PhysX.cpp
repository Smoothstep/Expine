
#include "PhysX.h"

namespace PhysX
{
	void * CPhysX::XAllocatorCallback::allocate(size_t size, const char * typeName, const char * filename, int line)
	{
		return malloc(size);
	}

	void CPhysX::XAllocatorCallback::deallocate(void * ptr)
	{
		free(ptr);
	}

	void CPhysX::XErrorCallback::reportError(physx::PxErrorCode::Enum code, const char * message, const char * file, int line)
	{
		
	}
	
	CPhysX::~CPhysX()
	{
		Physics->release();
		PhysicsCooking->release();
		PhysicsPvd->release();
		PhysicsFoundation->release();
	}

#define PHYSX_FOUNDATION_VERSION 0x01000000
	
	CPhysX::CPhysX()
	{
		PhysicsFoundation = PxCreateFoundation(PHYSX_FOUNDATION_VERSION, CBAllocator, CBErrors);

		if (!PhysicsFoundation)
		{
			throw ExPhysX(FoundationInitFailed);
		}

		Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *PhysicsFoundation, ToleranceScale);
		
		if (!Physics)
		{
			throw ExPhysX(PhysicsInitFailed);
		}

		physx::PxCookingParams CookingParamters(ToleranceScale);

		PhysicsCooking = PxCreateCooking(PX_PHYSICS_VERSION, *PhysicsFoundation, CookingParamters);

		if (!PhysicsCooking)
		{
			throw ExPhysX(CookingInitFailed);
		}

		if (!PxInitExtensions(*Physics, PhysicsPvd = physx::PxCreatePvd(*PhysicsFoundation)))
		{
			throw ExPhysX(ExtensionInitFailed);
		}

		PhysicsDefaultMaterial = Physics->createMaterial(1, 1, 1);
	}

	XScene * CPhysX::CreateScene()
	{
		physx::PxSceneDesc Desc(ToleranceScale);
		{
			Desc.filterShader = physx::PxDefaultSimulationFilterShader;
			Desc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(1);
		}

		if (!Desc.isValid())
		{
			return nullptr;
		}
	
		return new XScene(Physics->createScene(Desc));
	}

	void CPhysX::CreateHeightField(XInputStream & InputStream)
	{
		physx::PxHeightField * HeightField = Physics->createHeightField(InputStream);
	}

	void CPhysX::CreateHeightField(XActor * Actor, Uint16 * Data, int32_t Width, int32_t Height, Float HeightScale)
	{
		TVector<physx::PxHeightFieldSample> Sample(Width * Height);

		uint32_t O = 0;

		for (int32_t Y = 0; Y < Height; ++Y)
		{
			for (int32_t X = 0; X < Width; ++X)
			{
				Sample[O].height = Data[O++];
			}
		}

		physx::PxHeightFieldDesc HeightFieldDesc;
		{
			HeightFieldDesc.format			= physx::PxHeightFieldFormat::eS16_TM;
			HeightFieldDesc.nbColumns		= Height;
			HeightFieldDesc.nbRows			= Width;
			HeightFieldDesc.samples.data	= Sample.data();
			HeightFieldDesc.samples.stride	= sizeof(physx::PxHeightFieldSample);
		}

		physx::PxHeightField * HeightField = PhysicsCooking->createHeightField(HeightFieldDesc, Physics->getPhysicsInsertionCallback());
		physx::PxHeightFieldGeometry HeightFieldGeometry(HeightField, physx::PxMeshGeometryFlags(), HeightScale, 1.0, 1.0);
			
		physx::PxMaterial * const Materials[] = { PhysicsDefaultMaterial };
	
		Actor->CreateShape(HeightFieldGeometry, Materials, 1);
	}

	void CPhysX::CreateTriangleMesh(XInputStream & InputStream)
	{
		physx::PxTriangleMesh * TriangleMesh = Physics->createTriangleMesh(InputStream);
	}

	physx::PxTriangleMesh * CPhysX::CreateTriangleMesh(uint32_t * Indices, Vector3f * Vertices, uint32_t NumIndices, uint32_t NumVertices)
	{
		physx::PxTriangleMeshDesc TriMeshDesc = { };
		{
			TriMeshDesc.points.count		= NumVertices;
			TriMeshDesc.points.stride		= sizeof(Vector3f);
			TriMeshDesc.points.data			= Vertices;

			TriMeshDesc.triangles.count		= NumIndices;
			TriMeshDesc.triangles.stride	= 3 * sizeof(uint32_t);
			TriMeshDesc.triangles.data		= Indices;
		}

		physx::PxDefaultMemoryOutputStream WBuffer;

		if (!PhysicsCooking->cookTriangleMesh(TriMeshDesc, WBuffer))
		{
			return nullptr;
		}

		physx::PxDefaultMemoryInputData RBuffer(WBuffer.getData(), WBuffer.getSize());

		return Physics->createTriangleMesh(RBuffer);
	}

	physx::PxTriangleMesh * CPhysX::CreateTriangleMesh(uint16_t * Indices, Vector3f * Vertices, uint16_t NumIndices, uint32_t NumVertices)
	{
		physx::PxTriangleMeshDesc TriMeshDesc = {};
		{
			TriMeshDesc.points.count		= NumVertices;
			TriMeshDesc.points.stride		= sizeof(Vector3f);
			TriMeshDesc.points.data			= Vertices;

			TriMeshDesc.triangles.count		= NumIndices;
			TriMeshDesc.triangles.stride	= 3 * sizeof(uint16_t);
			TriMeshDesc.triangles.data		= Indices;
		}

		physx::PxDefaultMemoryOutputStream WBuffer;

		if (!PhysicsCooking->cookTriangleMesh(TriMeshDesc, WBuffer))
		{
			return nullptr;
		}

		physx::PxDefaultMemoryInputData RBuffer(WBuffer.getData(), WBuffer.getSize());

		return Physics->createTriangleMesh(RBuffer);
	}

	XActor * CPhysX::CreateActor(const Vector3f & Position)
	{
		return new XActor(Physics->createRigidDynamic(physx::PxTransform(ToPx(Position))));
	}

	void CPhysX::InitializeActor(XActor * Actor)
	{
		if (Actor->Actor)
		{
			return;
		}
		
		Actor->Actor = Physics->createRigidDynamic(physx::PxTransform(ToPx(Actor->PhysicalPosition)));
	}
	
	XScene::XScene(physx::PxScene * Scene)
		: Scene(Scene)
	{
		Scene->userData = this;
		ControllerManager = PxCreateControllerManager(*Scene);
	}

	XScene::~XScene()
	{
		if (Scene)
		{
			Scene->release();
		}
	}

	void XScene::AddActor(XActor * Actor)
	{
		Scene->addActor(Actor->Px());
	}

	void XScene::Simulate(float TimeElapsed)
	{
		Scene->simulate(TimeElapsed);
	}

	bool XScene::ProcessSimulationResult()
	{
		uint32_t ActiveActorCount;

		if (Scene->fetchResults(true, &LastError))
		{
			physx::PxActor ** Actors = Scene->getActiveActors(ActiveActorCount);

			for (uint32_t A = 0; A < ActiveActorCount; ++A)
			{
				physx::PxRigidActor * RigidActor = static_cast<physx::PxRigidActor*>(Actors[A]);

				if (Actors[A]->is<physx::PxRigidActor>())
				{
					XActor * Actor = reinterpret_cast<XActor*>(Actors[A]->userData);
					{
						Actor->SetPhysicalPosition(ToHx(RigidActor->getGlobalPose().p));
					}
				}
			}

			return true;
		}

		return false;
	}

	physx::PxRaycastBuffer XScene::CheckSceneRayHit(const Vector3f & Origin, const Vector3f & Direction, const float Distance) const
	{
		physx::PxRaycastBuffer Hits;
		{
			Scene->raycast(ToPx(Origin), ToPx(Direction), Distance, Hits);
		}

		return Hits;
	}

	physx::PxOverlapBuffer XScene::CheckSceneRayIntersection(const physx::PxGeometry & Geometry, const physx::PxTransform & Transform) const
	{
		physx::PxOverlapBuffer Intersections;
		{
			Scene->overlap(Geometry, Transform, Intersections);
		}

		return Intersections;
	}

	XActor::XActor(const Vector3f & Position)
	{
		PhysicalPosition = Position;
	}

	XActor::XActor(physx::PxRigidDynamic * Actor) : Actor(Actor)
	{
		Actor->userData = this;
	}

	void XActor::CreateCapsuleShape(float Radius, float HalfHeight)
	{
		if (ActorController)
		{
			ActorController->release();
			ActorController = 0;
		}

		ActorShape = Actor->createShape(physx::PxCapsuleGeometry(Radius, HalfHeight), CPhysX::Instance().GetDefaultMaterial());
		{
			ActorShape->setLocalPose(physx::PxTransform(physx::PxQuat(physx::PxHalfPi, ToPx(Vector3f::UpVector))));
		}

		physx::PxRigidBodyExt::updateMassAndInertia(*Actor, 1.0f);
		
		XScene * Scene = static_cast<XScene*>(Actor->getScene()->userData);
		{
			physx::PxCapsuleControllerDesc CapspuleControllerDesc;
			{
				CapspuleControllerDesc.setToDefault();
			}

			ActorController = Scene->GetControllerManager().createController(CapspuleControllerDesc);
		}

		ActorController->setUpDirection(ToPx(Vector3f::UpVector));
	}

	void XActor::CreateBoxShape(float X, float Y, float Z)
	{
		if (ActorController)
		{
			ActorController->release();
			ActorController = 0;
		}
		
		ActorShape = Actor->createShape(physx::PxBoxGeometry(X, Y, Z), CPhysX::Instance().GetDefaultMaterial());
		{
			ActorShape->setLocalPose(physx::PxTransform(physx::PxQuat(physx::PxHalfPi, ToPx(Vector3f::UpVector))));
		}

		physx::PxRigidBodyExt::updateMassAndInertia(*Actor, 1.0f, &ToPx(MassLocalPosition));

		XScene * Scene = static_cast<XScene*>(Actor->getScene()->userData);
		{
			physx::PxBoxControllerDesc BoxControllerDesc;
			{
				BoxControllerDesc.setToDefault();
			}

			ActorController = Scene->GetControllerManager().createController(BoxControllerDesc);
		}

		ActorController->setUpDirection(ToPx(Vector3f::UpVector));
	}

	void XActor::CreateShape(physx::PxGeometry & Geometry, physx::PxMaterial * const * Materials, Uint16 NumMaterials)
	{
		if (ActorController)
		{
			ActorController->release();
			ActorController = 0;
		}

		ActorShape = Actor->createShape(Geometry, Materials, NumMaterials);
	}

	void XActor::SetDensity(float Density)
	{
		physx::PxRigidBodyExt::updateMassAndInertia(*Actor, Density, &ToPx(MassLocalPosition));
	}
}
