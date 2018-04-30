#pragma once

#include <Windows.h>

class ComException : public std::exception
{
public:
	ComException(HRESULT hr) : Result(hr) {}

	virtual const char* what() const override
	{
		static char s_str[64] = { 0 };
		sprintf_s(s_str, "Failure with HRESULT of %08X", Result);
		return s_str;
	}

private:

	HRESULT Result;
};

class ScopedCriticalSection
{
private:

	LPCRITICAL_SECTION Section;

public:

	ScopedCriticalSection
	(
		LPCRITICAL_SECTION pSection
	)
	{
		EnterCriticalSection(Section = pSection);
	}

	~ScopedCriticalSection()
	{
		LeaveCriticalSection(Section);
	}
};


#define HRESULT_LAST_ERROR		(HRESULT_FROM_WIN32(GetLastError()))
#define IS_INVALID_HANDLE(X)	(X == NULL || X == INVALID_HANDLE_VALUE)
#define IS_VALID_HANDLE(X)		(!IS_INVALID_HANDLE(X))
#define CLOSE_HANDLE(X)			CloseHandle(X); X = INVALID_HANDLE_VALUE;
#define THROW_IF_FAILED(x)		if(FAILED(x)) throw new ComException(x);