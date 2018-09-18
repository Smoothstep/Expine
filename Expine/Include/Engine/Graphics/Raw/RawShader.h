#pragma once

#include "DirectX/D3D.h"
#include "Pipeline/PipelineObject.h"

namespace D3D
{
	class RShader
	{
		friend class CShaderManager;

	protected:

		ComPointer<IBlob>				Code;
		ComPointer<IBlob>				Error;

		SharedPointer<ID3DInclude>		IncludeHandler;

		TVector<ShaderMacro>			Macros;

		WString							Path;
		WString							PathFull;

		String							EntryPoint;
		String							Target;

	private:

		TVector<ConstPointer<PipelineObjectBase> > ConnectedObjects;

	public:

		inline size_t ConnectedObjectCount() const
		{
			return ConnectedObjects.size();
		}

		inline PipelineObjectBase * GetConnectedObject
		(
			const size_t Index
		)	const
		{
			return ConnectedObjects[Index].Get();
		}

		inline IBlob * GetByteCode() const
		{
			return Code.Get();
		}

		inline IBlob * GetError() const
		{
			return Error.Get();
		}

		inline const WString & GetPath() const
		{
			return Path;
		}

		inline const String & GetEntryPoint() const
		{
			return EntryPoint;
		}

		inline const String & GetTarget() const
		{
			return Target;
		}

		inline SharedPointer<ID3DInclude> GetIncludeHandler() const
		{
			return IncludeHandler;
		}

	protected:

		ErrorCode ReCompileShader();
		ErrorCode CompileShader
		(
			const	WString					& Directory,
			const	WString					& Filename,
			const	String					& EntryPoint,
			const	String					& Target,
			const	TVector<ShaderMacro>	& Macros,
					ID3DInclude				* Includes = NULL
		);

		ErrorCode LoadShader
		(
			const WString & Path
		);

		bool RemoveObject
		(
			const PipelineObjectBase * pObjectBase
		);

		void AddObject
		(
			const PipelineObjectBase * pObjectBase
		);
	};
}