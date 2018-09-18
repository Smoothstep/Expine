#pragma once

#include "Shader/ShaderManager.h"
#include "Pipeline/PipelineObject.h"

namespace D3D
{
	class CShaderDefines
	{
	protected:

		ShaderMacro * Macros;
	};

	class CGrpShader
	{
		friend class RPipelineState;

	public:

		enum EShaderType
		{
			Pixel, 
			Vertex, 
			Geometry, 
			Hull, 
			Domain, 
			Compute, 
			NumShaderTypes
		};

	protected:

		ConstPointer<PipelineObjectBase> OwnerObject;

	private:

		TArray<SharedPointer<RShader>, NumShaderTypes> Shaders;

	public:

		inline RShader * GetShader
		(
			const EShaderType Type
		)	const
		{
			Ensure(Type < NumShaderTypes);
			return Shaders[Type].Get();
		}

		CGrpShader
		(
			const PipelineObjectBase * pObjectOwner
		);

		~CGrpShader();

	public:

		ErrorCode CompileShaderType
		(
					EShaderType				Type,
			const	WString				&	Path,
			const	String				&	EntryPoint,
			const	String				&	Target,
					ShaderInclude		*	Includes	= NULL,
			const	ShaderMacro			*	Macros		= NULL
		);

		ErrorCode CompileShaderGroupVariation
		(
			TArray<ShaderMacro*, NumShaderTypes>	Macros,
			SharedPointer<CGrpShader>				Result
		) const;
	};
}