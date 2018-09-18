#pragma once

#include <Windows.h>

#undef min
#undef max

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

class ScopedHandle
{
private:

	HANDLE Handle;

public:

	inline ScopedHandle() = default;
	inline ScopedHandle(HANDLE Handle) : Handle(Handle) {}
	inline ScopedHandle(ScopedHandle&& Other) : Handle(Other.Handle)
	{
		Other.Handle = NULL;
	}

	inline ~ScopedHandle()
	{
		if (IsValid())
		{
			Ensure(CloseHandle(Handle));
		}
	}

	inline operator HANDLE() const
	{
		return Handle;
	}

	inline void operator=(const HANDLE Other)
	{
		if (IsValid())
		{
			Ensure(Close());
		}

		Handle = Other;
	}

	inline bool IsValid() const
	{
		return Handle != NULL && Handle != INVALID_HANDLE_VALUE;
	}

	inline bool Close()
	{
		bool Result = CloseHandle(Handle);
		Handle = NULL;
		return Result;
	}
};


#define HRESULT_LAST_ERROR		(HRESULT_FROM_WIN32(GetLastError()))
#define IS_INVALID_HANDLE(X)	(X == NULL || X == INVALID_HANDLE_VALUE)
#define IS_VALID_HANDLE(X)		(X != NULL && X != INVALID_HANDLE_VALUE)
#define CLOSE_HANDLE(X)			CloseHandle(X); X = INVALID_HANDLE_VALUE;
#define THROW_IF_FAILED(x)		if(FAILED(x)) throw ComException(x);

constexpr std::pair<unsigned int, unsigned int> ExtractLoHi(size_t Size)
{
	if constexpr (sizeof(size_t) == 8)
		return std::make_pair(Size & 0x00000000FFFFFFFF, (Size >> 32) & 0x00000000FFFFFFFF);
	else
		return std::make_pair(Size, 0U);
}

constexpr size_t CombineLoHi(unsigned int Lo, unsigned int Hi)
{
	if constexpr (sizeof(size_t) == 8)
		return size_t(Lo) + ((size_t(Hi) << 32) & 0xFFFFFFFF00000000);
	else
		return Lo;
}

constexpr size_t MakeAlign
(
	const size_t Size,
	const size_t Align
)
{
	return (Size + (Align - 1)) & ~(Align - 1);
}

static SYSTEM_INFO GetSysInfo()
{
	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo);
	return SysInfo;
}