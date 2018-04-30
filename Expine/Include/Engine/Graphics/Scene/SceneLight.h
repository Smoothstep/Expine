#pragma once

#include "Hyper.h"
#include "VolumetricLighting.h"

namespace D3D
{
	namespace Light
	{
		enum ELightTypes
		{
			LIGHT_UNKNOWN		= -1,
			LIGHT_DIRECTIONAL	=  0,
			LIGHT_SPOT			=  1,
			LIGHT_OMNI			=  2
		};

		struct LightProperties
		{
			ELightTypes Type;

			RGBAColor	AmbientColor;
			RGBAColor	DiffuseColor;
			RGBAColor	BloomColor;

			Float		BloomScale;
			Float		Radius;
			Float		SourceRadius;
			Float		OcclusionMaskDarkness;
			Float		OcclusionDepthRange;

			size_t		LightPower;

			union
			{
				struct
				{
					Float			ZNear;
					Float			ZFar;
					AttenuationMode AttenuationMode;
					Float			AttenuationFactors[4];
				} 
				Omni;

				struct
				{
					Float					ZNear;
					Float					ZFar;
					Vector3f				Direction;
					AttenuationMode			AttenuationMode;
					SpotlightFalloffMode	FalloffMode;
					Float					FalloffCosTheta;
					Float					FalloffPower;
					Float					AttenuationFactors[4];
				} 
				Spot;

				struct
				{
					Vector3f Direction;
				} 
				Directional;
			};

			Vector3f	Position;
			Vector3f	Origin;
			Vector3f	Up;
			Vector2f	ShiftAngles;

			Matrix4x4	LightViewMatrix;
			Matrix4x4	LightProjectionMatrix;
			Matrix4x4	LightViewProjectionMatrix;
			Matrix4x4	LightScreenToLightMatrix;
			Matrix4x4	LightTransform;

			Matrix4x4	LightViewInverseMatrix;
			Matrix4x4	LightProjectionInverseMatrix;
			Matrix4x4	LightViewProjectionInverseMatrix;

			Vector3f GetLightIntesity() const;

			void SetPosition
			(
				const Vector3f & LightPosition
			);

			LightProperties() = default;
			LightProperties
			(
				const Default & Default
			);
		};
	}

	class CScene;
	class CSceneRenderer;
	class CSceneLight
	{
	protected:

		Light::CVolumetricLighting VolumetricLighting;

	protected:

		Light::LightProperties Properties;
		Light::ShadowMapDesc ShadowMap = {};
		Light::MediumDesc Medium = {};

		D3D12_VIEWPORT	ShadowMapViewport;
		D3D12_RECT		ShadowMapScissorRect;

	public:

		class CRenderer
		{
		private:

			ConstPointer<CScene>			Scene;
			ConstPointer<CSceneRenderer> SceneRenderer;
			ConstPointer<CSceneLight>	SceneLight;

		public:

			ErrorCode Create();

		public:

			CRenderer
			(
				const CSceneRenderer * pSceneRenderer
			);

			void BeginAccumulation() const;
			void EndAccumulation() const;

			void ApplyLighting
			(
				const Light::PostprocessDesc & Postprocess
			)	const;
		
			void StartVolumeRender() const;
		};

	public:

		inline const Light::LightProperties & GetProperties() const
		{
			return Properties;
		}

		inline const Light::ShadowMapDesc & GetShadowMapDesc() const
		{
			return ShadowMap;
		}

		inline const Light::MediumDesc & GetMediumDesc() const
		{
			return Medium;
		}

	public:

		CSceneLight();
		CSceneLight
		(
			const Light::LightProperties & Properties
		);

		ErrorCode Initialize
		(
			const CScene * pScene
		);

		void SetMediumType
		(
			const uint32_t Type
		);
	};
}