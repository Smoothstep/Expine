#pragma once

#include "RawCommandList.h"
#include "RawPipelineState.h"
#include "ConstantBuffer.h"

namespace D3D
{
	namespace Pipelines
	{
		class PipelineColorLine : public CSingleton<PipelineColorLine>, public PipelineObject<PipelineColorLine>
		{
			enum ERootParameters
			{
				CBV_Transform
			};

		private:

			virtual ErrorCode CreateRootSignature() override;
			virtual ErrorCode CreatePipelineState() override;

		public:

			virtual ErrorCode Initialize();

		public:

			virtual inline void Apply
			(
				RGrpCommandList * CmdList
			)	const
			{
				CmdList->ApplyPipelineState(PipelineState);
				CmdList->SetCommandSignature(CommandSignature);
			}

			virtual inline void SetConstantBuffer
			(
				const RGrpCommandList & CmdList,
				const CConstantBuffer & ConstantBuffer
			)	const
			{
				CmdList.SetConstantBuffer(CBV_Transform, ConstantBuffer);
			}
		};

		class PipelineColorTriangle : public CSingleton<PipelineColorTriangle>, public PipelineObject<PipelineColorTriangle>
		{
			enum ERootParameters
			{
				CBV_Transform
			};

		private:

			virtual ErrorCode CreateRootSignature() override;
			virtual ErrorCode CreatePipelineState() override;

		public:

			virtual ErrorCode Initialize();

		public:

			virtual inline void Apply
			(
				RGrpCommandList * CmdList
			)	const
			{
				CmdList->ApplyPipelineState(PipelineState);
				CmdList->SetCommandSignature(CommandSignature);
			}

			virtual inline void SetConstantBuffer
			(
				const RGrpCommandList & CmdList,
				const CConstantBuffer & ConstantBuffer
			)	const
			{
				CmdList.SetConstantBuffer(CBV_Transform, ConstantBuffer);
			}
		};

		class PipelineWireframeTriangle : public CSingleton<PipelineWireframeTriangle>, public PipelineObject<PipelineWireframeTriangle>
		{
			enum ERootParameters
			{
				CBV_Transform
			};

		private:

			virtual ErrorCode CreateRootSignature() override;
			virtual ErrorCode CreatePipelineState() override;

		public:

			virtual ErrorCode Initialize();

		public:

			virtual inline void Apply
			(
				RGrpCommandList * CmdList
			)	const
			{
				CmdList->ApplyPipelineState(PipelineState);
				CmdList->SetCommandSignature(CommandSignature);
			}

			virtual inline void SetConstantBuffer
			(
				const RGrpCommandList & CmdList,
				const CConstantBuffer & ConstantBuffer
			)	const
			{
				CmdList.SetConstantBuffer(CBV_Transform, ConstantBuffer);
			}
		};

		class PipelineComposite : public CSingleton<PipelineComposite>, public PipelineObject<PipelineComposite>
		{
		public:

			enum ERootParameters
			{
				SceneConstants,
				SceneResources
			};

			enum EDescriptors
			{
				BloomOverlay,
				Noise,
				LightingGGX,
				NumDescriptors
			};

		public:

			virtual ErrorCode CreateRootSignature() override;
			virtual ErrorCode CreatePipelineState() override;

		public:

			virtual inline void Apply
			(
				RGrpCommandList * CmdList
			)	const
			{
				CmdList->ApplyPipelineState(PipelineState);
			}
		};

		class PipelinePresent : public CSingleton<PipelinePresent>, public PipelineObject<PipelinePresent>
		{
		public:

			enum ERootParameters
			{
				SceneComposite
			};

		public:

			virtual ErrorCode CreateRootSignature() override;
			virtual ErrorCode CreatePipelineState() override;

		public:

			virtual inline void Apply
			(
				RGrpCommandList * CmdList
			)	const
			{
				CmdList->ApplyPipelineState(PipelineState);
			}
		};

		namespace VolumetricLighting
		{
			enum EConstantBuffers
			{
				CB_CONTEXT,
				CB_FRAME,
				CB_VOLUME,
				CB_APPLY,
				NumConstantBuffers
			};

			enum ERenderTargets
			{
				TARGET_DEPTH = NumConstantBuffers,
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
				TARGET_ACCUMULATION_OUTPUT,
				TARGET_DEPTH_OUTPUT,
				NumRenderTargets
			};

			enum EFrameRenderTargets
			{
				TARGET_ACCUMULATION_FILTERED,
				TARGET_DEPTH_FILTERED,
				NumFrameRenderTargets
			};

			class DownsamplePipeline : public CSingleton<DownsamplePipeline>, public PipelineObject<DownsamplePipeline>
			{
			private:

				virtual ErrorCode CreateRootSignature() override;
				virtual ErrorCode CreatePipelineState() override;

			public:

				inline void Apply
				(
					RGrpCommandList * CmdList
				)	const
				{
					CmdList->ApplyPipelineState(PipelineState);
				}
			};

			class ResolvePSPipeline : public CSingleton<ResolvePSPipeline>, public PipelineObject<ResolvePSPipeline>
			{
			public:

				enum ERootParameters
				{
					PARAMETER_CBV_SRV_DESCRIPTOR_TABLE
				};

			private:

				virtual ErrorCode CreateRootSignature() override;
				virtual ErrorCode CreatePipelineState() override;

			public:

				inline void Apply
				(
					RGrpCommandList * CmdList
				)	const
				{
					CmdList->ApplyPipelineState(PipelineState);
				}
			};

			class ResolvePSPipelineMSAA : public CSingleton<ResolvePSPipelineMSAA>, public PipelineObject<ResolvePSPipelineMSAA>
			{
			public:

				enum ERootParameters
				{
					PARAMETER_CBV_SRV_DESCRIPTOR_TABLE
				};

			private:

				virtual ErrorCode CreateRootSignature() override;
				virtual ErrorCode CreatePipelineState() override;

			public:

				inline void Apply
				(
					RGrpCommandList * CmdList
				)	const
				{
					CmdList->ApplyPipelineState(PipelineState);
				}
			};

			class TemporalFilter : public CSingleton<TemporalFilter>, public PipelineObject<TemporalFilter>
			{
			public:

				enum ERootParameters
				{
					PARAMETER_CBV_SRV_DESCRIPTOR_TABLE,
					PARAMETER_SRV_FRAME_DESCRIPTOR_TABLE,
					PARAMETER_SRV_DEPTH_DESCRIPTOR_TABLE
				};

			private:

				virtual ErrorCode CreateRootSignature() override;
				virtual ErrorCode CreatePipelineState() override;

			public:

				inline void Apply
				(
					RGrpCommandList * CmdList
				)	const
				{
					CmdList->ApplyPipelineState(PipelineState);
				}
			};

			class ApplyLighting : public CSingleton<ApplyLighting>, public PipelineObject<ApplyLighting>
			{
			public:

				enum ERootParameters
				{
					PARAMETER_CBV_SRV_DESCRIPTOR_TABLE,
					PARAMETER_SRV_FRAME_DESCRIPTOR_TABLE,
					PARAMETER_SRV_DEPTH_DESCRIPTOR_TABLE
				};

			private:

				virtual ErrorCode CreateRootSignature() override;
				virtual ErrorCode CreatePipelineState() override;

			public:

				inline void Apply
				(
					RGrpCommandList * CmdList
				)	const
				{
					CmdList->ApplyPipelineState(PipelineState);
				}
			};

			class ApplyLightingMSAA : public CSingleton<ApplyLightingMSAA>, public PipelineObject<ApplyLightingMSAA>
			{
			public:

				enum ERootParameters
				{
					PARAMETER_CBV_SRV_DESCRIPTOR_TABLE,
					PARAMETER_SRV_FRAME_DESCRIPTOR_TABLE,
					PARAMETER_SRV_DEPTH_DESCRIPTOR_TABLE
				};

			private:

				virtual ErrorCode CreateRootSignature() override;
				virtual ErrorCode CreatePipelineState() override;

			public:

				inline void Apply
				(
					RGrpCommandList * CmdList
				)	const
				{
					CmdList->ApplyPipelineState(PipelineState);
				}
			};

			class ComputePhaseLookup : public CSingleton<ComputePhaseLookup>, public PipelineObject<ComputePhaseLookup>
			{
			public:

				enum ERootParameters
				{
					PARAMETER_CBV_SRV_DESCRIPTOR_TABLE,
					PARAMETER_SRV_FRAME_DESCRIPTOR_TABLE,
					PARAMETER_SRV_DEPTH_DESCRIPTOR_TABLE
				};

			private:

				virtual ErrorCode CreateRootSignature() override;
				virtual ErrorCode CreatePipelineState() override;

			public:

				inline void Apply
				(
					RGrpCommandList * CmdList
				)	const
				{
					CmdList->ApplyPipelineState(PipelineState);
				}
			};

			namespace Directional
			{
				enum ERootParameters
				{
					PARAMETER_CBV_SRV_DESCRIPTOR_TABLE,
					PARAMETER_SRV_DEPTH_DESCRIPTOR_TABLE
				};

				enum EPassModes
				{
					GEOMETRY,
					SKY,
					FINAL
				};

				static inline constexpr const char * GetPassMode(const EPassModes M)
				{
					return M == GEOMETRY ? "PASSMODE_GEOMETRY" : M == SKY ? "PASSMODE_SKY" : "PASSMODE_FINAL";
				}

				enum ELightModes
				{
					DIRECTIONAL,
					SPOT,
					OMNI
				};

				static inline constexpr const char * GetLightMode(const ELightModes M)
				{
					return M == DIRECTIONAL ? "LIGHTMODE_DIRECTIONAL" : M == SPOT ? "LIGHTMODE_SPOTLIGHT" : "LIGHTMODE_OMNI";
				}

				enum EFallOffModes
				{
					NONE,
					FIXED,
					CUSTOM
				};

				static inline constexpr const char * GetFallOffMode(const EFallOffModes M)
				{
					return M == NONE ? "FALLOFFMODE_NONE" : M == FIXED ? "FALLOFFMODE_FIXED" : "FALLOFFMODE_CUSTOM";
				}

				enum EMeshModes
				{
					GRID,
					BASE,
					CAP,
					VOLUME
				};

				static inline constexpr const char * GetMeshMode(const EMeshModes M)
				{
					return M == GRID ? "MESHMODE_FRUSTUM_GRID" : M == BASE ? "MESHMODE_FRUSTUM_BASE" : M == CAP ? "MESHMODE_FRUSTUM_CAP" : "MESHMODE_OMNI_VOLUME";
				}

				enum EVolumeModes
				{
					FRUSTUM,
					PARABOLOID
				};

				static inline constexpr const char * GetVolumeMode(const EVolumeModes M)
				{
					return M == FRUSTUM ? "VOLUMETYPE_FRUSTUM" : "VOLUMETYPE_PARABOLOID";
				}

				class PipelineSetup : public PipelineObject<PipelineSetup>
				{
				private:

					virtual ErrorCode CreateRootSignature() override;

				protected:

					D3D_SHADER_MACRO PSMacros[4] = {};
					D3D_SHADER_MACRO VSMacros[2] = {};
					D3D_SHADER_MACRO HSMacros[5] = {};
					D3D_SHADER_MACRO DSMacros[4] = {};

				protected:

					void InitializeMacros
					(
						const EPassModes	Pass,
						const ELightModes	Light,
						const EFallOffModes	FallOff,
						const EMeshModes	Mesh,
						const EVolumeModes	Volume
					);

					ErrorCode InitializePipelineStateGeometryBase();
					ErrorCode InitializePipelineStateGeometryGrid();
					ErrorCode InitializePipelineStateFinal();
					ErrorCode InitializePipelineStateSky();

				public:

					inline void Apply
					(
						RGrpCommandList * CmdList
					)	const
					{
						CmdList->ApplyPipelineState(PipelineState);
					}
				};

				class PipelineSky : public CSingleton<PipelineSky>, public PipelineSetup
				{
				private:
					virtual ErrorCode CreatePipelineState() override;

				public:
					PipelineSky()
					{
						InitializeMacros(EPassModes::SKY, ELightModes::DIRECTIONAL, EFallOffModes::FIXED, EMeshModes::BASE, EVolumeModes::FRUSTUM);
					}
				};

				class PipelineGeometryFrustumGrid : public CSingleton<PipelineGeometryFrustumGrid>, public PipelineSetup
				{
				private:
					virtual ErrorCode CreatePipelineState() override;

				public:
					PipelineGeometryFrustumGrid()
					{
						InitializeMacros(EPassModes::GEOMETRY, ELightModes::DIRECTIONAL, EFallOffModes::FIXED, EMeshModes::GRID, EVolumeModes::FRUSTUM);
					}
				};

				class PipelineGeometryFrustumBase : public CSingleton<PipelineGeometryFrustumBase>, public PipelineSetup
				{
				private:
					virtual ErrorCode CreatePipelineState() override;

				public:
					PipelineGeometryFrustumBase()
					{
						InitializeMacros(EPassModes::GEOMETRY, ELightModes::DIRECTIONAL, EFallOffModes::FIXED, EMeshModes::BASE, EVolumeModes::FRUSTUM);
					}
				};

				class PipelineFinal : public CSingleton<PipelineFinal>, public PipelineSetup
				{
				private:
					virtual ErrorCode CreatePipelineState() override;

				public:
					PipelineFinal()
					{
						InitializeMacros(EPassModes::FINAL, ELightModes::DIRECTIONAL, EFallOffModes::NONE, EMeshModes::VOLUME, EVolumeModes::FRUSTUM);
					}
				};
			}
		}

		extern ErrorCode InitializePipelines();
	}
}