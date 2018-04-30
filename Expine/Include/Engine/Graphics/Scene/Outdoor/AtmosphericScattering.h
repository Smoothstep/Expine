#pragma once

#include "RawRenderTarget.h"
#include "RawShaderResourceView.h"

#include "PipelinesAtmosphere.h"

#include "CommandList.h"

namespace D3D
{
	class CScene;
	class CSceneRenderer;

	namespace Atmosphere
	{
		const DXGI_FORMAT InscatterFormat		= DXGI_FORMAT_R32G32B32A32_FLOAT;
		const DXGI_FORMAT TransmittanceFormat	= DXGI_FORMAT_R32G32B32A32_FLOAT;
		const DXGI_FORMAT IrradianceFormat		= DXGI_FORMAT_R32G32B32A32_FLOAT;

		using namespace Pipelines::AtmosphericScattering;

		enum EStages
		{
			AP_Transmittance = 0,
			AP_Irradiance1,
			AP_Inscatter1,
			AP_ClearIrradiance,
			AP_CopyInscatter1,
			AP_StartOrder,
			AP_InscatterS,
			AP_IrradianceN,
			AP_InscatterN,
			AP_CopyIrradiance,
			AP_CopyInscatterN,
			AP_EndOrder,
			AP_CopyInscatterF,
			AP_CopyInscatterFBack,
			AP_MAX
		};

		struct AtmosphereParameters
		{
			UINT InscatterAltitudeSampleNum;
			FLOAT DensityHeight;
			UINT MaxOrdering;
			UINT TransmittanceTexWidth;
			UINT TransmittanceTexHeight;
			UINT IrradianceTexWidth;
			UINT IrradianceTexHeight;
			UINT InscatterMuSNum;
			UINT InscatterNuNum;

			AtmosphereParameters() = default;
			AtmosphereParameters
			(
				const Default &
			);
		};

		struct AtmosphereConstants
		{
			FLOAT AtmosphericFogInscatterAltitudeSampleNum;
			FLOAT AtmosphericFogHeightScaleRayleigh;
			FLOAT AtmosphericFogSunDiscScale;
			FLOAT AtmosphericFogDistanceScale;
			FLOAT AtmosphericFogStartDistance;
			FLOAT AtmosphericFogGroundOffset;
			FLOAT AtmosphericFogAltitudeScale;
			FLOAT AtmosphericFogPower;
			FLOAT AtmosphericFogDistanceOffset; PADDING(3);
			Vector4f AtmosphericFogSunDirection;
			FLOAT AtmosphericFogSunPower;		PADDING(3);
			Vector4f AtmosphericFogSunColor;
			FLOAT AtmosphericFogDensityScale;
			FLOAT AtmosphericFogDensityOffset;	PADDING(2);

			AtmosphereConstants() = default;
			AtmosphereConstants
			(
				const AtmosphereParameters & AtmosphereProperties
			);
		};

		struct AtmosphereInscatterConstants
		{
			Vector4f	DhdH;
			FLOAT		AtmosphereR;
			UINT		AtmosphereLayer;
			FLOAT		FirstOrder;

			inline AtmosphereInscatterConstants() = default;
			inline AtmosphereInscatterConstants
			(
				const Default &
			)
			{
				DhdH			= Vector4f::ZeroVector;
				AtmosphereR		= 0.0;
				AtmosphereLayer = 0;
				FirstOrder		= 0.0;
			}
		};

		class CScattering
		{
		private:

			RenderTargetViewGroup<NumSRV>			RenderTargets;
			ShaderResourceViewGroup<NumSRV>			ShaderResources;
			DescriptorHeapRange						ViewDescriptorHeapRange;
			DescriptorHeapRange						ViewDescriptorHeapRangeSRV;
			DescriptorHeapRange						ViewDescriptorHeapRangeCBV;
			SharedPointer<RDescriptorHeap>			ViewDescriptorHeap;
			SharedPointer<CConstantBuffer>			ConstantBuffer;
			SharedPointer<CVertexBuffer>			FogVertexBuffer;
			SharedPointer<CIndexBuffer>				FogIndexBuffer;
			ConstPointer<CCommandListContext>		CommandListContext;
			AtmosphereConstants						AtmosphereDescriptor;
			AtmosphereInscatterConstants			AtmosphereInscatterDescriptor;
			AtmosphereParameters					AtmosphereProperties;

			UINT									TextureIndex	= 0;
			UINT									Phase			= AP_Transmittance;
			UINT									Order			= 2;

		protected:

			ErrorCode InitializeViews();
			ErrorCode InitializeBuffers();

		private:

			void PrepareRenderer
			(
				const CSceneRenderer * pScene
			);
			void UpdateRenderer();

			void RenderTransmittance();
			void RenderIrradiance();
			void RenderInscatter();
			void ClearIrradiance();
			void CopyInscatter();
			void InscatterS();
			void IrradianceN();
			void InscatterN();
			void CopyIrradiance();
			void CopyInscatterN();
			void CopyInscatterF();
			void CopyInscatterFBack();

			void GetLayerValue
			(
				Uint		iLayer,
				Float	 &	fAtmosphereR,
				Vector4f &	DhdH
			);

			void GetCurrentRenderView
			(
				D3D12_VIEWPORT	&	Viewport,
				D3D12_RECT		&	ScissorRect
			);

		public:

			CScattering();
			~CScattering();

			ErrorCode Initialize();	
			ErrorCode Precompute
			(
				const CSceneRenderer * pSceneRenderer
			);
			void Finalize
			(
				const CCommandListContext * pCommandListCtx
			);
			void Render
			(
				const CSceneRenderer * pSceneRenderer
			);

			bool Finished();
		};
	}
}