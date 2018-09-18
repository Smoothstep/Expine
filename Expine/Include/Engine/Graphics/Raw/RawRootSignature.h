#pragma once

#include "DirectX/D3D.h"

namespace D3D
{
	class RRootSignature
	{
	private:

		ComPointer<IRootSignature> RootSignature;

	private:

		D3D12_ROOT_SIGNATURE_DESC SignatureDesc;

	public:

		inline IRootSignature * operator ->() const
		{
			return RootSignature.Get();
		}

		inline operator IRootSignature * () const
		{
			return RootSignature.Get();
		}

		inline IRootSignature * Get() const
		{
			return RootSignature.Get();
		}

	public:

		struct InitializeOptions : public D3D12_ROOT_SIGNATURE_DESC
		{
			inline explicit InitializeOptions() {}
			inline explicit InitializeOptions
			(
				const UINT							NumParameters,
				const D3D12_ROOT_PARAMETER		*	pRootParameter,
				const UINT							NumSamplers,
				const D3D12_STATIC_SAMPLER_DESC	*	pSamplerDescs,
				const D3D12_ROOT_SIGNATURE_FLAGS	Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
			) 
			{
				this->pParameters		= pRootParameter;
				this->NumStaticSamplers = NumSamplers;
				this->NumParameters		= NumParameters;
				this->pStaticSamplers	= pSamplerDescs;
				this->Flags				= Flags;
			}
		};

		ErrorCode Create
		(
			const InitializeOptions & Options
		);

		inline UINT GetDescriptorRangeSamplers() const
		{
			UINT TableStart = 0;
			UINT TableEnd = 0;

			for (UINT P = 0; P < SignatureDesc.NumParameters; ++P)
			{
				if (SignatureDesc.pParameters[P].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
				{
					for (UINT T = 0; T < SignatureDesc.pParameters[P].DescriptorTable.NumDescriptorRanges; ++T)
					{
						if (SignatureDesc.pParameters[P].DescriptorTable.pDescriptorRanges[T].RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
						{
							TableStart	= std::min(TableStart,	SignatureDesc.pParameters[P].DescriptorTable.pDescriptorRanges[T].OffsetInDescriptorsFromTableStart);
							TableEnd	= std::max(TableEnd,		SignatureDesc.pParameters[P].DescriptorTable.pDescriptorRanges[T].OffsetInDescriptorsFromTableStart + SignatureDesc.pParameters[P].DescriptorTable.pDescriptorRanges[T].NumDescriptors);
						}
					}
				}
			}

			return TableEnd - TableStart;
		}

		inline UINT GetDescriptorRangeViews() const
		{
			UINT TableStart = 0;
			UINT TableEnd = 0;

			for (UINT P = 0; P < SignatureDesc.NumParameters; ++P)
			{
				if (SignatureDesc.pParameters[P].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
				{
					for (UINT T = 0; T < SignatureDesc.pParameters[P].DescriptorTable.NumDescriptorRanges; ++T)
					{
						if (SignatureDesc.pParameters[P].DescriptorTable.pDescriptorRanges[T].RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_CBV ||
							SignatureDesc.pParameters[P].DescriptorTable.pDescriptorRanges[T].RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SRV || 
							SignatureDesc.pParameters[P].DescriptorTable.pDescriptorRanges[T].RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_UAV)
						{
							TableStart	= std::min(TableStart,	SignatureDesc.pParameters[P].DescriptorTable.pDescriptorRanges[T].OffsetInDescriptorsFromTableStart);
							TableEnd	= std::max(TableEnd,		SignatureDesc.pParameters[P].DescriptorTable.pDescriptorRanges[T].OffsetInDescriptorsFromTableStart + SignatureDesc.pParameters[P].DescriptorTable.pDescriptorRanges[T].NumDescriptors);
						}
					}
				}
			}

			return TableEnd - TableStart;
		}

		inline UINT GetViewCount() const
		{
			UINT Views = 0;

			for (UINT P = 0; P < SignatureDesc.NumParameters; ++P)
			{
				if (SignatureDesc.pParameters[P].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
				{
					for (UINT T = 0; T < SignatureDesc.pParameters[P].DescriptorTable.NumDescriptorRanges; ++T)
					{
						if (SignatureDesc.pParameters[P].DescriptorTable.pDescriptorRanges[T].RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_CBV || 
							SignatureDesc.pParameters[P].DescriptorTable.pDescriptorRanges[T].RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SRV || 
							SignatureDesc.pParameters[P].DescriptorTable.pDescriptorRanges[T].RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_UAV)
						{
							Views += SignatureDesc.pParameters[P].DescriptorTable.pDescriptorRanges[T].NumDescriptors;
						}
					}
				}
				else if (	SignatureDesc.pParameters[P].ParameterType == D3D12_ROOT_PARAMETER_TYPE_CBV ||
							SignatureDesc.pParameters[P].ParameterType == D3D12_ROOT_PARAMETER_TYPE_SRV ||
							SignatureDesc.pParameters[P].ParameterType == D3D12_ROOT_PARAMETER_TYPE_UAV)
				{
					Views++;
				}
			}

			return Views;
		}
	};
}