#pragma once

#include "DirectX/D3D.h"

namespace D3D
{
	class IParallelProcessingUnit
	{
	public:
		virtual void Process
		(
			int32_t N
		) = 0;

		virtual int32_t GetPriority() const = 0;
		virtual int32_t GetIterationCount() const = 0;

		static int32_t IsLessRelevant
		(
			const IParallelProcessingUnit* Lhs,
			const IParallelProcessingUnit* Rhs
		)
		{
			int32_t Plhs = Lhs->GetPriority();
			int32_t Prhs = Rhs->GetPriority();

			return Plhs == Prhs ? 0 : Plhs < Prhs ? 0 : 1;
		}

		static int32_t IsMoreRelevant
		(
			const IParallelProcessingUnit* Lhs,
			const IParallelProcessingUnit* Rhs
		)
		{
			int32_t Plhs = Lhs->GetPriority();
			int32_t Prhs = Rhs->GetPriority();

			return Plhs == Prhs ? 0 : Plhs > Prhs ? 0 : 1;
		}
	};
}