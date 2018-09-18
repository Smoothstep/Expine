#include "Precompiled.h"

#include "Scene/Scene.h"
#include "Scene/SceneRenderer.h"
#include "Scene/SceneLight.h"

namespace D3D
{
	namespace Light
	{
		constexpr float	GLightRange				= 5000.0f;
		constexpr float	GSpotlightFallOffAngle	= PI / 4.0f;
		constexpr float	GSpotlightFallOffPower	= 1.0f;

		void LightProperties::SetPosition(const Vector3f & LightOrigin)
		{
			Up = Vector3f::UpVector;

			Origin = Vector3f
			(
				2000.0f, 
				2000.0f, 
				0.0f
			);

			switch (Type)
			{
				case LIGHT_OMNI:
				{
					Position = LightTransform.TransformVector(Vector3f(15, 10, 0));

					LightViewMatrix = Matrix4x4
					(
						Vector4f(1, 0, 0, 0),
						Vector4f(0, 1, 0, 0),
						Vector4f(0, 0, 1, 0),
						Vector4f(-Origin, 1)
					);

					LightProjectionMatrix = Matrix4x4::Identity();
				}

				break;

				case LIGHT_SPOT:
				{
					Position = LightTransform.TransformVector(Vector3f(10, 15, 5));

					LightViewMatrix = LookAt
					(
						Position,
						Origin,
						Up
					);

					LightProjectionMatrix = CreateOrthographicMatrix
					(
						GSpotlightFallOffAngle,
						1.0f,
						0.50f,
						GLightRange
					);
				}

				break;

				default: case LIGHT_DIRECTIONAL:
				{
					Position = Vector3f
					(
						 0.0f, 
						 0.0f, 
						 1500.0f
					);

					Directional.Direction = (Origin - Position).GetSafeNormal();

					LightViewMatrix = LookAt
					(
						Position,
						Origin,
						Up
					);

					const float FarZ	= 0.5f;
					const float NearZ	= GLightRange;

					LightProjectionMatrix = CreateOrthographicMatrix(2000, 2000, NearZ, FarZ);
				}

				break;
			}

			LightViewProjectionMatrix = LightViewMatrix * LightProjectionMatrix;

			LightViewMatrix.VectorInverse(LightViewInverseMatrix);
			LightProjectionMatrix.VectorInverse(LightProjectionInverseMatrix);
			LightViewProjectionMatrix.VectorInverse(LightViewProjectionInverseMatrix);

			Rotation  Rotation = Directional.Direction.ToOrientationRotation();
			Matrix4x4 RotationMatrix = CreateInversedRotationMatrix(Rotation) *
				Matrix4x4(
					Vector4f(0, 0, 1, 0),
					Vector4f(1, 0, 0, 0),
					Vector4f(0, 1, 0, 0),
					Vector4f(0, 0, 0, 1));

			LightScreenToLightMatrix = Matrix4x4
			(
				Vector4f(1, 0, 0, 0),
				Vector4f(0, 1, 0, 0),
				Vector4f(0, 0, LightProjectionMatrix.MatrixArray[2][2], 1),
				Vector4f(0, 0, LightProjectionMatrix.MatrixArray[3][2], 0)
			) * (LightProjectionInverseMatrix * RotationMatrix.GetTransposed());
		}

		Vector3f LightProperties::GetLightIntesity() const
		{
			static const Vector3f LightPowerLookUpTable[] =
			{
				1.00f * Vector3f(1.00f, 0.95f, 0.90f),
				0.50f * Vector3f(1.00f, 0.95f, 0.90f),
				1.50f * Vector3f(1.00f, 0.95f, 0.90f),
				1.00f * Vector3f(1.00f, 0.75f, 0.50f),
				1.00f * Vector3f(0.75f, 1.00f, 0.75f),
				1.00f * Vector3f(0.50f, 0.75f, 1.00f)
			};

			const int TempLightPower = Math::Clamp(LightPower, 0ULL, _countof(LightPowerLookUpTable) - 1ULL);

			switch (Type)
			{
				case LIGHT_OMNI:
				{
					return LightPowerLookUpTable[TempLightPower] * 25000.0f;
				}

				case LIGHT_SPOT:
				{
					return LightPowerLookUpTable[TempLightPower] * 50000.0f;
				}
			}

			return LightPowerLookUpTable[TempLightPower];
		}

		LightProperties::LightProperties(const Default & Default)
		{
			AmbientColor			= RGBAColor(1.0f);
			DiffuseColor			= RGBAColor(1.0f);
			BloomColor				= RGBAColor(1.0f, 1.0f, 1.0f, 0.0f);
			BloomScale				= 0.2f;
			Radius					= FLT_MAX;
			SourceRadius			= 0.0f;
			LightPower				= 0;
			OcclusionMaskDarkness	= 0.05f;
			OcclusionMaskDarkness	= 100000.0f;

			Omni.AttenuationMode		= AttenuationMode::INV_POLYNOMIAL;
			Omni.AttenuationFactors[0]	= 1.0f;
			Omni.AttenuationFactors[1]	= 2.0f / (GLightRange);
			Omni.AttenuationFactors[2]	= 1.0f / (GLightRange * GLightRange);
			Omni.AttenuationFactors[3]	= 1.0f;

			Omni.ZNear	= 0.5f;
			Omni.ZFar	= GLightRange;

			Type = LIGHT_DIRECTIONAL;

			SetPosition(Vector3f::ZeroVector);
		}
	}

	ErrorCode CSceneLight::CRenderer::Create()
	{
		try
		{
			SceneLight->VolumetricLighting.Finalize(SceneRenderer->GetShadowMapCommandContext());
		}
		catch (ECException & Exception)
		{
			return Exception.GetErrorCode();
		}

		return S_OK;
	}

	CSceneLight::CRenderer::CRenderer(const CSceneRenderer * pSceneRenderer)
	{
		Ensure
		(
			pSceneRenderer
		);

		Scene			= pSceneRenderer->GetScene();
		SceneRenderer	= pSceneRenderer;
		SceneLight		= &Scene->GetLight();
	}

	void CSceneLight::CRenderer::BeginAccumulation() const
	{
		SceneLight->VolumetricLighting.BeginAccumulation_Start(Scene);

		SceneLight->VolumetricLighting.BeginAccumulation_UpdateMediumLUT();
		SceneLight->VolumetricLighting.BeginAccumulation_CopyDepth(Scene);

		SceneLight->VolumetricLighting.BeginAccumulation_End();
	}

	void CSceneLight::CRenderer::EndAccumulation() const
	{
		
	}

	void CSceneLight::CRenderer::ApplyLighting(const Light::PostprocessDesc & Postprocess) const
	{
		SceneLight->VolumetricLighting.ApplyLighting_Start(Scene.Get(), Postprocess);

		if (SceneLight->VolumetricLighting.GetFilterMode() == Light::FILTER_TEMPORAL)
		{
			SceneLight->VolumetricLighting.ApplyLighting_Resolve(Scene.Get(), Postprocess);
			SceneLight->VolumetricLighting.ApplyLighting_TemporalFilter(Scene.Get(), Postprocess);
		}

		SceneLight->VolumetricLighting.ApplyLighting_Composite(Scene.Get(), Postprocess);
	}

	void CSceneLight::CRenderer::StartVolumeRender() const
	{
		SceneLight->VolumetricLighting.RenderVolume_Start(Scene, SceneLight->ShadowMap);

		if (SceneLight->GetProperties().Type == Light::LIGHT_DIRECTIONAL)
		{
			SceneLight->VolumetricLighting.RenderVolume_Directional(Scene, SceneLight->ShadowMap);
		}

		SceneLight->VolumetricLighting.RenderVolume_End();
	}

	CSceneLight::CSceneLight() : Properties(GlobalDefault)
	{}

	CSceneLight::CSceneLight(const Light::LightProperties & Properties) : Properties(Properties)
	{}

	ErrorCode CSceneLight::Initialize(const CScene * pScene)
	{
		ErrorCode Error;

		if ((Error = VolumetricLighting.Create(pScene)))
		{
			return Error;
		}

		D3D12_RESOURCE_DESC ShadowMapResourceDesc = pScene->GetShadowDSV()->GetResource()->Get()->GetDesc();
		{
			ShadowMap.uWidth	= ShadowMapResourceDesc.Width;
			ShadowMap.uHeight	= ShadowMapResourceDesc.Height;

			ShadowMap.Elements[0].uOffsetX		= 0;
			ShadowMap.Elements[0].uOffsetY		= 0;
			ShadowMap.Elements[0].uWidth		= ShadowMap.uWidth;
			ShadowMap.Elements[0].uHeight		= ShadowMap.uHeight;
			ShadowMap.Elements[0].mViewProj		= Properties.LightViewProjectionMatrix;
			ShadowMap.Elements[0].mArrayIndex	= 0;

			if (Properties.Type == Light::LIGHT_OMNI)
			{
				ShadowMap.uElementCount = 2;

				ShadowMap.Elements[1].uOffsetX		= 0;
				ShadowMap.Elements[1].uOffsetY		= 0;
				ShadowMap.Elements[1].uWidth		= ShadowMap.uWidth;
				ShadowMap.Elements[1].uHeight		= ShadowMap.uHeight;
				ShadowMap.Elements[1].mViewProj		= Properties.LightViewProjectionMatrix;
				ShadowMap.Elements[1].mArrayIndex	= 1;

				ShadowMap.eType = Light::ShadowMapLayout::PARABOLOID;
			}
			else
			{
				ShadowMap.uElementCount = 1;

				ShadowMap.eType = Light::ShadowMapLayout::SIMPLE;
			}

			ShadowMapViewport.Height	= ShadowMapResourceDesc.Height;
			ShadowMapViewport.Width		= ShadowMapResourceDesc.Width;
			ShadowMapViewport.MaxDepth	= 1.0f;
			ShadowMapViewport.MinDepth	= 0.0f;
			ShadowMapViewport.TopLeftX	= 0.0f;
			ShadowMapViewport.TopLeftY	= 0.0f;

			ShadowMapScissorRect.bottom = ShadowMapResourceDesc.Height;
			ShadowMapScissorRect.top	= ShadowMapResourceDesc.Width;
			ShadowMapScissorRect.left	= 0;
			ShadowMapScissorRect.right	= 0;
		}

		SetMediumType(0);

		return S_OK;
	}

	void CSceneLight::SetMediumType(const uint32_t Type)
	{
		const float ScatterParamScale = 0.0001f;

		uint32_t Term = 0;

		Medium.PhaseTerms[Term].ePhaseFunc			= Light::PhaseFunctionType::RAYLEIGH;
		Medium.PhaseTerms[Term].vDensity			= 10.00f * ScatterParamScale * Vector3f(0.596f, 0.524f, 0.610f);
		Medium.PhaseTerms[Term].fEccentricity		= 0.0f;
		Term++;

		switch (Type)
		{
			default: case 0:
			{
				Medium.PhaseTerms[Term].ePhaseFunc		= Light::PhaseFunctionType::HENYEYGREENSTEIN;
				Medium.PhaseTerms[Term].vDensity		= 10.00f * ScatterParamScale * Vector3f(1.00f, 1.00f, 1.00f);
				Medium.PhaseTerms[Term].fEccentricity	= 0.85f;
				Term++;
				Medium.Absorption = 5.0f * ScatterParamScale * Vector3f(1, 1, 1);
			}

			break;

			case 1:
			{
				Medium.PhaseTerms[Term].ePhaseFunc		= Light::PhaseFunctionType::HENYEYGREENSTEIN;
				Medium.PhaseTerms[Term].vDensity		= 15.00f * ScatterParamScale * Vector3f(1.00f, 1.00f, 1.00f);
				Medium.PhaseTerms[Term].fEccentricity	= 0.60f;
				Term++;
				Medium.Absorption = 25.0f * ScatterParamScale * Vector3f(1, 1, 1);
			}

			break;

			case 2:
			{
				Medium.PhaseTerms[Term].ePhaseFunc		= Light::PhaseFunctionType::MIE_HAZY;
				Medium.PhaseTerms[Term].vDensity		= 20.00f * ScatterParamScale * Vector3f(1.00f, 1.00f, 1.00f);
				Medium.PhaseTerms[Term].fEccentricity	= 0.0f;
				Term++;
				Medium.Absorption = 25.0f * ScatterParamScale * Vector3f(1, 1, 1);
			}

			break;

			case 3:
			{
				Medium.PhaseTerms[Term].ePhaseFunc		= Light::PhaseFunctionType::MIE_MURKY;
				Medium.PhaseTerms[Term].vDensity		= 30.00f * ScatterParamScale * Vector3f(1.00f, 1.00f, 1.00f);
				Medium.PhaseTerms[Term].fEccentricity	= 0.0f;
				Term++;
				Medium.Absorption = 50.0f * ScatterParamScale * Vector3f(1, 1, 1);
			}

			break;
		}

		Medium.NumPhaseTerms = Term;
	}
}
