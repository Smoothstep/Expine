#include "Precompiled.h"

#include "Scene/Outdoor/Atmosphere.h"

#include "Scene/Scene.h"
#include "Scene/SceneRenderer.h"
#include "Scene/SceneOutdoor.h"

namespace D3D
{
	namespace Atmosphere
	{
		ErrorCode ScatteringProperties::Initialize(File::CConfig & ShaderConfig)
		{
			const StringValue<StringView> Value;

			if (ShaderConfig.GetValue("AtmosphericFogInscatterAltitudeSampleNum", &Value))
			{
				Value >> Constants.AtmosphericFogInscatterAltitudeSampleNum;
			}

			return S_OK;
		}

		ErrorCode AtmosphereProperties::Initialize(const WString & CloudConfig, const WString & ScatteringConfig)
		{
			return E_NOTIMPL;
		}

		CAtmosphere::CRenderer::CRenderer(const CSceneRenderer * pSceneRenderer)
		{
			Scene			= pSceneRenderer->GetScene();			Ensure(Scene);
			SceneRenderer	= pSceneRenderer;						Ensure(SceneRenderer);

			Update();
		}

		ErrorCode CAtmosphere::CRenderer::Create()
		{
			try
			{
				if (Atmosphere->AtmosphericScattering)
				{
					Atmosphere->AtmosphericScattering->Finalize(SceneRenderer->GetShadowMapCommandContext());
				}
			}
			catch (ECException & Exception)
			{
				return Exception.GetErrorCode();
			}

			return S_OK;
		}

		void CAtmosphere::CRenderer::Render()
		{
			if (Atmosphere->AtmosphericScattering)
			{
				Atmosphere->AtmosphericScattering->Render(SceneRenderer.Get());
			}

			if (Atmosphere->AtmosphericClouds)
			{

			}
		}

		void CAtmosphere::CRenderer::Update()
		{
			if (Scene)
			{
				Atmosphere = static_cast<const CSceneOutdoor*>(Scene->GetArea())->GetAtmosphere();	Ensure(Atmosphere);
			}
		}

		ErrorCode CAtmosphere::CRenderer::Precompute()
		{
			ErrorCode Error;

			CCommandListContext * CommandListCtx = SceneRenderer->GetShadowMapCommandContext();

			const bool CommandListOpened = CommandListCtx->IsOpened();

			UINT Frame = Scene->GetOutputScreen()->GetBackBufferIndex();

			if (Atmosphere->AtmosphericScattering)
			{
				if (!CommandListOpened)
				{
					if ((Error = CommandListCtx->ResetCommandAllocator(Frame)))
					{
						return Error;
					}

					if ((Error = CommandListCtx->ResetCommandList(Frame)))
					{
						return Error;
					}
				}

				while (true)
				{
					Atmosphere->AtmosphericScattering()->Precompute(SceneRenderer.Get());

					if ((Error = CommandListCtx->Close()))
					{
						return Error;
					}

					CommandListCtx->Finish(Frame);

					if (Atmosphere->AtmosphericScattering->Finished())
					{
						break;
					}

					if ((Error = CommandListCtx->ResetCommandAllocator(Frame)))
					{
						return Error;
					}

					if ((Error = CommandListCtx->ResetCommandList(Frame)))
					{
						return Error;
					}
				}
			}

			if (CommandListOpened)
			{
				if ((Error = CommandListCtx->ResetCommandAllocator(Frame)))
				{
					return Error;
				}

				if ((Error = CommandListCtx->ResetCommandList(Frame)))
				{
					return Error;
				}
			}

			return S_OK;
		}

		ErrorCode CAtmosphere::Create(const AtmosphereProperties & Properties)
		{
			ErrorCode Error;

			if (Properties.HasClouds)
			{
				AtmosphericClouds = new CClouds();
			}
			else
			{
				AtmosphericClouds = NULL;
			}

			if (Properties.HasScattering)
			{
				AtmosphericScattering = new CScattering();

				if ((Error = AtmosphericScattering->Initialize()))
				{
					return Error;
				}
			}
			else
			{
				AtmosphericScattering = NULL;
			}

			return S_OK;
		}

		void CClouds::Finalize(const CCommandListContext * pCommandListCtx)
		{
		}

		void CClouds::Render(const CSceneRenderer * pSceneRenderer)
		{
		}

		void CClouds::RenderShadows(const CSceneRenderer * pSceneRenderer)
		{
		}
}
}
