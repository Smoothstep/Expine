#include "Precompiled.h"

#include "Raw/RawShader.h"
#include "Shader/ShaderManager.h"

#include "Utils/File/FileSystemWatcher.h"

#ifndef MAX_SHADERS
static constexpr UINT MaxShaders = 512;
#else
static constexpr UINT MaxShaders = MAX_SHADERS;
#endif

namespace D3D
{
	static UniquePointer<File::CFileSystemWatcher> g_pShaderChangeWatcher;

	class CFileNotifyCallback : public File::CFileSystemWatcher::IChangeNotifyCallback
	{
		virtual bool OnFileChangeNotify(File::CFileSystemWatcher::TFileNotifyInformation * pChanges) override
		{
			do
			{
				if (CShaderManager::Instance().OnUpdateShader(pChanges->GetFileName()))
				{
					return true;
				}
			} while (pChanges = pChanges->Next());

			return true;
		}
	};

	ErrorCode CShaderManager::OnUpdateShader(const WString & Path) const
	{
		SharedPointer<RShader> Shader;

		if (!GetShaderByPath(Path, Shader))
		{
			return S_OK;
		}

		ErrorCode Error;

		if ((Error = Shader->ReCompileShader()))
		{
			return Error;
		}

		return S_OK;
	}

	ErrorCode CShaderManager::EnableDevelopmentMode()
	{
		static CFileNotifyCallback ShaderChangeCallback;

		if (g_pShaderChangeWatcher == NULL)
		{
			g_pShaderChangeWatcher = new File::CFileSystemWatcher(MaxShaders);
		}

		ErrorCode Error = g_pShaderChangeWatcher->Initialize(ShaderDefaultDirectory.c_str());

		if (Error)
		{
			return Error;
		}

		g_pShaderChangeWatcher->SetCallback(&ShaderChangeCallback);

		return S_OK;
	}

	bool CShaderManager::GetShader
	(
		const	WString					& Path,
		const	String					& EntryPoint,
		const	TVector<ShaderMacro>	& Macros,
				SharedPointer<RShader>	& Result
	)	const
	{
		for (auto & Shader : Shaders)
		{
			bool Equal = true;

			if (Shader->GetEntryPoint() == EntryPoint && Shader->GetPath() == Path)
			{
				if (!Macros.empty())
				{
					if (Macros.size() != Shader->Macros.size())
					{
						continue;
					}

					for (UINT N = 0; N < Shader->Macros.size() - 1; ++N)
					{
						if (strcmp(Macros[N].Definition, Shader->Macros[N].Definition))
						{
							Equal = false;
							break;
						}

						if (strcmp(Macros[N].Name, Shader->Macros[N].Name))
						{
							Equal = false;
							break;
						}
					}

					if (!Equal)
					{
						continue;
					}
				}

				Result = Shader;
				return true;
			}
		}

		return false;
	}

	bool CShaderManager::GetShaderByPathEntrypoint
	(
		const	WString					& Path,
		const	String					& EntryPoint,
				SharedPointer<RShader>	& Result
	)	const
	{
		for (auto & Shader : Shaders)
		{
			if (Shader->GetEntryPoint() == EntryPoint && Shader->GetPath() == Path)
			{
				Result = Shader;
				return true;
			}
		}

		return false;
	}

	inline bool CShaderManager::GetShaderByPath
	(
		const	WString					& Path,
				SharedPointer<RShader>	& Result
	)	const
	{
		for (auto & Shader : Shaders)
		{
			if (Shader->GetPath() == Path)
			{
				Result = Shader;
				return true;
			}
		}

		return false;
	}

	CShaderManager::CShaderManager() : 
		ShaderDefaultDirectory(L"Shader")
	{}

	CShaderManager::~CShaderManager()
	{
		g_pShaderChangeWatcher.SafeRelease();
	}

	CShaderManager::CShaderManager(const WString & Directory) : 
		ShaderDefaultDirectory(Directory) 
	{}

	void CShaderManager::Update()
	{
		if (g_pShaderChangeWatcher)
		{
			g_pShaderChangeWatcher->Watch();
		}
	}

	void CShaderManager::OnRemoveObject(const PipelineObjectBase * Object)
	{
		if (Object)
		{
			std::scoped_lock<TMutex> Lock(Mutex);

			for (UINT N = 0; N < Shaders.size();)
			{
				RShader * pShader = Shaders[N].Get();

				if (pShader->RemoveObject(Object))
				{
					if (pShader->ConnectedObjectCount() == 0)
					{
						Shaders.erase(Shaders.begin() + N);
					}
				}
				else
				{
					++N;
				}
			}
		}
	}

	ErrorCode CShaderManager::InitializeShader
	(
		const	PipelineObjectBase 		* Object,
		const	InitializeOptions		& Options,
				SharedPointer<RShader>	& Shader
	)
	{
		std::scoped_lock<TMutex> Lock(Mutex);

		TVector<ShaderMacro> MacroList;

		if(Options.Macros)
		{
			UINT N;

			for (N = 0; Options.Macros[N].Definition; ++N)
			{
				MacroList.push_back(Options.Macros[N]);
			}

			MacroList.push_back(Options.Macros[N]);

			if (MacroList.size() == 1)
			{
				return E_INVALIDARG;
			}

			if (MacroList[MacroList.size() - 1].Name		!= NULL ||
				MacroList[MacroList.size() - 1].Definition	!= NULL)
			{
				return E_INVALIDARG;
			}
		}

		if (GetShader(Options.Path, Options.EntryPoint, MacroList, Shader))
		{
			return S_OK;
		}

		Shader = new RShader();

		ErrorCode Error = Shader->CompileShader
		(
			ShaderDefaultDirectory,
			Options.Path,
			Options.EntryPoint,
			Options.TargetVersion,
			MacroList,
			Options.IncludeHandler.Get()
		);

		if (Error)
		{
			CErrorLog::Log<LogError>() << "Error on Shader compilation: ";

			if (Shader->GetError())
			{
				CErrorLog::Log<LogError>() << String
				(
					static_cast<char*>(Shader->GetError()->GetBufferPointer()),
					static_cast<char*>(Shader->GetError()->GetBufferPointer()) + Shader->GetError()->GetBufferSize()
				);
			}
			
			CErrorLog::Log<LogError>() << CErrorLog::EndLine << "ErrorCode: " << Error << CErrorLog::EndLine;

			return Error;
		}

		Shaders.push_back(Shader);
		{
			Shader->AddObject(Object);
		}

		return S_OK;
	}

	CShaderManager GShaderManager;
}
