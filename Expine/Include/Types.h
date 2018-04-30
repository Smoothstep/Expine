#pragma once

#include <array>
#include <vector>
#include <map>
#include <set>
#include <bitset>
#include <queue>
#include <sstream>
#include <ostream>
#include <istream>
#include <strstream>
#include <immintrin.h>
#include <xmmintrin.h>
#include <String.h>

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/thread/mutex.hpp>

#include <Container/HopscotchMap.h>

template <
	class K, 
	class V, 
	class H = std::hash<K> >
class THashMap : public tsl::hopscotch_map<K, V, H>
{
public:
	using tsl::hopscotch_map<K, V, H>::hopscotch_map;

	inline V * Find(const K& Key);
	inline V * FindOrAdd(const K& Key);
};

template<
	class K, 
	class V, 
	class H>
inline V * THashMap<K, V, H>::Find(const K& Key)
{
	auto Iter = this->find(Key);

	if (Iter == this->end())
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
	auto Iter = this->find(Key);

	if (Iter == this->end())
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

	inline V * Find(const K& Key);
};

template<
	class K,
	class V>
inline V * TMap<K, V>::Find(const K& Key)
{
	auto Iter = this->find(Key);

	if (Iter == this->end())
	{
		return nullptr;
	}

	V * Value = reinterpret_cast<V*>(&*Iter);

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

template<class T>
class TVector : public std::vector<T>
{
public:
	using std::vector<T>::vector;

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

	inline TVector<T> CartesianProduct();
};

template<class T>
template<class Pred>
inline T * TVector<T>::Find(Pred Predicate) const
{
	for (auto Iter = begin(); Iter != end(); ++Iter)
	{
		if (Predicate(*Iter))
		{
			return std::addressof(*Iter);
		}
	}

	return nullptr;
}

template<class T>
template<class Pred>
inline void TVector<T>::FindVoid(Pred Predicate) const
{
	for (auto Iter = begin(); Iter != end(); ++Iter)
	{
		if (Predicate(*Iter))
		{
			return;
		}
	}
}

template<class T>
template<class Comp>
inline T * TVector<T>::FindSorted(Comp Compare) const
{
	size_t L = 0;
	size_t R = size() - 1;
	size_t C;

	while (L <= R)
	{
		const T & Value = at(L + ((R - L) / 2));

		if ((C = Compare(Value)) == 0)
		{
			return std::addressof(Value);
		}

		if (Cmp > 0)
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

template<class T>
template<class Comp>
inline void TVector<T>::FindSortedVoid(Comp Compare) const
{
	size_t L = 0;
	size_t R = size() - 1;
	size_t C;

	while (L <= R)
	{
		const T & Value = at(L + ((R - L) / 2));

		if ((C = Compare(Value)) == 0)
		{
			return;
		}

		if (Cmp > 0)
		{
			R = M - 1;
		}
		else
		{
			L = M + 1;
		}
	}
}

template<class T>
inline void TVector<T>::AddUnitialized(size_t NumElements)
{
	std::vector<T>::reserve(this->capacity() + NumElements);

	this->_Myfirst() = this->data();
	this->_Mylast() = this->data() + this->capacity();
	this->_Myend() = this->data() + this->capacity();
}

template<class T>
inline void TVector<T>::ResizeUninitialized(size_t size)
{
	std::vector<T>::reserve(size);

	this->_Myfirst() = this->data();
	this->_Mylast() = this->data() + this->capacity();
	this->_Myend() = this->data() + this->capacity();
}

template<class T>
inline void TVector<T>::ResizeNulled(size_t size)
{
	std::vector<T>::reserve(size);

	this->_Myfirst() = this->data();
	this->_Mylast() = this->data() + this->capacity();
	this->_Myend() = this->data() + this->capacity();

	memset(this->data(), 0, size * sizeof(T));
}

template<class T>
inline bool TVector<T>::Contains(const T& Element)
{
	return std::find(this->begin(), this->end(), Element) != this->end();
}

template<class T>
inline void TVector<T>::RemoveOnce(const T& Element)
{
	auto Iter = std::find(this->begin(), this->end(), Element);

	if (Iter != this->end())
	{
		this->erase(Iter);
	}
}

template<class T>
inline void TVector<T>::Remove(const T& Element)
{
	while (auto Iter = std::find(this->begin(), this->end(), Element) != this->end())
	{
		this->erase(Iter);
	}
}

template<class T>
inline void TVector<T>::Fill(const T& Element)
{
	std::fill(this->begin(), this->end(), Element);
}

template<class T>
inline T * TVector<T>::Detach()
{
	T * Data = this->_Myfirst();

	this->_Myfirst() = this->_Mylast() = this->_Myend() = 0;

	return Data;
}

template<class T>
inline TVector<T> TVector<T>::CartesianProduct()
{
	TVector<T> Result;

	for (auto Y = begin(); Y != end(); ++Y)
	{
		for (auto X = begin(); X != end(); ++X)
		{
			if (X == Y)
			{
				continue;
			}

			Result.push_back(*X * *Y);
		}
	}
}

template<class T>
class TSmartVector : public TVector<T>
{
public:
	using TVector<T>::TVector;
};

template<class T>
class TPointerVector : public boost::ptr_vector<T>
{
public:
	using boost::ptr_vector<T>::ptr_vector;
};

using WStringList	= TVector<WString>;
using StringList	= TVector<String>;

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

template <class Type, class Comp = std::less<Type>, class Cont = TVector<Type>>
using TPriorityQueue
= std::priority_queue<Type, Cont, Comp>;

template <class T>
using TAtomic
= std::atomic<T>;

using TMutex
= boost::mutex;

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
}

enum EAxis
{
	X, Y, Z
};