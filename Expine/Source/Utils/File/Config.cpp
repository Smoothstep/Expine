#include "Utils/File/Config.h"

namespace File
{
	void CConfig::Parse()
	{
		CContentReader<String> FileReader(Storage);
	
		String Line;
		size_t LineCounter = 0;

		for (size_t LineCounter = 0; FileReader.GetNextLine(Line); ++LineCounter)
		{
			const size_t SplitOffset = Line.find_last_of('=');

			if (SplitOffset == String::npos)
			{
				continue;
			}

			if (SplitOffset == 0)
			{
				continue;
			}

			StringType Key(Line.data(), SplitOffset);

			const size_t ValueLength = Line.size() - SplitOffset - 1;

			if (ValueLength <= 1)
			{
				continue;
			}

			StringType Val(Line.data() + SplitOffset + 1, ValueLength);
			
			if (!ConfigMap.emplace(
				StringValue<StringView>(Line.data(),  SplitOffset),
				StringValue<StringView>(Line.data() + SplitOffset, Line.size() - SplitOffset)).second)
			{
				throw CConfigParseError(LineCounter, "Key already exists");
			}
		}
	}

	void CConfig::ReadConfig
	(
		CFile & File
	)
	{
		Detach(File.GetContentRef<char>(), Storage);
		{
			Parse();
		}
	}

	void CConfig::ReadConfig
	(
		String&& Content
	)
	{
		Storage = std::move(Content);
		{
			Parse();
		}
	}
}