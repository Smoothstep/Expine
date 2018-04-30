#pragma once

#include <Smart.h>
#include "D3D.h"

namespace D3D
{
	class RGrpCommandList;
	class RPipelineState;
	class RRootSignature;
	class RCommandSignature;
	class CConstantBuffer;

	struct ErrorCode;

	class PipelineObjectBase
	{
	public:

		virtual ErrorCode CreateRootSignature();
		virtual ErrorCode CreatePipelineState();

	public:

		virtual ErrorCode Initialize();

	protected:

		SharedPointer<RPipelineState>		PipelineState;
		SharedPointer<RRootSignature>		RootSignature;
		SharedPointer<RCommandSignature>	CommandSignature;
		SharedPointer<CConstantBuffer>		ConstantBuffer;

	public:

		inline const SharedPointer<RPipelineState>		& GetPipeline()			const	{ return PipelineState;		}
		inline const SharedPointer<RRootSignature>		& GetRootSignature()	const	{ return RootSignature;		}
		inline const SharedPointer<RCommandSignature>	& GetCommandSignature() const	{ return CommandSignature;	}
		inline const SharedPointer<CConstantBuffer>		& GetConstantBuffer()	const	{ return ConstantBuffer;	}

	public:

		virtual inline void Apply
		(
			RGrpCommandList * CmdList
		)	const PURE;

		virtual inline void SetConstantBuffer
		(
			const RGrpCommandList & CmdList,
			const CConstantBuffer & ConstantBuffer
		)	const
		{}

		virtual ErrorCode CreateVariation
		(
			TArray<ShaderMacro*, 6>	Macros,
			PipelineObjectBase	*	Result
		);
	};

	template
	<
		class Child
	>
	class PipelineObject : public PipelineObjectBase
	{};

	template
	<
		class Child
	>
	PipelineObject<Child> * NewPipelineObject()
	{
		return new PipelineObject<Child>();
	}

	class ShaderDefinition
	{
	private:

		TSet<UINT32>	HashValues;
		UINT32			Hash;
		UINT32			HashName;
		UINT32			HashDefinition;
		String			NameDefinition;

	public:

		inline UINT32 GetHash() const
		{
			return Hash ^ HashName;
		}

	public:

		ShaderDefinition
		(
			const String			& Name,
			const THashSet<String>	& Values
		) :
			NameDefinition(Name)
		{
			for each(const String & Val in Values)
			{
				HashValues.emplace(std::hash<String>()(Val));
			}

			HashName = std::hash<String>()(Name);
		}

		inline void operator=(const UINT Index)
		{
			Hash = *HashValues.find(Index);
		}
	};

	class ShaderDefinitionValueMap
	{
	private:

		TVector<ShaderDefinition> Definitions;

	public:

		inline void AddDefinition
		(
			const String			& Name,
			const THashSet<String>	& Values
		)
		{
			Definitions.emplace_back(Name, Values);
		}

		inline UINT32 GetHash()
		{
			UINT32 Hash = 0;

			for each(const ShaderDefinition & Definition in Definitions)
			{
				Hash ^= Definition.GetHash();
			}

			return Hash;
		}
	};

	template
	<
		class Child
	>
	class PipelineObjectVariations : public CSingleton<PipelineObjectVariations<Child> >
	{
	protected:

		SharedPointer<PipelineObject<Child> >					Default;
		TMap<UINT32, SharedPointer<PipelineObject<Child> > >	Permutations;
		ShaderDefinitionValueMap								DefinitionValueMap;

	public:

		inline ShaderDefinitionValueMap GetDefinitionValueMap()
		{
			return DefinitionValueMap;
		}

	public:

		void CreateShaderDefinitionValueMap
		(
			const THashMap<String, THashSet<String> > & NameValueMap
		)
		{
			for (const auto & NameValue : NameValueMap)
			{
				DefinitionValueMap.AddDefinition(NameValue.first, NameValue.second);
			}
		}

		PipelineObjectVariations
		(
			PipelineObject<Child> * DefaultObject
		)
		{
			Default = DefaultObject;
		}

		ErrorCode AddPermutation
		(
			TArray<ShaderMacro*, 6>	Macros
		)
		{
			SharedPointer<PipelineObject<Child> > PipelineObject = NewPipelineObject<Child>();

			UINT32 Hash;

			for (Hash = 0; Defines->Definition && Defines->Name; Defines++)
			{
				Hash ^= std::hash<LPCSTR>(Defines->Definition) ^ std::hash<LPCSTR>(Defines->Name);
			}

			if (Permutations.Find(Hash))
			{
				return S_OK;
			}

			ErrorCode Error = Default->CreateVariation(Macros, PipelineObject.Get());

			if (Error)
			{
				return Error;
			}

			Permutations.insert_or_assign(Hash, PipelineObject);

			return S_OK;
		}

		inline PipelineObject<Child> * GetPipeline
		(
			const ShaderDefinitionValueMap & Definitions
		)	const
		{
			SharedPointer<PipelineObject<Child> > * Object = Permutations.Find(Definitions.GetHash());

			if (Object)
			{
				return Object->Get();
			}

			return nullptr;
		}
	};
}