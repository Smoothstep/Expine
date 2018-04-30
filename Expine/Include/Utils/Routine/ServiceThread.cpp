#include "ServiceThread.h"

namespace Thread
{
	void CServiceThread::ThreadMain(CServiceThread * Owner)
	{
		DWORD dwTransferred;
		ULONG_PTR ulCompletionKey;
		LPOVERLAPPED lpOverlapped;

		while (true)
		{
			if (!GetQueuedCompletionStatus(Owner->ExecQueue, &dwTransferred, &ulCompletionKey, &lpOverlapped, INFINITE))
			{
				continue;
			}

			if (!Owner->ShallContinue)
			{
				break;
			}

			Owner->FunctionSequence.ExecuteAll();

			assert(PostQueuedCompletionStatus(Owner->WaitQueue, dwTransferred, ulCompletionKey, lpOverlapped));
		}
	}

	bool CServiceThread::Initialize()
	{
		ExecQueue = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);

		if (IS_INVALID_HANDLE(ExecQueue))
		{
			return false;
		}

		WaitQueue = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);

		if (IS_INVALID_HANDLE(WaitQueue))
		{
			return false;
		}

		ExecThread = boost::make_shared<boost::thread>(ThreadMain, this);

		return true;
	}
}