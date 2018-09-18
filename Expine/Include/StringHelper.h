#pragma once

#include <string>

#if _HAS_CXX17
#include <string_view>

class StringView : public std::string_view
{
public:

	using std::string_view::basic_string_view;
	using std::string_view::operator=;

};

class WStringView : public std::wstring_view
{
public:
	using std::wstring_view::basic_string_view;
	using std::wstring_view::operator=;
};
#endif

class String : public std::string
{
public:
	
	using std::string::basic_string;
#if _HAS_CXX17
	// DuDe.
	using std::string::operator std::basic_string_view<char, std::char_traits<char>>;

	inline constexpr operator StringView() const
	{
		return StringView(std::string::data(), std::string::length());
	}
#endif

	enum ConcatenateMode
	{
		None,
		Whitespace,
		Delimiter,
		Tabulator,
		Newline
	};

public:

	inline const char * data() const
	{
		return std::string::data();
	}

	inline char * data()
	{
		return std::string::data();
	}

	inline String() = default;
	inline String(const std::string_view & Other) : 
		std::string(Other)
	{}
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
		else if constexpr (std::is_base_of<Argument, std::wstring>::value)
		{
			return std::string(std::begin(Arg), std::end(Arg));
		}
		else if (std::is_constructible<std::string, Argument>::value)
		{
			return std::string(Arg);
		}
		else
		{
			return std::string();
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

class WString : public std::wstring
{
public:

	using std::wstring::basic_string;
#if _HAS_CXX17
	// DuDe.
	using std::wstring::operator std::basic_string_view<wchar_t, std::char_traits<wchar_t>>;

	inline operator WStringView() const
	{
		return WStringView(std::wstring::data(), std::wstring::length());
	}
#endif

	enum ConcatenateMode
	{
		None,
		Whitespace,
		Delimiter,
		Tabulator,
		Newline
	};

public:

	inline WString() = default;
	inline WString(const std::wstring_view & Other) :
		std::wstring(Other)
	{}
	inline WString(const std::wstring & Other) :
		std::wstring(Other)
	{}
};

template<class StringType>
class UnicodeString
{
private:

	const StringType View;

public:
	UnicodeString(const StringType& View) : View(View)
	{}

	WString Unicode() const
	{
		if constexpr (std::is_constructible<WString, StringType>::value)
		{
			return WString(View);
		}
		else
		{
			return WString(View.begin(), View.end());
		}
	}
};

class StringFinder
{
private:

	const StringView Content;

public:

	StringFinder(const StringView Content) : Content(Content)
	{}

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