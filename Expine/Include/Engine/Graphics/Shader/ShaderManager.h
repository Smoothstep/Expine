#pragma once

#include "RawShader.h"

#include <boost/thread/mutex.hpp>

namespace D3D
{
	class CShaderManager : public CSingleton<CShaderManager>
	{
		friend class RShader;
		friend class CGrpShader;
		friend class CFileNotifyCallback;

	private:

		boost::mutex Mutex;

	private:

		WString ShaderDefaultDirectory;

	private:

		TVector<SharedPointer<RShader> > Shaders;

	public:

		ErrorCode EnableDevelopmentMode();

	public:

		CShaderManager();
		CShaderManager
		(
			const	WString & Directory
		);

		~CShaderManager();

		inline bool GetShader
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

		inline bool GetShaderByPathEntrypoint
		(
			const	WString					& Path,
			const	String					& EntryPoint,
					SharedPointer<RShader>	& Result
		)	const
		{
			for(auto & Shader : Shaders)
			{
				if (Shader->GetEntryPoint() == EntryPoint && Shader->GetPath() == Path)
				{
					Result = Shader;
					return true;
				}
			}

			return false;
		}

		inline bool GetShaderByPath
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

	public:

		void Update();

		struct InitializeOptions
		{
			const WString		& Path;
			const String		& EntryPoint;
			const String		& TargetVersion;
			const ShaderMacro	* Macros;

			SharedPointer<ShaderInclude> IncludeHandler;

			inline explicit InitializeOptions
			(
				const WString & Path,
				const String  & EntryPoint,
				const String  & TargetVersion
			) :
				Path(Path),
				EntryPoint(EntryPoint),
				TargetVersion(TargetVersion),
				IncludeHandler(NULL),
				Macros(NULL)
			{}

			inline explicit InitializeOptions
			(
				const	WString						& Path,
				const	String						& EntryPoint,
				const	String						& TargetVersion,
				const	SharedPointer<ShaderInclude>  IncludeHandler
			) :
				Path(Path),
				EntryPoint(EntryPoint),
				TargetVersion(TargetVersion),
				IncludeHandler(IncludeHandler),
				Macros(NULL)
			{}

			inline explicit InitializeOptions
			(
				const	WString						& Path,
				const	String						& EntryPoint,
				const	String						& TargetVersion,
				const	SharedPointer<ShaderInclude>  IncludeHandler,
				const	ShaderMacro					* pMacros
			) :
				Path(Path),
				EntryPoint(EntryPoint),
				TargetVersion(TargetVersion),
				IncludeHandler(IncludeHandler),
				Macros(pMacros)
			{}
		};

	protected:

		void OnRemoveObject
		(
			const PipelineObjectBase * Object
		);

		ErrorCode OnUpdateShader
		(
			const WString & Path
		)	const;

		ErrorCode InitializeShader
		(
			const	PipelineObjectBase 		* Object,
			const	InitializeOptions		& Options,
					SharedPointer<RShader>	& Shader
		);
	};

	extern CShaderManager GShaderManager;
}