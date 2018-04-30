#pragma once

#include "File.h"
#include "../LexicalCast.h"

namespace File
{
	class CConfig
	{
	private:

		THashMap<String, StringValue> ConfigMap;

	public:

		inline StringValue & operator[]
		(
			const char * Key
		)
		{
			return ConfigMap[Key];
		}

		inline StringValue & operator[]
		(
			const String & Key
		)
		{
			return ConfigMap[Key];
		}

		inline bool GetValue
		(
			const	String		& Key,
					StringValue & Value
		)
		{
			auto Iter = ConfigMap.find(Key);

			if (Iter == ConfigMap.end())
			{
				return false;
			}

			Value = Iter->second;

			return true;
		}

		inline bool Contains
		(
			const String & Key
		)	const
		{
			return ConfigMap.find(Key) != ConfigMap.end();
		}

	public:

		void ReadConfig
		(
			const CFile & File
		);
	};
}