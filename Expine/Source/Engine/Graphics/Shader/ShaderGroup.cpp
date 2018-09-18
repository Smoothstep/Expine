#include "Precompiled.h"
#include "Raw/RawShader.h"
#include "Shader/ShaderGroup.h"

namespace D3D
{
	CGrpShader::CGrpShader(const PipelineObjectBase * pObjectOwner)
	{
		OwnerObject = pObjectOwner;
	}

	CGrpShader::~CGrpShader()
	{
		CShaderManager::Instance().OnRemoveObject(OwnerObject);
	}

	ErrorCode CGrpShader::CompileShaderType(EShaderType Type, const WString & Path, const String & EntryPoint, const String & Target, ShaderInclude * Includes, const ShaderMacro * Macros)
	{
		if (OwnerObject == NULL)
		{
			return E_FAIL;
		}

		if (Type >= NumShaderTypes)
		{
			return E_INVALIDARG;
		}

		ErrorCode Result = CShaderManager::Instance().InitializeShader
		(
			OwnerObject,
			CShaderManager::InitializeOptions
			(
				Path, 
				EntryPoint, 
				Target, 
				Includes, 
				Macros
			),
			Shaders[Type]
		);

		return Result;
	}

	ErrorCode CGrpShader::CompileShaderGroupVariation(TArray<ShaderMacro*, NumShaderTypes> Macros, SharedPointer<CGrpShader> ResultShader) const
	{
		if (OwnerObject == NULL)
		{
			return E_FAIL;
		}

		ResultShader = new CGrpShader(OwnerObject);

		for (UINT N = 0; N < NumShaderTypes; ++N)
		{
			if (Macros[N])
			{
				RShader * Shader = Shaders[N].Get();

				ErrorCode ErrorCode = CShaderManager::Instance().InitializeShader
				(
					OwnerObject,
					CShaderManager::InitializeOptions
					(
						Shader->GetPath(),
						Shader->GetEntryPoint(),
						Shader->GetTarget(),
						Shader->GetIncludeHandler(),
						Macros[N]
					),
					ResultShader->Shaders[N]
				);

				if (ErrorCode)
				{
					return ErrorCode;
				}
			}
		}

		return S_OK;
	}
}
