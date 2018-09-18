#include "Precompiled.h"

#include "Raw/RawShader.h"

#include <Utils/File/File.h>
#include <d3dcompiler.h>

#include <locale>
#include <codecvt>

//#define _DEBUG_SHADER_OPTIMIZATION

#ifdef _DEBUG
#ifdef _DEBUG_SHADER_OPTIMIZATION
#define SHADER_COMPILE_FLAGS (D3DCOMPILE_OPTIMIZATION_LEVEL3)
#else
#define SHADER_COMPILE_FLAGS (D3DCOMPILE_DEBUG | D3DCOMPILE_PREFER_FLOW_CONTROL | D3DCOMPILE_SKIP_OPTIMIZATION)
#endif
#else
#define SHADER_COMPILE_FLAGS (D3DCOMPILE_OPTIMIZATION_LEVEL3)
#endif

namespace D3D
{
	class CDefaultIncludeHandler : public ID3DInclude
	{
	private:

		UniquePointer<File::CFile> File;
		WString Directory;

	public:

		CDefaultIncludeHandler(const WString & Main)
		{
			std::experimental::filesystem::path Path(Main);
			std::experimental::filesystem::path Absolute = std::experimental::filesystem::absolute(Path);

			Directory = Absolute.parent_path().wstring();
		}

		virtual HRESULT WINAPI Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes) override
		{
			const Uint Length = std::strlen(pFileName);
			
			WString FileName;
			{
				FileName.reserve(Length * 2);
			}

			MultiByteToWideChar(CP_UTF8, 0, pFileName, std::strlen(pFileName), FileName.data(), FileName.length());

			WString FullPath = Directory;
			{
				FullPath.append(L"\\").append(FileName);
			}

			File = new File::CFile(FullPath);

			if (File->ReadFileContent() != File::ErrorNone)
			{
				return ERROR_FILE_INVALID;
			}

			*ppData = File->GetContentRef().data();
			*pBytes = File->GetContentRef().size();

			return S_OK;
		}

		virtual HRESULT WINAPI Close(LPCVOID pData) override
		{
			if (File)
			{
				File.SafeRelease();
			}

			return S_OK;
		}
	};

	ErrorCode RShader::ReCompileShader()
	{
		ErrorCode EC = D3DCompileFromFile
		(
			PathFull.c_str(),
			Macros.empty() ? NULL : Macros.data(),
			IncludeHandler.Get(),
			EntryPoint.c_str(),
			Target.c_str(),
			SHADER_COMPILE_FLAGS,
			0,
			&Code,
			&Error
		);

		if (EC)
		{
			CErrorLog::Log<LogError>() << "Unable to compile shader: " << PathFull << CErrorLog::EndLine;
			
			if (Error)
			{
				CErrorLog::Log<LogError>() << String(
					static_cast<char*>(Error->GetBufferPointer()),
					static_cast<char*>(Error->GetBufferPointer()) + Error->GetBufferSize()) << CErrorLog::EndLine;
			}

			return EC;
		}

		TVector<ConstPointer<PipelineObjectBase> > Copy = ConnectedObjects;

		for (auto & Object : Copy)
		{
			if ((EC = Object->CreatePipelineState()))
			{
				return EC;
			}
		}

		return S_OK;
	}

	ErrorCode RShader::CompileShader
	(
		const	WString					& ShaderDirectory,
		const	WString					& ShaderFilename,
		const	String					& ShaderEntryPoint,
		const	String					& ShaderTarget,
		const	TVector<ShaderMacro>	& ShaderMacros,
				ID3DInclude				* ShaderIncludeHandler
	)
	{
		Path		= ShaderFilename;
		EntryPoint	= ShaderEntryPoint;
		Target		= ShaderTarget;
		PathFull	= ShaderDirectory + L"\\" + Path;
		Macros		= ShaderMacros;
		
		if (ShaderIncludeHandler == NULL)
		{
			IncludeHandler = new CDefaultIncludeHandler(PathFull);
		}
		else
		{
			IncludeHandler = ShaderIncludeHandler;
		}

		return D3DCompileFromFile
		(
			PathFull.c_str(),
			Macros.empty() ? NULL : Macros.data(),
			IncludeHandler.Get(),
			EntryPoint.c_str(),
			Target.c_str(),
			SHADER_COMPILE_FLAGS,
			0,
			&Code,
			&Error
		);
	}

	ErrorCode RShader::LoadShader(const WString & Path)
	{
		this->Path = Path;
		{
			return D3DReadFileToBlob(Path.c_str(), &Code);
		}
	}

	bool RShader::RemoveObject(const PipelineObjectBase * pObjectBase)
	{
		for (auto Iter = ConnectedObjects.begin(); Iter != ConnectedObjects.end(); ++Iter)
		{
			if (*Iter == pObjectBase)
			{
				ConnectedObjects.erase(Iter);
				{
					return true;
				}
			}
		}

		return false;
	}

	void RShader::AddObject(const PipelineObjectBase * pObjectBase)
	{
		ConnectedObjects.push_back(pObjectBase);
	}
}
