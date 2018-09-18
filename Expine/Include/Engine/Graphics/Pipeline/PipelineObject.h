#pragma once

#include <Smart.h>
#include "DirectX/D3D.h"

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
			for (const String & Val : Values)
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

		inline size_t GetHash() const
		{
			size_t Hash = 0;

			for (const ShaderDefinition & Definition : Definitions)
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
	class PSOPermutations : public CSingleton<PSOPermutations<Child> >
	{
	protected:

		SharedPointer<PipelineObject<Child> >					Default;
		TMap<size_t, SharedPointer<PipelineObject<Child> > >	Permutations;
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
		);

		PSOPermutations
		(
			PipelineObject<Child> * DefaultObject
		);

		ErrorCode AddPermutation
		(
			TArray<ShaderMacro*, 6>	Macros
		);

		PipelineObject<Child> * GetPipeline
		(
			const ShaderDefinitionValueMap & Definitions
		)	const;
	};
}