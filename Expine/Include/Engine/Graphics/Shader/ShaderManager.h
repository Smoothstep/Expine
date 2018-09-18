#pragma once

#include <DirectX/D3D.h>

namespace D3D
{
	class _EX_ CShaderManager : public CSingleton<CShaderManager>
	{
		friend class RShader;
		friend class CGrpShader;
		friend class CFileNotifyCallback;

	private:

		TMutex								Mutex;
		WString								ShaderDefaultDirectory;
		TVector<SharedPointer<RShader> >	Shaders;

	public:

		CShaderManager();
		CShaderManager
		(
			const	WString & Directory
		);

		~CShaderManager();

		bool GetShader
		(
			const	WString					& Path,
			const	String					& EntryPoint,
			const	TVector<ShaderMacro>	& Macros,
					SharedPointer<RShader>	& Result
		)	const;

		bool GetShaderByPathEntrypoint
		(
			const	WString					& Path,
			const	String					& EntryPoint,
					SharedPointer<RShader>	& Result
		)	const;

		bool GetShaderByPath
		(
			const	WString					& Path,
					SharedPointer<RShader>	& Result
		)	const;

		void Update();
		ErrorCode EnableDevelopmentMode();

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