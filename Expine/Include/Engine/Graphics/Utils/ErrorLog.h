#pragma once

#include "Types.h"
#include "Debug.h"
#include "Singleton.h"
#include "ErrorCode.h"

#include <sstream>
#include <io.h>
#include <fstream>
#include <iostream>

namespace D3D
{
	enum ELogLevel
	{
		LogInfo,
		LogWarning,
		LogError,
		LogException,
		LogFatal,
		LogNum
	};

	class CErrorLog
	{
	private:

		std::wstringstream InputStream;

	private:

		static CErrorLog LogInstance[LogNum];

	public:

		template<ELogLevel Level = LogInfo>
		static inline CErrorLog & Log()
		{
			return LogInstance[Level];
		}

		template<ELogLevel Level = LogInfo, class... Arguments>
		static inline CErrorLog & Log(Arguments&&... Args)
		{
			return (LogInstance[Level] << ... << Args);
		}

		CErrorLog() = default;
		CErrorLog(const CErrorLog & Other)
		{
			InputStream.copyfmt(Other.InputStream);
		}

		inline CErrorLog & operator << 
		(
			const char * Log
		)
		{
			if constexpr (IsDebugMode)
			{
				std::cout << Log;
			}

			InputStream << Log;
			return *this;
		}

		inline CErrorLog & operator <<
		(
			const ErrorCode & Code
		)
		{
			if constexpr (IsDebugMode)
			{
				std::cout << Code;
			}

			InputStream << Code;
			return *this;
		}

		inline CErrorLog & operator <<
		(
			CErrorLog & (*Function)(CErrorLog & Log)
		)
		{
			return Function(*this);
		}

		inline CErrorLog & operator <<
		(
			const WString & Log
		)
		{
			if constexpr (IsDebugMode)
			{
				std::wcout << Log;
			}

			InputStream << Log;
			return *this;
		}

		inline CErrorLog & operator << 
		(
			const String & Log
		)
		{
			if constexpr (IsDebugMode)
			{
				std::cout << Log;
			}

			InputStream << WString(Log.begin(), Log.end());
			return *this;
		}

		inline CErrorLog & DoEndLine()
		{
			if constexpr (IsDebugMode)
			{
				std::wcout << std::endl;
			}

			InputStream << std::endl;

			return *this;
		}

		static inline CErrorLog & EndLine(CErrorLog & Log)
		{
			return Log << "\n";
		}
	};
}