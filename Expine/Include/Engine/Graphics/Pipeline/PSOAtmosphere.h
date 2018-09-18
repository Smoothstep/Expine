#pragma once

#include "Raw/RawCommandList.h"
#include "Raw/RawPipelineState.h"
#include "Buffer/BufferConstant.h"

namespace D3D
{
	namespace Pipelines
	{
		namespace AtmosphericScattering
		{
			enum EConstants
			{
				CBV_Atmosphere,
				NumCBV
			};

			enum EAtmosphereTextureOffsets
			{
				SRV_Transmittance = 0,
				SRV_Irradiance,
				SRV_Inscatter,
				SRV_DeltaE,
				SRV_DeltaSR,
				SRV_DeltaSM,
				SRV_DeltaJ,
				NumSRV
			};

			enum EPipelines
			{
				eTransmittancePS = 0,
				eIrradiancePS,
				eIrradianceNPS,
				eIrradianceCopyPS,
				eInscatterPS,
				eInscatterCopyPS,
				eInscatterCopyNPS,
				eInscatterSPS,
				eInscatterNPS,
				eInscatterCopyFPS,
				eInscatterCopyFBackPS,
				eNumPipelines
			};

			class PipelineTransmittance : public CSingleton<PipelineTransmittance>, public PipelineObject<PipelineTransmittance>
			{
			private:

				virtual ErrorCode CreateRootSignature() override;
				virtual ErrorCode CreatePipelineState() override;

			public:

				virtual inline void Apply
				(
					RGrpCommandList * CmdList
				)	const override
				{
					CmdList->ApplyPipelineState(PipelineState);
				}
			};

			class PipelineIrradiance : public CSingleton<PipelineIrradiance>, public PipelineObject<PipelineIrradiance>
			{
			private:

				virtual ErrorCode CreateRootSignature() override;
				virtual ErrorCode CreatePipelineState() override;

			public:

				virtual inline void Apply
				(
					RGrpCommandList * CmdList
				)	const override
				{
					CmdList->ApplyPipelineState(PipelineState);
				}
			};

			class PipelineIrradianceClear : public CSingleton<PipelineIrradianceClear>, public PipelineObject<PipelineIrradianceClear>
			{
			private:

				virtual ErrorCode CreateRootSignature() override;
				virtual ErrorCode CreatePipelineState() override;
			};

			class PipelineIrradianceN : public CSingleton<PipelineIrradianceN>, public PipelineObject<PipelineIrradianceN>
			{
			private:

				virtual ErrorCode CreateRootSignature() override;
				virtual ErrorCode CreatePipelineState() override;

			public:

				virtual inline void Apply
				(
					RGrpCommandList * CmdList
				)	const override
				{
					CmdList->ApplyPipelineState(PipelineState);
				}
			};

			class PipelineIrradianceCopy : public CSingleton<PipelineIrradianceCopy>, public PipelineObject<PipelineIrradianceCopy>
			{
			private:

				virtual ErrorCode CreateRootSignature() override;
				virtual ErrorCode CreatePipelineState() override;

			public:

				virtual inline void Apply
				(
					RGrpCommandList * CmdList
				)	const override
				{
					CmdList->ApplyPipelineState(PipelineState);
				}
			};

			class PipelineInscatter : public CSingleton<PipelineInscatter>, public PipelineObject<PipelineInscatter>
			{
			private:

				virtual ErrorCode CreateRootSignature() override;
				virtual ErrorCode CreatePipelineState() override;

			public:

				virtual inline void Apply
				(
					RGrpCommandList * CmdList
				)	const override
				{
					CmdList->ApplyPipelineState(PipelineState);
				}
			};

			class PipelineInscatterCopy : public CSingleton<PipelineInscatterCopy>, public PipelineObject<PipelineInscatterCopy>
			{
			private:

				virtual ErrorCode CreateRootSignature() override;
				virtual ErrorCode CreatePipelineState() override;

			public:

				virtual inline void Apply
				(
					RGrpCommandList * CmdList
				)	const override
				{
					CmdList->ApplyPipelineState(PipelineState);
				}
			};

			class PipelineInscatterCopyN : public CSingleton<PipelineInscatterCopyN>, public PipelineObject<PipelineInscatterCopyN>
			{
			private:

				virtual ErrorCode CreateRootSignature() override;
				virtual ErrorCode CreatePipelineState() override;

			public:

				virtual inline void Apply
				(
					RGrpCommandList * CmdList
				)	const override
				{
					CmdList->ApplyPipelineState(PipelineState);
				}
			};

			class PipelineInscatterS : public CSingleton<PipelineInscatterS>, public PipelineObject<PipelineInscatterS>
			{
			private:

				virtual ErrorCode CreateRootSignature() override;
				virtual ErrorCode CreatePipelineState() override;

			public:

				virtual inline void Apply
				(
					RGrpCommandList * CmdList
				)	const override
				{
					CmdList->ApplyPipelineState(PipelineState);
				}
			};

			class PipelineInscatterN : public CSingleton<PipelineInscatterN>, public PipelineObject<PipelineInscatterN>
			{
			private:

				virtual ErrorCode CreateRootSignature() override;
				virtual ErrorCode CreatePipelineState() override;

			public:

				virtual inline void Apply
				(
					RGrpCommandList * CmdList
				)	const override
				{
					CmdList->ApplyPipelineState(PipelineState);
				}
			};

			class PipelineInscatterCopyF : public CSingleton<PipelineInscatterCopyF>, public PipelineObject<PipelineInscatterCopyF>
			{
			private:

				virtual ErrorCode CreateRootSignature() override;
				virtual ErrorCode CreatePipelineState() override;

			public:

				virtual inline void Apply
				(
					RGrpCommandList * CmdList
				)	const override
				{
					CmdList->ApplyPipelineState(PipelineState);
				}
			};

			class PipelineInscatterCopyFBack :public CSingleton<PipelineInscatterCopyFBack>, public PipelineObject<PipelineInscatterCopyFBack>
			{
			private:

				virtual ErrorCode CreateRootSignature() override;
				virtual ErrorCode CreatePipelineState() override;

			public:

				virtual inline void Apply
				(
					RGrpCommandList * CmdList
				)	const override
				{
					CmdList->ApplyPipelineState(PipelineState);
				}
			};

			class PipelineFog : public CSingleton<PipelineFog>, public PipelineObject<PipelineFog>
			{
			private:

				virtual ErrorCode CreateRootSignature() override;
				virtual ErrorCode CreatePipelineState() override;

			public:

				virtual inline void Apply
				(
					RGrpCommandList * CmdList
				)	const override
				{
					CmdList->ApplyPipelineState(PipelineState);
				}
			};

			extern ErrorCode InitializePipelines();
		}
	}
}