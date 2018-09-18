#pragma once

#include "Scene/Scene.h"

namespace D3D
{
	namespace PostProcess
	{
		struct PassContext
		{
			ConstPointer<CCommandListContext> CommandListCtx;

			DescriptorHeapRange DescriptorHeapRangeSRV;
			DescriptorHeapRange DescriptorHeapRangeCBV;
		};

		struct PostProcessTarget
		{
			LongPoint						SizeSource;
			LongPoint						SizeDestination;
			D3D12_RECT						ViewRect;
			D3D12_VIEWPORT					ViewPort;
			ConstPointer<RRenderTargetView>	RenderTarget;
		};

		struct PostProcessSettings
		{
			Float AutoExposureLowPercent;
			Float AutoExposureHighPercent;
			Float AutoExposureMinBrightness;
			Float AutoExposureMaxBrightness;
			Float AutoExposureBias;
			Float AutoExposureSpeedUp;
			Float AutoExposureSpeedDown;
			Float HistogramLogMin;
			Float HistogramLogMax;

			PostProcessSettings() = default;
			PostProcessSettings
			(
				const Default &
			)
			{
				AutoExposureMinBrightness	= 1.0f;
				AutoExposureMaxBrightness	= 1.0f;
				AutoExposureLowPercent		= 70.0f;
				AutoExposureMaxBrightness	= 90.0f;
				AutoExposureSpeedUp			= 0.0f;
				AutoExposureSpeedDown		= 0.0f;
				AutoExposureBias			= 0.0f;
				HistogramLogMax				= 16.0f;
				HistogramLogMin				= 1.0f / 256.0f;
			}
		};

		class CPostProcessBloom;
		class CPostProcessDOF;
		class CPostProcess
		{
		public:

			struct Constants
			{
				Vector4f ViewportRect;
				Vector4f ViewportSize;
				Vector4f PostprocessInput0Size;
			};

		private:

			ConstPointer<CScene>			Scene;
			ConstPointer<CSceneRenderer>	SceneRenderer;
			ConstPointer<CPostProcessBloom>	PostProcessBloom;
			ConstPointer<CPostProcessDOF>	PostProcessDOF;
			UniquePointer<CConstantBuffer>	PostProcessConstants;

			Uint ProcessBloom;
			Uint ProcessDOF;

			PostProcessSettings Settings;

			PassContext BloomSetupPass;
			PassContext BloomOverlayPass;

			PassContext DOFBlurPass;
			PassContext DOFCombinePass;

		public:

			CPostProcess
			(
				const CSceneRenderer * SceneRenderer
			);

			ErrorCode Create();

			void Process
			(
				const PostProcessTarget & Target
			);

			void ToggleBloom();
			void ToggleDOF();
			void Update();

		public:

			static inline float ComputeExposureScaleValue
			(
				const PostProcessSettings & Settings
			)
			{
				Vector4f EyeAdaptationParams[3];

				ComputeEyeAdaptationValues(EyeAdaptationParams, Settings);

				float Exposure = (EyeAdaptationParams[0].Z + EyeAdaptationParams[0].W) * 0.5f;
				float ExposureScale = 1.0f / Math::Max(0.0001f, Exposure);
				float ExposureOffsetMultipler = EyeAdaptationParams[1].X;

				return ExposureScale * ExposureOffsetMultipler;
			}

			static inline void ComputeEyeAdaptationValues
			(
						Vector4f				Out[3], 
				const	PostProcessSettings &	Settings
			)
			{
				float EyeAdaptationMin = Settings.AutoExposureMinBrightness;
				float EyeAdaptationMax = Settings.AutoExposureMaxBrightness;

				float LocalOverrideExposure = std::numeric_limits<float>::max();
				float LocalExposureMultipler = Math::Pow(2.0f, Settings.AutoExposureBias);

				if (EyeAdaptationMin > EyeAdaptationMax)
				{
					EyeAdaptationMin = EyeAdaptationMax;
				}

				float LowPercent = Math::Clamp(Settings.AutoExposureLowPercent, 1.0f, 99.0f) * 0.01f;
				float HighPercent = Math::Clamp(Settings.AutoExposureHighPercent, 1.0f, 99.0f) * 0.01f;

				if (LowPercent > HighPercent)
				{
					LowPercent = HighPercent;
				}

				Out[0] = Vector4f(LowPercent, HighPercent, EyeAdaptationMin, EyeAdaptationMax);
				Out[1] = Vector4f(LocalExposureMultipler, 0.0f, Settings.AutoExposureSpeedUp, Settings.AutoExposureSpeedDown);

				float HistogramLogMin = Settings.HistogramLogMin;
				float HistogramLogMax = Settings.HistogramLogMax;
				float DeltaLog = HistogramLogMax - HistogramLogMin;
				float Multiply = 1.0f / DeltaLog;
				float Add = -HistogramLogMin * Multiply;
				float MinIntensity = Math::Exp2(HistogramLogMin);

				Out[2] = Vector4f(Multiply, Add, MinIntensity, 0);
			}
		};
	}
}