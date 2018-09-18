#pragma once

#include "Function.h"

class CFunctionDeque
{
	struct PriorityFunction
	{
		int			FunctionPriority;
		CFunction * Function;

		inline PriorityFunction
		(
			CFunction *	pFunction,
			int			Priority
		)
		{
			Function = pFunction;
			FunctionPriority = Priority;
		}

		static inline bool Compare
		(
			const PriorityFunction & L,
			const PriorityFunction & R
		)
		{
			return L.FunctionPriority > R.FunctionPriority;
		}
	};

private:

	std::vector<PriorityFunction> Functions;

public:

	inline void AddFunction
	(
		CFunction * pFunction,
		int			Priority = 1
	)
	{
		if (!pFunction)
		{
			return;
		}

		Functions.push_back(PriorityFunction(pFunction, Priority));
		{
			std::push_heap(Functions.begin(), Functions.end(), PriorityFunction::Compare);
		}
	}

	inline void ClearAll()
	{
		Functions.clear();
	}
};