#pragma once

#include "Types.h"
#include "Defines.h"

#include <algorithm>

#include "Hyper.h"

template
<
	class Derived
>
class StringOp
{
	FORCEINLINE const char * GetData() const
	{
		return (static_cast<const Derived*>(this))->Data();
	}

	FORCEINLINE size_t GetSize() const
	{
		return (static_cast<const Derived*>(this))->Size();
	}

public:

	static FORCEINLINE bool IsCharArray
	(
		const char * P
	)
	{
		return P != NULL && P[0] != NULL;
	}

	/************************************************************
	*
	*	Returns true if character array is a naive integer.
	*
	************************************************************/

	static FORCEINLINE bool IsInteger
	(
		const char	*	P,
		const unsigned	S
	)
	{
		for (unsigned N = 0; N < S; ++N)
		{
			unsigned char C = P[N];

			if (C - '0' > 9)
			{
				if (N != 0)
				{
					return false;
				}

				if (C != '-')
				{
					return false;
				}
			}
		}

		return true;
	}

	/************************************************************
	*
	*	Returns true if character array is a naive floating point.
	*
	************************************************************/

	static FORCEINLINE bool IsFloatingPoint
	(
		const char	*	P,
		const unsigned	S
	)
	{
		for (unsigned N = 0; N < S; ++N)
		{
			unsigned char C = P[N];

			if (C - '0' > 9)
			{
				if (C != '.')
				{
					return false;
				}

				if (N == 0)
				{
					return false;
				}
			}
		}

		return true;
	}

	/********************************************************************************************************
	*
	*	Returns true if character array is a floating point with format of (-)x[1 + n].x[1 + m]( ) or (-)x[1 + n]( ) .
	*
	********************************************************************************************************/

	template
	<
		typename FloatingPoint
	> 
	static bool __fastcall TryConvertToFloatingPoint
	(
		const char * P,
		FloatingPoint & R
	)
	{
		if (!IsCharArray(P))
		{
			return false;
		}

		FloatingPoint D = 0.0;
		FloatingPoint F;

		bool N;

		if (*P == '-')
		{
			N = true;
			++P;
		}
		else
		{
			N = false;
		}

		while (*P - '0' <= 9U)
		{
			D = (D * 10) + (*P++ - '0');
		}

		if (*P == '.')
		{
			F = 0.0;

			const char * O = P++;

			while (*P - '0' <= 9U)
			{
				F = (F * 10) + (*P++ - '0');
			}

			D += F * Math::FPower10LookupTable[P - O];
		}

		if (*P && *P != ' ')
		{
			return false;
		}

		if (N)
		{
			R = -D;
		}
		else
		{
			R = D;
		}

		return true;
	}

	/**********************************************************************************
	*
	*	Returns true if character array is a integer with format of (-)x[1 + n]( ) .
	*
	**********************************************************************************/

	template
	<
		typename Integer
	>
	static bool __fastcall TryConvertToInteger
	(
		const char * P,
		Integer & R
	)
	{
		if (!IsCharArray(P))
		{
			return false;
		}

		bool N;

		if (*P == '-')
		{
			N = true;
			++P;
		}
		else
		{
			N = false;
		}

		int V = 0;

		while (*P)
		{
			if (*P - '0' > 9U)
			{
				if (*P == ' ')
				{
					break;
				}

				return false;
			}
			else
			{
				V = V * 10 + (*P++ - '0');
			}
		}

		R = N ? -V : V;

		return true;
	}

	template
	<
		typename Integer
	>
	static bool __fastcall TryConvertToUnsignedInteger
	(
		const char * P,
		Integer & R
	)
	{
		if (!IsCharArray(P))
		{
			return false;
		}

		int V = 0;

		while (*P)
		{
			if (*P - '0' > 9U)
			{
				if (*P == ' ')
				{
					break;
				}

				return false;
			}
			else
			{
				V = V * 10 + (*P++ - '0');
			}
		}

		R = V;

		return true;
	}

	FORCEINLINE bool operator >>
	(
		double & R
	) const
	{
		return TryConvertToFloatingPoint(GetData(), R);
	}

	FORCEINLINE bool operator >>
	(
		String & R
	) const
	{
		R = String(GetData(), GetSize());
		return true;
	}

	FORCEINLINE bool operator >>
	(
		float & R
	) const
	{
		return TryConvertToFloatingPoint(GetData(), R);
	}

	FORCEINLINE bool operator >>
	(
		int8_t & R
	) const
	{
		return TryConvertToInteger(GetData(), R);
	}

	FORCEINLINE bool operator >>
	(
		int16_t & R
	) const
	{
		return TryConvertToInteger(GetData(), R);
	}

	FORCEINLINE bool operator >>
	(
		int32_t & R
	) const
	{
		return TryConvertToInteger(GetData(), R);
	}

	FORCEINLINE bool operator >>
	(
		int64_t & R
	) const
	{
		return TryConvertToInteger(GetData(), R);
	}

	FORCEINLINE bool operator >>
	(
		uint8_t & R
	) const
	{
		return TryConvertToInteger(GetData(), R);
	}

	FORCEINLINE bool operator >>
	(
		uint16_t & R
	) const
	{
		return TryConvertToInteger(GetData(), R);
	}

	FORCEINLINE bool operator >>
	(
		uint32_t & R
	) const
	{
		return TryConvertToInteger(GetData(), R);
	}

	FORCEINLINE bool operator >>
	(
		uint64_t & R
	) const
	{
		return TryConvertToInteger(GetData(), R);
	}

	FORCEINLINE bool operator >>
	(
		WString & R
	) const
	{
		if (GetSize())
		{
			R.reserve(GetSize() * 2);
			MultiByteToWideChar(CP_UTF8, 0, GetData(), GetSize(), R.data(), R.size());
			return true;
		}
		return false;
	}

	FORCEINLINE bool operator >>
	(
		Vector2f & R
	) const
	{
		unsigned O = 0;
		unsigned L;

		const char * P = GetData();

		Vector2f V;

#pragma unroll(1)
		for (unsigned N = 0; N < 2; ++N)
		{
			L = O;
			O = static_cast<const Derived*>(this)->Find(' ', O);

			if (O == static_cast<const Derived*>(this)->npos)
			{
				if (!TryConvertToFloatingPoint(P + O + 1, V.Component(N)))
				{
					return false;
				}

				R = V;

				return true;
			}

			if (!TryConvertToFloatingPoint(P + L, V.Component(N)))
			{
				return false;
			}

			O++;
		}

		return false;
	}

	FORCEINLINE bool operator >>
	(
		Vector3f & R
	) const
	{
		Vector3f V;

		unsigned O = 0;
		unsigned L;

		const char * P = GetData();

#pragma unroll(1)
		for (unsigned N = 0; N < 3; ++N)
		{
			L = O;
			O = static_cast<const Derived*>(this)->Find(' ', O);

			if (O == static_cast<const Derived*>(this)->npos)
			{
				if (!TryConvertToFloatingPoint(P + O + 1, V.Component(N)))
				{
					return false;
				}

				R = V;

				return true;
			}

			if (!TryConvertToFloatingPoint(P + L, V.Component(N)))
			{
				return false;
			}

			O++;
		}

		return false;
	}

	FORCEINLINE bool operator >>
	(
		Vector4f & R
	) const
	{
		Vector4f V;

		unsigned O = 0;
		unsigned L;

		const char * P = GetData();

#pragma unroll(1)
		for (unsigned N = 0; N < 4; ++N)
		{
			L = O;
			O = static_cast<const Derived*>(this)->Find(' ', O);

			if (O == static_cast<const Derived*>(this)->npos)
			{
				if (!TryConvertToFloatingPoint(P + O + 1, V.Component(N)))
				{
					return false;
				}

				R = V;

				return true;
			}

			if (!TryConvertToFloatingPoint(P + L, V.Component(N)))
			{
				return false;
			}

			O++;
		}

		return false;
	}
};

class StringEmu : 
	public StringOp<StringEmu>
{
	char * DataPointer;
	size_t DataLength;

public:

	using StringOp<StringEmu>::operator>>;

	FORCEINLINE StringEmu()
	{
		DataPointer = nullptr;
		DataLength = 0;
	}

	template<class Iterator>
	FORCEINLINE StringEmu
	(
		Iterator Begin,
		Iterator End
	)
	{
		DataPointer = reinterpret_cast<char*>(&*Begin);
		DataLength = std::distance(Begin, End);
	}

	FORCEINLINE StringEmu
	(
		char * P,
		size_t L
	)
	{
		DataLength = L;
		DataPointer = P;
	}

	FORCEINLINE StringEmu
	(
		char * P
	)
	{
		DataLength = std::strlen(DataPointer);
		DataPointer = P;
	}

	FORCEINLINE char * Data() const
	{
		return DataPointer;
	}

	FORCEINLINE size_t Size() const
	{
		return DataLength;
	}

	FORCEINLINE void Resize(size_t NewLength)
	{
		DataLength = NewLength;
	}

	FORCEINLINE char operator[](const size_t Index)
	{
		return DataPointer[Index];
	}

	FORCEINLINE size_t Find
	(
		const char C,
		const size_t S
	)	const
	{
		auto Result = std::find(
			DataPointer, 
			DataPointer + DataLength, 
			C);

		if (!Result)
		{
			return -1;
		}

		return Result - DataPointer;
	}

	FORCEINLINE bool operator >>
	(
		String & R
	)
	{
		R = String(DataPointer, DataPointer + DataLength);
		{
			return true;
		}
	}

	FORCEINLINE bool operator >>
	(
		WString & R
	)
	{
		R = WString(DataPointer, DataPointer + DataLength);
		{
			return true;
		}
	}
};

template<
	class StringType = String>
class StringValue : 
	public StringType,
	public StringOp<StringValue<StringType>>
{
public:

	using StringType::operator=;
	using StringType::operator[];
	using StringOp<StringValue>::operator>>;

public:

	FORCEINLINE StringValue() = default;
	FORCEINLINE StringValue
	(
		const char * P
	) : StringType(P)
	{}

	FORCEINLINE StringValue
	(
		const char * P,
		const size_t L
	) : StringType(P, L)
	{}

	FORCEINLINE StringValue
	(
		const StringType & S
	) : StringType(S)
	{}

	FORCEINLINE bool operator >>
	(
		String & R
	)
	{
		R = *this;
		{
			return true;
		}
	}

	FORCEINLINE bool operator >>
	(
		WString & R
	)
	{
		R = WString(StringType::begin(), StringType::end());
		{
			return true;
		}
	}

	FORCEINLINE size_t Find
	(
		char C, 
		size_t S
	)
	{
		return StringType::find_first_of(C, S);
	}

	FORCEINLINE bool StartsWith
	(
		const char* Str
	)	const
	{
		size_t Length = strnlen_s(Str, -1);

		if (StringType::length() < Length)
		{
			return false;
		}

		return strncmp(StringType::data(), Str, Length) == 0;
	}

	FORCEINLINE bool StartsWith
	(
		const StringType& Str
	)	const
	{
		size_t Length = Str.length();

		if (StringType::length() < Length)
		{
			return false;
		}

		return strncmp(StringType::data(), Str.data(), Length);
	}

	FORCEINLINE bool EndsWith
	(
		const StringType& Str
	)	const
	{
		size_t Length = Str.length();

		if (StringType::length() < Length)
		{
			return false;
		}

		return strncmp(StringType::data() + StringType::length() - Length, Str.data(), Length);
	}

	template
	<
		typename O,
		typename C
	>
	FORCEINLINE StringValue FetchSubString
	(
		const O & Opening,
		const C & Closure
	)	const
	{
		auto First = std::find(StringType::begin(), StringType::end(), Opening);

		if (First == StringType::end())
		{
			return StringValue();
		}

		return StringValue(StringType
		(
			First, std::find(First, StringType::end(), Closure)
		));
	}

	FORCEINLINE StringValue FetchSubStringBetween
	(
		const StringType & Opening,
		const StringType & Closure
	)	const
	{
		size_t Begin = StringType::find(Opening, 0);

		if (Begin == StringType::npos)
		{
			return StringType();
		}

		Begin += Opening.length();

		size_t End = find(Closure, Begin);

		if (End == StringType::npos)
		{
			return StringValue(StringType::substr(Begin + Opening.length(), StringType::npos));
		}

		return StringValue(StringType::substr(Begin, End - Begin));
	}

	FORCEINLINE TVector<String> Split
	(
		const char * Seperator
	)
	{
		TVector<String> List;
		
		size_t N = 0;
		size_t M = 0;
		
		while(true)
		{
			M = N;
			N = StringType::find_first_of(Seperator, N);

			if(N != StringType::npos)
			{
				if (N == M)
				{
					continue;
				}

				List.emplace_back(StringType::begin() + M, StringType::begin() + N);
			}
			else
			{
				break;
			}

			N++;
		}

		List.emplace_back(StringType::begin() + M, StringType::end());

		return List;
	}

	FORCEINLINE TVector<String> Split
	(
		const char Seperator
	)
	{
		TVector<String> List;

		size_t N = 0;
		size_t M = 0;

		while (true)
		{
			M = N;
			N = StringType::find_first_of(Seperator, N + 1);

			if (N != StringType::npos)
			{
				if (N == M)
				{
					continue;
				}

				List.emplace_back(StringType::begin() + M, StringType::begin() + N);
			}
			else
			{
				break;
			}

			N++;
		}

		List.emplace_back(StringType::begin() + M, StringType::end());

		return List;
	}

	template
	<
		typename O
	>
	FORCEINLINE StringValue FetchSubString
	(
		const O & Opening
	)	const
	{
		return StringValue(std::find(StringType::begin(), StringType::end(), Opening), StringType::end());
	}

	FORCEINLINE const char * Data() const
	{
		return StringType::data();
	}

	FORCEINLINE size_t Size() const
	{
		return StringType::size();
	}
};

namespace std
{
	template<class UnderlyingType>
	struct hash<StringValue<UnderlyingType> >
	{
		inline size_t operator()(const StringValue<UnderlyingType>& Value) const
		{
			return std::hash<UnderlyingType>()(static_cast<const UnderlyingType&>(Value));
		}
	};
}