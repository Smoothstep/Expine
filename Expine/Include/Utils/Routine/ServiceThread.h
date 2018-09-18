#pragma once

#include "FunctionDeque.h"

#include <thread>

namespace Thread
{
	class CServiceThread
	{
	private:

		CFunctionDeque FunctionSequence;

		SharedPointer<std::thread> ExecThread;

		volatile bool ShallContinue = true;

		HANDLE ExecQueue = INVALID_HANDLE_VALUE;
		HANDLE WaitQueue = INVALID_HANDLE_VALUE;

	private:

		static void ThreadMain
		(
			CServiceThread * Owner
		);

	public:

		inline CFunctionDeque & GetFunctionSequence()
		{
			return FunctionSequence;
		}

		bool Initialize();

		inline void WaitForCompletion()
		{
			DWORD dwTransferred;
			ULONG_PTR ulCompletionKey;
			LPOVERLAPPED lpOverlapped;

			assert(GetQueuedCompletionStatus(WaitQueue, &dwTransferred, &ulCompletionKey, &lpOverlapped, INFINITE));
		}

		inline void Run()
		{
			assert(PostQueuedCompletionStatus(ExecQueue, 0, NULL, NULL));
		}
	};
}