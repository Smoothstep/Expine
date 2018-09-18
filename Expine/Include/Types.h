#pragma once

#include <array>
#include <vector>
#include <map>
#include <set>
#include <deque>
#include <bitset>
#include <queue>
#include <sstream>
#include <ostream>
#include <istream>
#include <mutex>
#include <atomic>
#include <immintrin.h>
#include <xmmintrin.h>
#include <StringHelper.h>

#undef min
#undef max

#include <Utils/Container/hopscotch_map.h>
#include <Utils/Container/hopscotch_set.h>

template <
	class K, 
	class V, 
	class H = std::hash<K> >
class THashMap : public tsl::hmap<K, V, H>
{
public:
	using tsl::hmap<K, V, H>::hmap;

	inline V * Find(const K& Key);
	inline V * FindOrAdd(const K& Key);
};

template<
	class K, 
	class V, 
	class H>
inline V * THashMap<K, V, H>::Find(const K& Key)
{
	auto Iter = tsl::hmap<K, V, H>::find(Key);

	if (Iter == tsl::hmap<K, V, H>::end())
	{
		return nullptr;
	}

	V * Value = reinterpret_cast<V*>(&Iter.value());

	return Value;
}

template<
	class K,
	class V,
	class H>
inline V * THashMap<K, V, H>::FindOrAdd(const K& Key)
{
	auto Iter = tsl::hmap<K, V, H>::find(Key);

	if (Iter == tsl::hmap<K, V, H>::end())
	{
		return nullptr;
	}

	V * Value = reinterpret_cast<V*>(&Iter.value());

	return Value;
}

template<
	class K,
	class V>
class TMap : public std::map<K, V>
{
public:
	using std::map<K, V>::map;

	inline			V * Find(const K& Key);
	inline const	V * Find(const K& Key) const;
};

template<
	class K,
	class V>
inline V * TMap<K, V>::Find(const K& Key)
{
	auto Iter = std::map<K, V>::find(Key);

	if (Iter == std::map<K, V>::end())
	{
		return nullptr;
	}

	V * Value = reinterpret_cast<V*>(&*Iter);

	return Value;
}

template<
	class K,
	class V>
	inline const V * TMap<K, V>::Find(const K& Key) const
{
	auto Iter = std::map<K, V>::find(Key);

	if (Iter == std::map<K, V>::end())
	{
		return nullptr;
	}

	const V * Value = reinterpret_cast<const V*>(&*Iter);

	return Value;
}

class Exception : public std::exception
{
public:
	using std::exception::exception;

	Exception(const char * Message) :
		exception(Message)
	{}
	Exception(const std::string & Message) :
		exception(Message.c_str())
	{}
};

template<class T, class A = std::allocator<T>>
class TVector : public std::vector<T, A>
{
public:
	using std::vector<T, A>::vector;

	inline void ResizeUninitialized(size_t size);
	inline void ResizeNulled(size_t size);

	inline bool Contains(const T& Element);
	inline void RemoveOnce(const T& Element);
	inline void Remove(const T& Element = T());
	inline void Fill(const T& Element = T());
	inline void Add(const T& Element = T());
	inline void AddUnitialized(size_t NumElements);

	inline T * Detach();

	template<class Pred>
	inline T * Find(Pred Predicate) const;

	template<class Pred>
	inline void FindVoid(Pred Predicate) const;

	template<class Comp>
	inline T * FindSorted(Comp Comparator) const;

	template<class Comp>
	inline void FindSortedVoid(Comp Comparator) const;

	inline TVector<T, A> CartesianProduct();
};

template<class T>
using MappedVector = TVector<T, std::allocator<T> >;

template<class T, class A>
template<class Pred>
inline T * TVector<T, A>::Find(Pred Predicate) const
{
	for (auto Iter = std::vector<T, A>::begin(); Iter != std::vector<T, A>::end(); ++Iter)
	{
		if (Predicate(*Iter))
		{
			return std::addressof(*Iter);
		}
	}

	return nullptr;
}

template<class T, class A>
template<class Pred>
inline void TVector<T, A>::FindVoid(Pred Predicate) const
{
	for (auto Iter = std::vector<T, A>::begin(); Iter != std::vector<T, A>::end(); ++Iter)
	{
		if (Predicate(*Iter))
		{
			return;
		}
	}
}

template<class T, class A>
template<class Comp>
inline T * TVector<T, A>::FindSorted(Comp Compare) const
{
	size_t L = 0;
	size_t R = std::vector<T, A>::size() - 1;
	size_t C;
	size_t M;

	while (L <= R)
	{
		M = L + (R - L) / 2;

		const T & Value = std::vector<T, A>::at(M);

		if ((C = Compare(Value)) == 0)
		{
			return std::addressof(Value);
		}

		if (C > 0)
		{
			R = M - 1;
		}
		else
		{
			L = M + 1;
		}
	}

	return nullptr;
}

template<class T, class A>
template<class Comp>
inline void TVector<T, A>::FindSortedVoid(Comp Compare) const
{
	size_t L = 0;
	size_t R = std::vector<T, A>::size() - 1;
	size_t C;
	size_t M;

	while (L <= R)
	{
		M = L + (R - L) / 2;

		const T & Value = std::vector<T, A>::at(M);

		if ((C = Compare(Value)) == 0)
		{
			return;
		}

		if (C > 0)
		{
			R = L - 1;
		}
		else
		{
			L = L + R / 2 + 1;
		}
	}
}

template<class T, class A>
inline void TVector<T, A>::AddUnitialized(size_t NumElements)
{
	std::vector<T, A>::reserve(this->capacity() + NumElements);

	std::vector<T, A>::_Myfirst() = std::vector<T, A>::data();
	std::vector<T, A>::_Mylast() = std::vector<T, A>::data() + std::vector<T, A>::capacity();
	std::vector<T, A>::_Myend() = std::vector<T, A>::data() + std::vector<T, A>::capacity();
}

template<class T, class A>
inline void TVector<T, A>::ResizeUninitialized(size_t size)
{
	std::vector<T, A>::reserve(size);

	std::vector<T, A>::_Myfirst() = std::vector<T, A>::data();
	std::vector<T, A>::_Mylast() = std::vector<T, A>::data() + std::vector<T, A>::capacity();
	std::vector<T, A>::_Myend() = std::vector<T, A>::data() + std::vector<T, A>::capacity();
}

template<class T, class A>
inline void TVector<T, A>::ResizeNulled(size_t size)
{
	std::vector<T, A>::reserve(size);

	std::vector<T, A>::_Myfirst() = std::vector<T, A>::data();
	std::vector<T, A>::_Mylast() = std::vector<T, A>::data() + std::vector<T, A>::capacity();
	std::vector<T, A>::_Myend() = std::vector<T, A>::data() + std::vector<T, A>::capacity();

	memset(std::vector<T, A>::data(), 0, size * sizeof(T));
}

template<class T, class A>
inline bool TVector<T, A>::Contains(const T& Element)
{
	return std::find(std::vector<T, A>::begin(), std::vector<T, A>::end(), Element) != std::vector<T, A>::end();
}

template<class T, class A>
inline void TVector<T, A>::RemoveOnce(const T& Element)
{
	auto Iter = std::find(std::vector<T, A>::begin(), std::vector<T, A>::end(), Element);

	if (Iter != std::vector<T, A>::end())
	{
		std::vector<T, A>::erase(Iter);
	}
}

template<class T, class A>
inline void TVector<T, A>::Remove(const T& Element)
{
	std::remove_if(std::vector<T, A>::begin(), std::vector<T, A>::end(), [Element](const T& E)
	{
		return E == Element;
	});
}

template<class T, class A>
inline void TVector<T, A>::Fill(const T& Element)
{
	std::fill(std::vector<T, A>::begin(), std::vector<T, A>::end(), Element);
}

template<class T, class A>
inline T * TVector<T, A>::Detach()
{
	T * Data = std::vector<T, A>::_Myfirst();

	std::vector<T, A>::_Myfirst() = std::vector<T, A>::_Mylast() = std::vector<T, A>::_Myend() = 0;

	return Data;
}

template<class T, class A>
inline TVector<T, A> TVector<T, A>::CartesianProduct()
{
	TVector<T, A> Result;

	for (auto Y = std::vector<T, A>::begin(); Y != std::vector<T, A>::end(); ++Y)
	{
		for (auto X = std::vector<T, A>::begin(); X != std::vector<T, A>::end(); ++X)
		{
			if (X == Y)
			{
				continue;
			}

			Result.push_back(*X * *Y);
		}
	}
}

template<class T, class A = std::allocator<T>>
class TSmartVector : public TVector<T, A>
{
public:
	using TVector<T, A>::TVector;
};

using StringStream		= std::stringstream;
using WStringStream		= std::wstringstream;
using WStringList		= TVector<WString>;
using StringList		= TVector<String>;
using StringViewList	= TVector<StringView>;
using WStringViewList	= TVector<WStringView>;

template<class ElementType, class Allocator>
inline void Detach(std::basic_string<ElementType, std::char_traits<ElementType>, Allocator>& From, TVector<ElementType, Allocator>& To)
{
	auto& Data = From._Get_data();

	if (From.capacity() < 16)
	{
		// SSO
		To.resize(From.capacity());
		To.shrink_to_fit();
		To.insert(To.begin(), From.begin(), From.end());
	}
	else
	{
		To._Myfirst()	= Data._Bx._Ptr;
		To._Mylast()	= Data._Bx._Ptr + Data._Mysize;
		Data._Bx._Ptr	= 0;
		Data._Mysize	= 0;
		Data._Myres		= 0;
	}
}

template<class ElementType, class Allocator>
inline void Detach(TVector<ElementType, Allocator>& From, std::basic_string<ElementType, std::char_traits<ElementType>, Allocator>& To)
{
	auto& Data = To._Get_data();

	if (From.capacity() < 16)
	{
		// SSO
		To.resize(From.capacity());
		To.shrink_to_fit();
		To.insert(To.begin(), From.begin(), From.end());
	}
	else
	{
		Data._Mysize	= From.size();
		Data._Myres		= From.capacity();
		Data._Bx._Ptr	= From.Detach();
	}
}

template <class T>
using TSet 
= std::set<T>;

template <class T>
using THashSet
= tsl::hopscotch_set<T>;

template <class T, size_t N>
using TArray 
= std::array<T, N>;

template <size_t N>
using BoolArray 
= TArray<bool, N>;

template <size_t N>
using StringArray 
= TArray<String, N>;

template <class L, class R>
using TPair 
= std::pair<L, R>;

template <size_t S>
using TBitset
= std::bitset<S>;

template <class T>
using THash
= std::hash<T>;

template <class T>
using TUniquePtr
= std::unique_ptr<T>;

template <class T>
using TQueue
= std::queue<T>;

template <class T>
using TDeque
= std::deque<T>;

template <class Type, class Comp = std::less<Type>, class Cont = TVector<Type>>
using TPriorityQueue
= std::priority_queue<Type, Cont, Comp>;

template <class T>
using TAtomic
= std::atomic<T>;

using TMutex
= std::mutex;

typedef unsigned char Byte;

namespace Hyper
{
	typedef int					Int;
	typedef unsigned int		Uint;
	typedef float				Float;
	typedef double				Double;
	typedef __m128				M128;
	typedef __m128i				M128i;
	typedef __m256				M256;
	typedef __m256i				M256i;
	typedef float				Float1x1[1][1];
	typedef float				Float2x2[2][2];
	typedef float				Float3x3[3][3];
	typedef float				Float4x4[4][4];
	typedef float				Float4[4];
	typedef float				Float3[3];
	typedef float				Float2[2];
	typedef int					int32;
	typedef int					Int32;
	typedef long long			Int64;
	typedef long long			int64;
	typedef unsigned long long	Uint64;
	typedef unsigned long long	uint64;
	typedef unsigned int		uint32;
	typedef unsigned int		Uint32;
	typedef unsigned char		uint8;
	typedef unsigned char		Uint8;
	typedef unsigned short		uint16;
	typedef unsigned short		Uint16;
	typedef char				int8;
	typedef char				Int8;
	typedef short				int16;
	typedef short				Int16;
	typedef bool				Bool;
}

namespace std
{
	template<>
	struct hash<String>
	{
		inline size_t operator()(const String& Value) const
		{
			return hash<std::string>()(Value);
		}
	};
	template<>
	struct hash<WString>
	{
		inline size_t operator()(const WString& Value) const
		{
			return hash<std::wstring>()(Value);
		}
	};
	template<>
	struct hash<StringView>
	{
		inline size_t operator()(const StringView& Value) const
		{
			return std::hash<std::string_view>()(Value);
		}
	};
	template<>
	struct hash<WStringView>
	{
		inline size_t operator()(const WStringView& Value) const
		{
			return std::hash<std::wstring_view>()(Value);
		}
	};
}

enum EAxis
{
	X, Y, Z
};