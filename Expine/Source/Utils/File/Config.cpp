#include "File/Config.h"

namespace File
{
	void CConfig::ReadConfig
	(
		const CFile & File
	)
	{
		CFileReader FileReader(File.GetContentRef());

		String Line;

		while (FileReader.GetNextLine(Line))
		{
			size_t SplitOffset = Line.find_last_of('=');

			if (SplitOffset == String::npos)
			{
				continue;
			}

			if (SplitOffset == 0)
			{
				continue;
			}

			ConfigMap.insert(TMap<String, StringValue>::value_type
			(
				Line.substr(0, SplitOffset),
				Line.substr(SplitOffset + 1)
			));
		}
	}
}