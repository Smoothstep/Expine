#pragma once

#include "Raw/RawResource.h"
#include "Scene/Object/StaticObject.h"
#include "Object/Mesh.h"
#include "Utils/File/File.h"
#include "ThirdParty/SpeedTree/Core/Core.h"

namespace D3D
{
	class CScene;
	class CSceneRenderer;
	class CSceneEnvironment;

	class CSpeedTree
	{
		enum MaterialType
		{
			Default,
			Branches,
			Fronds,
			FacingLeaves,
			Leaves,
			Meshes,
			Billboards
		};

		enum WindType
		{
			Palm,
			Best,
			Better,
			Fast,
			Fastest
		};

		enum LodType
		{
			LodSmooth,
			LodPop
		};

		enum GeometryType
		{
			GeometryBranch,
			GeometryFrond,
			GeometryBillboard,
			GeometryLeaf,
			GeometryFacingLeaf
		};

		using KRenderState = SpeedTree::SRenderState;

		struct SPTMaterial : public KMaterialStatic
		{
			LodType							TypeLod;
			WindType						TypeWind;
			GeometryType					TypeGeometry;

			SharedPointer<RResource>		ResourceNormal;
			SharedPointer<RResource>		ResourceNormalDetail;
			SharedPointer<RResource>		ResourceDiffuse;
			SharedPointer<RResource>		ResourceDiffuseDetail;
			SharedPointer<RResource>		ResourceSpecular;

			const KRenderState * RenderState;

			SPTMaterial
			(
						KMaterialStatic	*	ParentMaterial,
						KMaterial		*	Material,
				const	KRenderState	*	RenderState,
				const	GeometryType		TypeGeometry,
				const	WindType			TypeWind
			);

			virtual void FreeComponents() override {};
		};

		class CMaterialLoader
		{
			enum MaterialShaderStates
			{
				EnableBranchSeamSmoothing,
				EnableDiffuseMap,
				EnableSpecularMap,
				EnableNormalMap,
				EnableDetailMap,
				EnableRigidMeshes,
				EnableAmbientOcclusion,
				EnableTwoSidedMaterials,
				NumMaterialShaderStates
			};

			enum MaterialShaderValues
			{
				ValueSeamSmoothAmount,
				ValueBillboardThreshold,
				NumMaterialShaderValues
			};
			
			TArray<Byte,  NumMaterialShaderStates> States;
			TArray<Float, NumMaterialShaderValues> Values;

			SPTMaterial * StaticMaterial = 0;

		public:

			ErrorCode LoadMaterial
			(
				const SpeedTree::SRenderState * pRenderState, 
				const WindType					WindType,
				const LodType					LodType,
				const int32_t					NumBillboards
			);

			SPTMaterial * GetMaterial()
			{
				return StaticMaterial;
			}
		};

	private:

		static CMaterialLab GMaterialLab;

	protected:

		SpeedTree::CCore Core;

	private:

		void ProcessTriangleCorners
		(
			const	int32_t						TriangleIdx,
			const	int32_t						Corner,
			const	SpeedTree::SDrawCall	&	DrawCall,
			const	SpeedTree::SRenderState *	RenderState,
			const	uint16_t				*	pIndices16,
			const	uint32_t				*	pIndices32,
					KMesh					&	Mesh,
			const	int32_t						Offset,
			const	int32_t						NumUVs
		);

	public:

		ErrorCode LoadSPT
		(
			const	String&	FileName, 
			const	bool	bGrass		= false, 
			const	float	ScaleFactor = 1.0f
		);

		ErrorCode LoadSPT
		(
			File::CFile	*	pFile, 
			bool			bGrass		= false, 
			float			ScaleFactor = 1.0f
		);

		ErrorCode CreateMesh
		(
			const String & Name,
			KMaterialStatic * ParentMaterial
		);

		inline const char * GetLastError() const;
	};

	class CSpeedTreeObjectController : public ISceneObjectController
	{
	public:

		virtual bool InsideFrustum
		(
			const ViewFrustum & Frustum
		)	const override;

		virtual bool IntersectsFrustum
		(
			const ViewFrustum & Frustum
		)	const override;
	};

	class CSpeedTreeObject : public CStaticObject
	{
	private:

		CSpeedTree			* Spt;
		CSceneEnvironment	* Environment;

	public:

		inline CSpeedTree * GetSpeedTreeComponents() const
		{
			return Spt;
		}

	public:

		CSpeedTreeObject
		(
			CSpeedTree * Spt,
			CSceneEnvironment * Environment
		);

		virtual inline EObjectType ObjectType() const override
		{
			return ObjectTypeSpeedTree;
		}

		virtual void Render
		(
			const CSceneRenderer * Renderer
		)	const override;
	};
}