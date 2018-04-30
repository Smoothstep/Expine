#pragma once

#include "Object.h"
#include <boost/compute/detail/lru_cache.hpp>

namespace D3D
{
	struct ObjectTableEntry
	{
		Uint32 Id;
		Uint32 Type;
		Vector3f Position;
		Vector3f Rotation;
	};

	class ObjectTable;
	class ObjectTableCache
	{
	private:

		boost::compute::detail::lru_cache<Uint32, SharedPointer<ISceneObjectComponents> > Cache;

	public:

		ObjectTableCache();
	};

	class ObjectTableLoader : public CSingleton<ObjectTableLoader>
	{
	private:

		ObjectTableCache Cache;

	public:

		class ILoadFinish
		{
		private:



		public:

			virtual void OnStart() = 0;
			virtual void OnFinish() = 0;
		};

		void LoadTable(ObjectTable & Table);
	};

	class ObjectTable
	{
	private:
		
		TVector<ObjectTableEntry> Entries;

	protected:

		void Sort();

	public:

		inline const TVector<ObjectTableEntry> & GetEntries() const
		{
			return Entries;
		}

		template<class... Args>
		inline void AddEntry
		(
			const std::initializer_list<Args...> & Init
		)
		{
			Entries.push_back(Init);
		}

		void AddEntries
		(
			const ObjectTableEntry * Entry,
			const size_t			 NumEntries
		)
		{
			Entries.insert(Entries.end(), 
				Entry, 
				Entry + NumEntries);
		}

		ObjectTable() = default;

		template<class Container>
		ObjectTable
		(
			const Container & Elements
		)
		{
			std::copy(
				std::begin(Elements), 
				std::end(Elements), 
				std::back_inserter(Entries)
			);
		}
	};
}