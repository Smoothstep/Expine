#pragma once

#include "Raw/RawRenderTarget.h"
#include "Raw/RawShaderResourceView.h"
#include "Raw/RawDepthStencilView.h"

#include "Pipeline/Pipelines.h"

#include "Command/CommandList.h"

namespace D3D
{
	class CScene;

	namespace Light
	{
		constexpr uint32_t MAX_JITTER_STEPS = 8;
		constexpr uint32_t MAX_PHASE_TERMS = 4;
		constexpr uint32_t LIGHT_LUT_DEPTH_RESOLUTION = 128;
		constexpr uint32_t LIGHT_LUT_WDOTV_RESOLUTION = 512;

		enum EFilterMode
		{
			FILTER_NONE,
			FILTER_TEMPORAL
		};

		enum EDownsampleMode
		{
			DOWNSAMPLE_FULL,
			DOWNSAMPLE_HALF,
			DOWNSAMPLE_QUARTER
		};

		enum ESampleMode
		{
			SAMPLE_SINGLE,
			SAMPLE_MSAA2,
			SAMPLE_MSAA4
		};

		//! Specifies the type of distance attenuation applied to the light
		enum class AttenuationMode
		{
			UNKNOWN = -1,
			NONE,           //!< No attenuation
			POLYNOMIAL,	    //!< f(x) = 1-(A+Bx+Cx^2)
			INV_POLYNOMIAL, //!< f(x) = 1/(A+Bx+Cx^2)+D
			COUNT
		};

		//! Specifies the type of angular falloff to apply to the spotlight
		enum class SpotlightFalloffMode
		{
			UNKNOWN = -1,
			NONE,   //!< No falloff (constant brightness across cone cross-section)
			FIXED,	//!< A_fixed(vL, vP) = (dot(vL, vP) - theta_max)/(1 - theta_max)
			CUSTOM, //!< A_custom(vL, vP) = (A_fixed(vL, vP))^n
			COUNT
		};

		//! Amount of tessellation to use
		enum class TessellationQuality
		{
			UNKNOWN = -1,
			LOW,        //!< Low amount of tessellation (16x)
			MEDIUM,     //!< Medium amount of tessellation (32x)
			HIGH,       //!< High amount of tessellation (64x)
			COUNT
		};

		//! Quality of upsampling
		enum class UpsampleQuality
		{
			UNKNOWN = -1,
			POINT,		//!< Point sampling (no filter)
			BILINEAR,	//!< Bilinear Filtering
			BILATERAL,	//!< Bilateral Filtering (using depth)
			COUNT
		};

		enum class PhaseFunctionType
		{
			UNKNOWN = -1,
			ISOTROPIC,	        //!< Isotropic scattering (equivalent to HG with 0 eccentricity, but more efficient)
			RAYLEIGH,	        //!< Rayleigh scattering term (air/small molecules)
			HENYEYGREENSTEIN,	//!< Scattering term with variable anisotropy
			MIE_HAZY,	        //!< Slightly forward-scattering
			MIE_MURKY,	        //!< Densely forward-scattering
			COUNT
		};

		//! Specifies the geometric mapping of the shadow map
		enum class ShadowMapLayout
		{
			UNKNOWN = -1,
			SIMPLE,			//!< Simple frustum depth texture
			CASCADE_ATLAS,	//!< Multiple depth views combined into one texture
			CASCADE_ARRAY,	//!< Multiple depth views as texture array slices
			PARABOLOID,     //!< Depth mapped using paraboloid warping
			COUNT
		};

		struct ShadowMapElementDesc
		{
			Matrix4x4 mViewProj;		//!< View-Proj transform for cascade
			uint32_t uOffsetX;		//!< X-offset within texture
			uint32_t uOffsetY;		//!< Y-offset within texture
			uint32_t uWidth;		//!< Footprint width within texture
			uint32_t uHeight;		//!< Footprint height within texture
			uint32_t mArrayIndex;   //!< Texture array index for this element (if used)
		};

		//! Maximum number of sub-elements in a shadow map set
		constexpr uint32_t MAX_SHADOWMAP_ELEMENTS = 4;

		//! Shadow Map Structural Description
		struct ShadowMapDesc
		{
			ShadowMapLayout eType;	//!< Shadow map structure type
			uint32_t uWidth; 		//!< Shadow map texture width
			uint32_t uHeight;		//!< Shadow map texture height
			uint32_t uElementCount; //!< Number of sub-elements in the shadow map

									//! Individual cascade descriptions
			ShadowMapElementDesc Elements[MAX_SHADOWMAP_ELEMENTS];
		};

		struct PhaseTerm
		{
			PhaseFunctionType	ePhaseFunc;	//!< Phase function this term uses
			Vector3f			vDensity;			    //!< Optical density in [R,G,B]
			Float				fEccentricity;		    //!< Degree/direction of anisotropy (-1, 1) (HG only)
		};

		struct MediumDesc
		{
			Vector3f	Absorption;
			uint32_t	NumPhaseTerms;
			PhaseTerm	PhaseTerms[MAX_PHASE_TERMS];
		};

		struct VolumeDesc
		{
			Float				DepthBias;
			Float				TargetRayResolution;
			uint32_t			MaxResolution;
			TessellationQuality TessQuality;

			inline uint32_t GetCoarseResolution() const
			{
				switch (TessQuality)
				{
					default: case TessellationQuality::HIGH:
					{
						return MaxResolution / 64;
					}

					case TessellationQuality::MEDIUM:
					{
						return MaxResolution / 32;
					}

					case TessellationQuality::LOW:
					{
						return MaxResolution / 16;
					}
				}
			}
		};

		struct PostprocessDesc
		{
			Matrix4x4		mUnjitteredViewProj;		//!< Camera view projection without jitter
			Float			fTemporalFactor;			//!< Weight of pixel history smoothing (0.0 for off)
			Float			fFilterThreshold;			//!< Threshold of frame movement to use temporal history
			UpsampleQuality eUpsampleQuality;			//!< Quality of upsampling to use
			Vector3f		vFogLight;                  //!< Light to use as "faked" multiscattering
			Float			fMultiscatter;              //<! strength of faked multiscatter effect
			bool			bDoFog;						//!< Apply fogging based on scattering
			bool			bIgnoreSkyFog;				//!< Ignore depth values of (1.0f) for fogging
			Float			fBlendfactor;				//!< Blend factor to use for compositing
		};

		class CVolumetricLighting
		{
		public:

			enum ERenderTargets
			{
				TARGET_LUT,
				TARGET_LUT_P_0,
				TARGET_LUT_P_1,
				TARGET_LUT_S0_0,
				TARGET_LUT_S0_1,
				TARGET_LUT_S1_0,
				TARGET_LUT_S1_1,
				TARGET_LUT_S2_0,
				TARGET_LUT_S2_1,
				TARGET_ACCUMULATION,
				TARGET_ACCUMULATION_RESOLVED,
				TARGET_DEPTH_RESOLVED,
				TARGET_ACCUMULATION_FILTERED_0,
				TARGET_DEPTH_FILTERED_0,
#if FRAME_COUNT > 1
				TARGET_ACCUMULATION_FILTERED_1,
				TARGET_DEPTH_FILTERED_1,
#endif
#if FRAME_COUNT > 2
				TARGET_ACCUMULATION_FILTERED_2,
				TARGET_DEPTH_FILTERED_2,
#endif
				NumRenderTargets
			};

			enum EShaderResources
			{
				CBV_PER_CONTEXT,
				CBV_PER_FRAME,
				CBV_PER_VOLUME,
				CBV_PER_APPLY,
				SRV_DEPTH_LOCAL,
				SRV_LUT,
				SRV_LUT_P0,
				SRV_LUT_P1,
				SRV_S0_0,
				SRV_S0_1,
				SRV_S1_0,
				SRV_S1_1,
				SRV_S2_0,
				SRV_S2_1,
				SRV_ACCUMULATION,
				SRV_ACCUMULATION_RESOLVED,
				SRV_DEPTH_RESOLVED,
				SRV_ACCUMULATION_FILTERED_0,
				SRV_DEPTH_FILTERED_0,
#if FRAME_COUNT > 1
				SRV_ACCUMULATION_FILTERED_1,
				SRV_DEPTH_FILTERED_1,
#endif
#if FRAME_COUNT > 2
				SRV_ACCUMULATION_FILTERED_2,
				SRV_DEPTH_FILTERED_2,
#endif
				NumShaderResources
			};

			struct ContextConstants
			{
				Vector2f	vOutputSize;
				Vector2f	vOutputSize_Inv;
				Vector2f	vBufferSize;
				Vector2f	vBufferSize_Inv;
				Float		fResMultiplier;
				uint32_t	uSampleCount; PADDING(2);
			};

			struct FrameConstants
			{
				Matrix4x4	mProj;
				Matrix4x4	mViewProj;
				Matrix4x4	mViewProj_Inv;
				Vector2f	vOutputViewportSize;
				Vector2f	vOutputViewportSize_Inv;
				Vector2f	vViewportSize;
				Vector2f	vViewportSize_Inv;
				Vector3f	vEyePosition; PADDING(1);
				Vector2f	vJitterOffset;
				Float		fZNear;
				Float		fZFar;
				Vector3f	vScatterPower;
				uint32_t	uNumPhaseTerms;
				Vector3f	vSigmaExtinction; PADDING(1);
				uint32_t	uPhaseFunc[4][4];
				Vector4f	vPhaseParams[4];
			};

			struct VolumeConstants
			{
				Matrix4x4	mLightToWorld;
				Float		LightFalloffAngle;
				Float		fLightFalloffPower;
				Float		fGridSectionSize;
				Float		fLightToEyeDepth;
				Float		fLightZNear;
				Float		fLightZFar;
				Float2		Padding;
				Float4		vAttenuationFactors;
				Matrix4x4	mLightProj[4];
				Matrix4x4	mLightProj_Inv[4];
				Vector3f	vLightDir;
				Float		fDepthBias;
				Vector3f	vLightPos;
				uint32_t	uMeshResolution;
				Vector3f	vLightIntensity;
				Float		fTargetRaySize;
				Vector4f	vElementOffsetAndScale[4];
				Vector4f	vShadowMapDim;
				uint32_t	uElementIndex[4][4];
			};

			struct ApplyConstants
			{
				Matrix4x4	mHistoryXform;
				Float		fFilterThreshold;
				Float		fHistoryFactor;	PADDING(2);
				Vector3f	vFogLight;
				Float		fMultiScattering;
			};

		private:

			SharedPointer<RDescriptorHeap>				DescriptorHeap;
			SharedPointer<RDescriptorHeap>				DescriptorHeapPerFrame;
			SharedPointer<RDescriptorHeap>				DescriptorHeapDSV;
			DescriptorHeapRange							DescriptorHeapRangeLocal;
			DescriptorHeapRange							DescriptorHeapRangeSceneSRV;
			DescriptorHeapRange							DescriptorHeapRangeSceneCBV;
			UniquePointer<CConstantBuffer>				ConstantBuffer;
			RenderTargetViewGroup<NumRenderTargets>		RenderTargets;
			ShaderResourceViewGroup<NumRenderTargets>	ShaderResources;
			UniquePointer<RDepthStencilView>			DepthStencil;
			UniquePointer<RDepthStencilView>			DepthStencilReadOnly;
			SharedPointer<RShaderResourceView>			DepthStencilSRV;
			ConstPointer<CCommandListContext>			CmdListCtx;

		protected:

			EFilterMode	FilterMode			= FILTER_NONE;
			ESampleMode	SampleMode			= SAMPLE_SINGLE;
			EDownsampleMode	DownsampleMode	= DOWNSAMPLE_FULL;

			VolumeDesc VolumeSettings;

		protected:

			D3D12_VIEWPORT	LUTViewport;
			D3D12_RECT		LUTScissorRect;

		protected:

			Matrix4x4 LastViewProjectionMatrix  = Matrix4x4::Identity();
			Matrix4x4 NextViewProjectionMatrix	= Matrix4x4::Identity();

		private:

			int JitterIndex = 0;

		public:

			CVolumetricLighting();
			CVolumetricLighting
			(
				const CCommandListContext * pCmdListCtx
			);

			inline EFilterMode GetFilterMode() const
			{
				return FilterMode;
			}

		private:

			ErrorCode CreateDescriptorHeaps
			(
				const CScene * pScene
			);
			ErrorCode CreateConstantBuffers();
			ErrorCode CreateViews
			(
				const CScene * pScene
			);

			Vector2f GetJitter() const;

			inline uint32_t GetInternalBufferSize(uint32_t Size) const
			{
				switch (DownsampleMode)
				{
					default: case EDownsampleMode::DOWNSAMPLE_FULL:
					{
						return Size;
					}

					case EDownsampleMode::DOWNSAMPLE_HALF:
					{
						return Size >> 1;
					}

					case EDownsampleMode::DOWNSAMPLE_QUARTER:
					{
						return Size >> 2;
					}
				}
			}

			inline uint32_t GetInternalSampleCount() const
			{
				switch (SampleMode)
				{
					default: case ESampleMode::SAMPLE_SINGLE:
					{
						return 1;
					}

					case ESampleMode::SAMPLE_MSAA2:
					{
						return 2;
					}

					case ESampleMode::SAMPLE_MSAA4:
					{
						return 4;
					}
				}
			}

			inline float GetInternalScale() const
			{
				switch (DownsampleMode)
				{
					default: case EDownsampleMode::DOWNSAMPLE_FULL:
					{
						return 1.00f;
					}

					case EDownsampleMode::DOWNSAMPLE_HALF:
					{
						return 0.50f;
					}

					case EDownsampleMode::DOWNSAMPLE_QUARTER:
					{
						return 0.25f;
					}
				}
			}

			void SetupPerContextConstants
			(
				const CScene * pScene
			);

			void SetupPerFrameConstants
			(
				const CScene * pScene
			);

			void SetupPerVolumeConstants
			(
				const CScene		* pScene,
				const VolumeDesc	& Volume,
				const ShadowMapDesc & ShadowMap
			);

			void SetupPerApplyConstants
			(
				const CScene * pScene,
				const PostprocessDesc & PostProcess
			);

		public:

			ErrorCode Create
			(
				const CScene * pScene
			);

			void Finalize
			(
				const CCommandListContext * pCmdListCtx
			);

			void ApplyLighting_Start
			(
				const CScene			* pScene,
				const PostprocessDesc	& Postprocess
			);
			void ApplyLighting_Resolve
			(
				const CScene			* pScene,
				const PostprocessDesc	& Postprocess
			);
			void ApplyLighting_TemporalFilter
			(
				const CScene			* pScene,
				const PostprocessDesc	& Postprocess
			);
			void ApplyLighting_Composite
			(
				const CScene			* pScene,
				const PostprocessDesc	& Postprocess
			);
			void ApplyLighting_End
			(
				const CScene			* pScene,
				const PostprocessDesc	& Postprocess
			);

			void BeginAccumulation_Start
			(
				const CScene * pScene
			);
			void BeginAccumulation_UpdateMediumLUT();
			void BeginAccumulation_CopyDepth
			(
				const CScene * pScene
			)	const;
			void BeginAccumulation_End();

			void RenderVolume_Start
			(
				const CScene		* pScene, 
				const ShadowMapDesc & ShadowMap
			);
			void RenderVolume_Directional
			(
				const CScene		* pScene,
				const ShadowMapDesc & ShadowMap
			);
			void RenderVolume_End();
		};
	}
}