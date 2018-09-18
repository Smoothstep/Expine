#pragma once

#include "Raw/RawShader.h"
#include "AtmosphericScattering.h"
#include "Hyper.h"
#include "Utils/File/Config.h"

namespace D3D
{
	namespace Atmosphere
	{
		struct CloudProperties
		{
			Uint CloudsID;

			RGBAColor Ambient;
			RGBAColor Diffuse;

			Float Scale;
			Float Coverage;

			Float Altitude;
			Float Density;
			
			Int NumSamples;
		};

		class CClouds
		{
		private:

			CloudProperties Properties;

		public:

			inline const CloudProperties & GetProperties() const
			{
				return Properties;
			}

			inline void SetProperties
			(
				const CloudProperties & Properties
			)
			{
				this->Properties = Properties;
			}

		public:

			ErrorCode Initialize()
			{
				return E_NOTIMPL;
			}

			void Finalize
			(
				const CCommandListContext * pCommandListCtx
			);

			void Render
			(
				const CSceneRenderer * pSceneRenderer
			);

			void RenderShadows
			(
				const CSceneRenderer * pSceneRenderer
			);
		};

		struct ScatteringProperties
		{
			UINT ScatteringID;

			Atmosphere::AtmosphereParameters			TextureParameters;
			Atmosphere::AtmosphereConstants				Constants;
			Atmosphere::AtmosphereInscatterConstants	ConstantsInscatter;

			ErrorCode Initialize
			(
				File::CConfig & ShaderConfig
			);
		};

		struct AtmosphereProperties
		{
			UINT AtmosphereID;

			Atmosphere::CloudProperties			ConfigClouds;
			Atmosphere::ScatteringProperties	ConfigScattering;

			BOOL HasClouds;
			BOOL HasScattering;

			ErrorCode Initialize
			(
				const WString & CloudConfig,
				const WString & ScatteringConfig
			);
		};

		class CAtmosphere
		{
		public:

			class CRenderer
			{
			private:

				ConstPointer<CScene>			Scene;
				ConstPointer<CSceneRenderer> SceneRenderer;
				ConstPointer<CAtmosphere>	Atmosphere;

			public:

				CRenderer
				(
					const CSceneRenderer * pScene
				);

				ErrorCode Create();
				ErrorCode Precompute();

			public:

				void Render();
				void Update();
			};

		protected:

			SharedPointer<RGrpCommandList> CommandList;

		protected:

			UniquePointer<CClouds>		AtmosphericClouds;
			UniquePointer<CScattering>	AtmosphericScattering;

		public:

			ErrorCode Create
			(
				const AtmosphereProperties & Properties
			);
		};
	}
}