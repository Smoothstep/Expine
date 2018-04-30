#pragma once

#include "FunctionDeque.h"

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

#include <WindowsH.h>

namespace Thread
{
	class CServiceThread
	{
	private:

		CFunctionDeque FunctionSequence;

		boost::shared_ptr<boost::thread> ExecThread;

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