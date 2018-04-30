#pragma once

#include "ParallelProcessingMesh.h"

#include <tbb\tbb.h>
#include <tbb\concurrent_priority_queue.h>

namespace D3D
{
	enum class RelevancyOrdering
	{
		LessRelevant
	};

	template<RelevancyOrdering Relevancy = RelevancyOrdering::LessRelevant>
	class CParallelProcessingRelevancy
	{
	public:
		template<typename = typename std::enable_if<Relevancy == RelevancyOrdering::LessRelevant> >
		inline bool operator()
		(
			const IParallelProcessingUnit * Lhs, 
			const IParallelProcessingUnit * Rhs
		)	const
		{
			return IParallelProcessingUnit::IsLessRelevant(Lhs, Rhs);
		}
	};

	class CParallelProcessor
	{
	public:
		inline void operator()
		(
			IParallelProcessingUnit * Unit
		)	const
		{
			for (int32_t N = 0; N < Unit->GetIterationCount(); ++N)
			{
				Unit->Process(N);
			}
		}
	};

	class IParallelProcess
	{
	public:
		virtual void AddProcessingUnit
		(
			IParallelProcessingUnit * Unit
		) = 0;
		virtual void RunProcessing() = 0;
	};

	template<RelevancyOrdering Ordering = RelevancyOrdering::LessRelevant>
	class CParallelProcess : public IParallelProcess
	{
	private:

		std::vector<IParallelProcessingUnit*> ProcessingUnits;
		tbb::reader_writer_lock ProcessingRWLock;

	private:

		CParallelProcessor Processor;
		CParallelProcessingRelevancy<Ordering> ProcessingRelevancy;

	private:

		void RunImpl()
		{
			ProcessingRWLock.lock_read();
			auto Queue = std::move(ProcessingUnits);
			ProcessingRWLock.unlock();
			tbb::parallel_do(Queue.begin(), Queue.end(), Processor);
		}

	public:

		virtual void AddProcessingUnit
		(
			IParallelProcessingUnit * Unit
		)	override
		{
			tbb::reader_writer_lock::scoped_lock Lock(ProcessingRWLock);
			{
				ProcessingUnits.push_back(Unit);

				std::push_heap(
					ProcessingUnits.begin(), 
					ProcessingUnits.end(), 
					ProcessingRelevancy);
			}
		}

		virtual void RunProcessing() override
		{
			RunImpl();
		}
	};

	class CParallelMeshProcess : public CParallelProcess<>
	{

	};

	class CParallelProcessPrerequisite
	{
	private:
		TSet<IParallelProcessingUnit> PrerequisitedProcesses;
	};

	class CParallelProcessManager : public CSingleton<CParallelProcessManager>
	{
	private:
		CParallelMeshProcess ProcessMesh;

	protected:
		void OnProcessingUnitFinish(IParallelProcessingUnit * Unit);
		void OnProcessingUnitAbort(IParallelProcessingUnit * Unit);

	public:
		template<typename T>
		inline typename std::enable_if<std::is_same<T, CMeshAttributeProcessor>::value > AddProcessingUnit(T * Unit)
		{
			ProcessMesh.AddProcessingUnit(Unit);
		}
	};
}