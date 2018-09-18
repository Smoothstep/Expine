#include "Precompiled.h"

#include "Pipeline/PipelineObject.h"
#include "Buffer/BufferConstant.h"
#include "Raw/RawCommandSignature.h"
#include "Raw/RawRootSignature.h"
#include "Raw/RawPipelineState.h"

namespace D3D
{
	ErrorCode PipelineObjectBase::CreateRootSignature()
	{
		return E_NOTIMPL;
	}

	ErrorCode PipelineObjectBase::CreatePipelineState()
	{
		return E_NOTIMPL;
	}

	ErrorCode PipelineObjectBase::Initialize()
	{
		ErrorCode Error;

		if (RootSignature == NULL)
		{
			if ((Error = CreateRootSignature()))
			{
				return Error;
			}
		}

		if ((Error = CreatePipelineState()))
		{
			return Error;
		}

		return S_OK;
	}

	ErrorCode PipelineObjectBase::CreateVariation(TArray<ShaderMacro *, 6> Macros, PipelineObjectBase * Result)
	{
		ErrorCode Error;

		if (RootSignature == NULL)
		{
			if ((Error = CreateRootSignature()))
			{
				return Error;
			}
		}

		SharedPointer<CGrpShader> ShaderGroup;

		if ((Error = CreatePipelineState()))
		{
			return Error;
		}

		const CGrpShader * Shaders = PipelineState->GetShaderGroup();

		if ((Error = Shaders->CompileShaderGroupVariation(Macros, ShaderGroup)))
		{
			return Error;
		}

		Result->RootSignature		= RootSignature;
		Result->ConstantBuffer		= ConstantBuffer;
		Result->CommandSignature		= CommandSignature;
		Result->PipelineState		= PipelineState.Copy();

		if ((Error = Result->PipelineState->ReCreate(ShaderGroup)))
		{
			return Error;
		}

		return S_OK;
	}

	template<class Child>
	void PSOPermutations<Child>::CreateShaderDefinitionValueMap
	(
		const THashMap<String, THashSet<String> > & NameValueMap
	)
	{
		for (const auto & NameValue : NameValueMap)
		{
			DefinitionValueMap.AddDefinition(NameValue.first, NameValue.second);
		}
	}

	template<class Child>
	PSOPermutations<Child>::PSOPermutations
	(
		PipelineObject<Child> * DefaultObject
	)
	{
		Default = DefaultObject;
	}

	template<class Child>
	ErrorCode PSOPermutations<Child>::AddPermutation
	(
		TArray<ShaderMacro*, 6>	Macros
	)
	{
		SharedPointer<PipelineObject<Child> > PipelineObject = NewPipelineObject<Child>();
		size_t Hash;
		std::hash<LPCSTR> Hasher;

		for (ShaderMacro * Macro : Macros)
		{
			for (Hash = 0; Macro->Definition && Macro->Name; Macro->Definition++)
			{
				Hash ^= Hasher(Macro->Definition) ^ Hasher(Macro->Name);
			}

			if (Permutations.Find(Hash))
			{
				continue;
			}

			ErrorCode Error = Default->CreateVariation(Macros, PipelineObject.Get());

			if (Error)
			{
				return Error;
			}

			Permutations.insert_or_assign(Hash, PipelineObject);
		}

		return S_OK;
	}

	template<class Child>
	PipelineObject<Child> * PSOPermutations<Child>::GetPipeline
	(
		const ShaderDefinitionValueMap & Definitions
	)	const
	{
		const SharedPointer<PipelineObject<Child> > * Object = Permutations.Find(Definitions.GetHash());

		if (Object)
		{
			return Object->Get();
		}

		return nullptr;
	}
}
