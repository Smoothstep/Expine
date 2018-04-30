#pragma once

#include <Windows.h>

namespace D3D
{
	struct ErrorCode
	{
		HRESULT Result;

		inline bool operator()() const
		{
			return FAILED(Result);
		}

		inline bool operator!() const
		{
			return SUCCEEDED(Result);
		}

		inline operator bool() const
		{
			return FAILED(Result);
		}

		inline ErrorCode operator=
		(
			HRESULT Result
		)
		{
			this->Result = Result;
#ifdef _DEBUG
			if (FAILED(Result))
			{
				_asm nop;
			}
#endif
			return *this;
		}

		inline ErrorCode
		(
			HRESULT Result
		)
		{
			this->Result = Result;
#ifdef _DEBUG
			if (FAILED(Result))
			{
				_asm nop;
			}
#endif
		}

		inline ErrorCode()
		{
			this->Result = S_OK;
		}
	};

#define ERROR_CODE ErrorCode Error;

	class ECException
	{
	private:

		ErrorCode EC;

	public:

		inline ErrorCode GetErrorCode() const
		{
			return EC;
		}

	public:

		ECException
		(
			const ErrorCode Error
		) :
			EC(Error)
		{}

		ECException() :
			EC(0)
		{}
	};

	static inline void ThrowOnError
	(
		const ErrorCode Error
	)
	{
		if (Error)
		{
			throw(ECException(Error));
		}
	}
}