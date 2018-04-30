#pragma once

#include <string>
#include <cstring>

class String : public std::string
{
public:
	
	using std::string::basic_string;

public:

	enum ConcatenateMode
	{
		None,
		Whitespace,
		Delimiter,
		Tabulator,
		Newline
	};

public:

	inline String() = default;
	inline String(const std::string & Other) :
		std::string(Other)
	{}

	template<
		ConcatenateMode Mode = None>
	static constexpr const char * GetConcatenateString()
	{
		if constexpr (Mode == Whitespace)
		{
			return " ";
		}
		else if constexpr (Mode == Delimiter)
		{
			return ",";
		}
		else if constexpr (Mode == Tabulator)
		{
			return "\t";
		}
		else if constexpr (Mode == Newline)
		{
			return "\n";
		}
		else
		{
			return "";
		}
	}

	template<
		ConcatenateMode Mode = None, 
		class Argument>
	static std::string ToString(Argument & Arg)
	{
		if constexpr (std::is_same<Argument, ConcatenateMode>::value)
		{
			return GetConcatenateString<Argument>();
		}
		else if constexpr (std::is_arithmetic<Argument>::value)
		{
			return std::to_string(Arg);
		}
		else
		{
			return std::string(Arg);
		}
	}

	template<
		ConcatenateMode Mode = None, 
		class		Argument, 
		class...	Arguments>
	static std::string ToString(Argument & Arg, Arguments&&... Args)
	{
		if constexpr (sizeof...(Args) > 1)
		{
			return ToString<Mode>(Arg) + GetConcatenateString<Mode>() + ToString<Mode>(Args...);
		}
		else
		{
			return ToString<Mode>(Arg);
		}
	}

	template<
		ConcatenateMode Mode = None, 
		class... Arguments>
	static void MakeStringImpl(std::string & S, Arguments&&... Args)
	{
		S = S + ToString<Mode>(Args...);
	}

	template<
		ConcatenateMode Mode = None, 
		class... Arguments>
	static String MakeString(Arguments&&... Args)
	{
		String Result;
		MakeStringImpl<Mode>(Result, std::forward<Arguments>(Args)...);
		return Result;
	}
};

class StringFinder
{
private:

	String * Content;

public:

	virtual int Find(char Character)
	{

	}

	virtual int Find(const String & SubString)
	{

	}

	virtual int Find(const char * SubString)
	{

	}
};

using StringStream	= std::stringstream;
using WString		= std::wstring;
using WStringStream = std::wstringstream;
