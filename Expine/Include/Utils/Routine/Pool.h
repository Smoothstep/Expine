#pragma once

#include <vector>
#include <tuple>
#include <utility>

#include "WindowsH.h"
#include "Function.h"

namespace Thread
{
	class CTask
	{
	private:

		CFunction	* Function	= NULL;
		CTask		* Next		= NULL;

	public:

		CTask
		(
			CFunction * pFunction
		)
		{
			Function = pFunction;
		}

		~CTask()
		{
			SafeRelease(Function);
		}

		inline void SetNext
		(
			CTask * pNext
		)
		{
			Next = pNext;
		}

		inline CTask * GetNext()
		{
			return Next;
		}

		inline void Run()
		{
			Function->Run();
		}
	};

	class CTaskQueue
	{
	private:

		CTask * Begin;
		CTask * End;

	public:

		CTaskQueue() :
			Begin(NULL),
			End(NULL)
		{}

		~CTaskQueue()
		{
			ClearTasks();
		}

		inline void ClearTasks()
		{
			CTask * pNext = NULL;

			for (CTask * pTask = Begin; pTask = pNext; pTask)
			{
				pNext = pTask->GetNext();
				{
					delete pTask;
				}
			}

			Begin = NULL;
			End	= NULL;
		}

		inline void AddTask
		(
			CTask * pTask
		)
		{
			if (!Begin)
			{
				Begin = End = pTask;
			}
			else
			{
				End->SetNext(pTask);
				End = pTask;
			}
		}

		inline CTask * GetTask()
		{
			return Begin;
		}

		inline CTask * GetPopTask()
		{
			if (!Begin)
			{
				return NULL;
			}

			CTask * First = Begin;
			{
				PopTask();
			}

			return First;
		}

		inline void PopTask()
		{
			Begin = Begin->GetNext();
		}

		inline bool Empty()
		{
			return !Begin;
		}
	};

	class CThreadGroup : public std::vector<boost::thread*> 
	{};

	class CThreadPool
	{
	private:

		CTaskQueue		TaskQueue;
		CThreadGroup	ThreadGroup;

		long StopWork;
		long StopAfterWork;
		long NumTasks;

		HANDLE Queue;

		boost::mutex WaitMutex;
		boost::condition_variable WaitCondition;

	private:

		void ThreadMain()
		{
			CTask * Task = NULL;

			DWORD		BytesTransferred	= 0;
			ULONG_PTR	CompletionKey		= 0;

			while (Queue)
			{
				if (!GetQueuedCompletionStatus(Queue, &BytesTransferred, &CompletionKey, reinterpret_cast<LPOVERLAPPED*>(&Task), INFINITE))
				{
					continue;
				}

				if (!Task)
				{
					PostQueuedCompletionStatus(Queue, 0, 0, 0);
					return;
				}

				Task->Run();
				{
					delete Task;
				}

				if (InterlockedDecrement(&NumTasks) == 0)
				{
					if (InterlockedCompareExchange(&StopAfterWork, 0, 0) == 1)
					{
						PostQueuedCompletionStatus(Queue, 0, 0, 0);
						return;
					}
					else
					{
						Task = TaskQueue.GetPopTask();

						if (Task)
						{
							PostQueuedCompletionStatus(Queue, 0, 0, reinterpret_cast<LPOVERLAPPED>(Task));
						}
						else
						{
							WaitMutex.lock();
							WaitCondition.notify_all();
							WaitMutex.unlock();
						}

						continue;
					}
				}
			}
		}

	public:

		CThreadPool
		(
			size_t iCount
		) :
			StopWork(0),
			StopAfterWork(0),
			Queue(NULL),
			NumTasks(0)
		{
			SetupThreads(iCount);
		}

		CThreadPool() :
			StopWork(0),
			StopAfterWork(0),
			Queue(NULL),
			NumTasks(0) 
		{}

		~CThreadPool()
		{
			ShutdownThreads();

			if (Queue)
			{
				CloseHandle(Queue);
				Queue = NULL;
			}
		}

		bool Empty()
		{
			return TaskQueue.Empty();
		}

		bool Initialize()
		{
			Queue = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 10);

			if (!Queue)
			{
				return false;
			}

			return true;
		}

		void ShutdownThreads()
		{
			for(auto & Thread : ThreadGroup)
			{
				Thread->interrupt();
				{
					delete Thread;
				}
			}

			ThreadGroup.clear();
		}

		bool SetupThreads
		(
			size_t Count
		)
		{
			if (Count == ThreadGroup.size())
			{
				return true;
			}

			if (!Queue)
			{
				if (!Initialize())
				{
					return false;
				}
			}

			ShutdownThreads();

			for (size_t i = 0; i < Count; ++i)
			{
				ThreadGroup.push_back(new boost::thread(boost::bind(&CThreadPool::ThreadMain, this)));
			}

			return true;
		}

		size_t ThreadCount()
		{
			return ThreadGroup.size();
		}

		void PostTask
		(
			CFunction * pFunction
		)
		{
			PostTask(new CTask(pFunction));
		}

		void PostTask
		(
			CTask * pTask
		)
		{
			if (!PostQueuedCompletionStatus(Queue, 0, 0, reinterpret_cast<LPOVERLAPPED>(pTask)))
			{
				TaskQueue.AddTask(pTask);
			}
			else
			{
				InterlockedIncrement(&NumTasks);
			}
		}

		void JoinAllWork()
		{
			InterlockedExchange(&StopAfterWork, true);

			if (InterlockedCompareExchange(&NumTasks, 0, 0) == 0)
			{
				PostQueuedCompletionStatus(Queue, 0, 0, 0);
			}

			for(auto & Thread : ThreadGroup)
			{
				Thread->join();
			}
		}

		bool HasWork()
		{
			return NumTasks || !Empty();
		}

		void JoinAll
		(
			DWORD dwMs = INFINITE
		)
		{
			boost::unique_lock<boost::mutex> Lock(WaitMutex);
			{
				WaitCondition.wait_until(Lock, boost::chrono::time_point<boost::chrono::high_resolution_clock>(boost::chrono::milliseconds(dwMs)), [this]
				{
					return !HasWork();
				});
			}
		}

		bool JoinAndInterruptAll()
		{
			if (!Cancel())
			{
				CLOSE_HANDLE(Queue);
			}

			if (!HasWork())
			{
				return Initialize();
			}

			for (auto & Thread : ThreadGroup)
			{
				Thread->join();
			}

			TaskQueue.ClearTasks();

			NumTasks = 0;

			return Initialize();
		}

	private:

		bool Cancel()
		{
			if (!CancelIo(Queue))
			{
				return false;
			}

			CLOSE_HANDLE(Queue);

			return true;
		}
	};
}