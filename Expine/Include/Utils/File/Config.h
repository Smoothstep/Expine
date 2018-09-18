#pragma once

#include "Utils/File/File.h"
#include "Utils/StringOp.h"

namespace File
{
	class CConfigParseError
	{
	private:
		size_t Line;
		StringView Reason;

	public:
		CConfigParseError(const size_t Line, const StringView Reason) : 
			Line(Line), Reason(Reason) {}
	};

	class CConfig
	{
	private:

		using StringType = StringValue<StringView>;
		using StringMap = THashMap<StringType, StringType>;

		String		Storage;
		StringMap	ConfigMap;

	public:

		inline StringType & operator[]
		(
			const StringType & Key
		)
		{
			return ConfigMap[Key];
		}

		inline bool GetValue
		(
			const	StringType	& Key,
					StringType	& Value
		)	const
		{
			auto Iter = ConfigMap.find(Key);

			if (Iter != ConfigMap.end())
			{
				Value = Iter.value();
				return true;
			}

			return false;
		}

		inline bool Contains
		(
			const StringType & Key
		)	const
		{
			return ConfigMap.find(Key) != ConfigMap.end();
		}

	public:

		void ReadConfig
		(
			CFile & File
		);

		void ReadConfig
		(
			String&& Config
		);

	private:

		void Parse();
	};
}