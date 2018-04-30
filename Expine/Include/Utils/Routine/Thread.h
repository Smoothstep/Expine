#pragma once

#include "Function.h"

#include "../Container/HopscotchMap.h"

class CThread
{
private:

	HANDLE ThreadHandle;

public:

	inline HRESULT SetDescription
	(
		PCWSTR Description
	)
	{
		return SetThreadDescription(ThreadHandle, Description);
	}

	inline BOOL SetPriority
	(
		const int Priority
	)
	{
		return SetThreadPriority(ThreadHandle, Priority);
	}

public:

	template
	<
		typename Routine,
		typename Parameter
	>
	bool Create
	(
		const Routine	*	pRoutine,
		const Parameter *	pParameter,
		const DWORD			dwFlags
	)
	{	
		ThreadHandle = CreateThread(NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(pRoutine), pParameter, dwFlags, NULL);

		if (IS_INVALID_HANDLE(ThreadHandle))
		{
			return false;
		}

		return true;
	}

	template
	<
		typename Routine,
		typename Parameter
	>
	bool CreateSuspended
	(
		const Routine	* pRoutine,
		const Parameter * pParameter
	)
	{	
		return Create(pRoutine, pParameter, CREATE_SUSPENDED);
	}

	~CThread()
	{
		Terminate();
	}

public:

	void Join()
	{
		WaitForSingleObject(ThreadHandle, INFINITE);
	}

	void Join
	(
		const DWORD dwMs
	)
	{
		WaitForSingleObject(ThreadHandle, dwMs);
	}

	void Terminate()
	{
		if (IS_VALID_HANDLE(ThreadHandle))
		{
			if (!TerminateThread(ThreadHandle, 0))
			{
				CloseHandle(ThreadHandle);
			}
		}

		ThreadHandle = INVALID_HANDLE_VALUE;
	}
};

class CThreadFunction : public CFunction
{
protected:

	int FunctionPriority;

public:

	CThreadFunction
	(
		int Priority = 0
	)
	{
		FunctionPriority = Priority;
	}

	inline bool operator()
	(
		const CThreadFunction & Other
	)
	{
		return FunctionPriority > Other.FunctionPriority;
	}
};