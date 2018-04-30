#pragma once

#include "Defines.h"

#include <Windows.h>

template<class Typename> class Pointer
{
protected:

	mutable Typename * ObjectPointer = NULL;

public:

	FORCEINLINE Pointer
	(
		Typename * Object
	)
	{
		ObjectPointer = Object;
	}

	FORCEINLINE Pointer
	(
		decltype (nullptr) Null
	)
	{
	}

	FORCEINLINE Pointer()
	{
	}

	FORCEINLINE operator bool() const
	{
		return ObjectPointer != NULL;
	}

	FORCEINLINE Typename * operator->() const
	{
		return ObjectPointer;
	}

	FORCEINLINE Typename * operator()() const
	{
		return ObjectPointer;
	}

	FORCEINLINE bool Nil() const
	{
		return ObjectPointer == nullptr;
	}

	FORCEINLINE Typename * Get() const
	{
		return ObjectPointer;
	}

	FORCEINLINE Typename * Copy() const
	{
		return new Typename(*ObjectPointer);
	}

	FORCEINLINE const Typename & GetRef() const
	{
		return *ObjectPointer;
	}

	FORCEINLINE Typename ** operator&()
	{
		return &ObjectPointer;
	}

	FORCEINLINE Typename * operator*() const
	{
		return ObjectPointer;
	}

	FORCEINLINE bool operator==
	(
		const decltype(nullptr)
	)	const
	{
		return ObjectPointer == nullptr;
	}

	FORCEINLINE bool operator==
	(
		const decltype(NULL)
	)	const
	{
		return ObjectPointer == nullptr;
	}

	FORCEINLINE bool operator==
	(
		const Typename * Other
	)	const
	{
		return ObjectPointer == Other;
	}

	FORCEINLINE bool operator==
	(
		const Pointer<Typename> & Other
	)	const
	{
		return ObjectPointer == Other.ObjectPointer;
	}
};

template<class Typename> class UniquePointer : public Pointer<Typename>
{
public:

	using Pointer::operator();
	using Pointer::operator->;
	using Pointer::Pointer;

	FORCEINLINE void SafeRelease()
	{
		if (ObjectPointer)
		{
			delete ObjectPointer;
		}

		ObjectPointer = NULL;
	}

	FORCEINLINE UniquePointer
	(
		const UniquePointer<Typename> & Other
	)
	{
		*this = Other.Detach();
	}

	FORCEINLINE UniquePointer()
	{}

	FORCEINLINE UniquePointer
	(
		const UniquePointer<Typename> && Other
	)
	{
		*this = Other.Detach();
	}

	FORCEINLINE UniquePointer
	(
		Typename * const Object
	)
	{
		ObjectPointer = Object;
	}
	
	FORCEINLINE ~UniquePointer()
	{
		SafeRelease();
	}

	FORCEINLINE Typename * Detach() const
	{
		Typename * Tmp = ObjectPointer;

		ObjectPointer = NULL;

		return Tmp;
	}

	FORCEINLINE UniquePointer & operator=
	(
		Typename * const Object
	)
	{
		ObjectPointer = Object;
		return *this;
	}

	FORCEINLINE UniquePointer & operator=
	(
		const UniquePointer<Typename> & Other
	)
	{
		*this = Other.Detach();
		return *this;
	}
};

class ReferenceCounter
{
private:

	volatile unsigned long ReferenceCount = 1;
	volatile unsigned long ReferenceCountWeak = 0;

public:

	FORCEINLINE unsigned long Add()
	{
		return InterlockedIncrement(&ReferenceCount);
	}

	FORCEINLINE unsigned long Remove()
	{
		return InterlockedDecrement(&ReferenceCount);
	}

	FORCEINLINE unsigned long AddWeak()
	{
		return InterlockedIncrement(&ReferenceCountWeak);
	}

	FORCEINLINE unsigned long RemoveWeak()
	{
		return InterlockedDecrement(&ReferenceCountWeak);
	}

	FORCEINLINE unsigned long WeakReferences() const
	{
		return ReferenceCountWeak;
	}

	FORCEINLINE unsigned long References() const
	{
		return ReferenceCount;
	}
};

template<class Typename> class SharedPointer : public Pointer<Typename>
{
	friend class WeakPointer<Typename>;

private:

	ReferenceCounter * Counter = NULL;

private:

	FORCEINLINE void Set
	(
		const SharedPointer<Typename> & Other
	)
	{
		RemoveReference();
		
		Counter			= Other.Counter;
		ObjectPointer	= Other.ObjectPointer;

		if (Counter)
		{
			AddReference();
		}
	}

public:

	FORCEINLINE void Release()
	{
		if (Counter &&
			Counter->WeakReferences() == 0)
		{
			delete Counter;
		}

		if (ObjectPointer)
		{
			delete ObjectPointer;
		}
	}

	FORCEINLINE unsigned long GetReferenceCount() const
	{
		if (Counter)
		{
			return Counter->References();
		}

		return 0;
	}

	FORCEINLINE void SafeRelease()
	{
		if (Counter &&
			Counter->WeakReferences())
		{
			delete Counter;
		}

		Counter = NULL;

		if (ObjectPointer)
		{
			delete ObjectPointer;
		}

		ObjectPointer = NULL;
	}

	FORCEINLINE SharedPointer()
	{
		Counter = NULL;
	}

	FORCEINLINE SharedPointer
	(
		const SharedPointer<Typename> & Other
	)
	{
		Set(Other);
	}

	FORCEINLINE SharedPointer
	(
		const SharedPointer<Typename> && Other
	)
	{
		Set(Other);
	}

	FORCEINLINE SharedPointer
	(
		Typename * const Object
	)
	{
		ObjectPointer = Object;
		{
			Counter = new ReferenceCounter();
		}
	}

	FORCEINLINE ~SharedPointer()
	{
		RemoveReference();
	}

	template
	<
		typename Derived
	>
	FORCEINLINE SharedPointer<Derived> & As()
	{
		return reinterpret_cast<SharedPointer<Derived> &> (*this);
	}

	FORCEINLINE SharedPointer & operator=
	(
		Typename * const Object
	)
	{
		if (ObjectPointer)
		{
			RemoveReference();
		}

		ObjectPointer = Object;
		{
			Counter = new ReferenceCounter();
		}

		return *this;
	}

	FORCEINLINE SharedPointer & operator=
	(
		const SharedPointer<Typename> & Other
	)
	{
		Set(Other);
		return *this;
	}

	FORCEINLINE void AddReference()
	{
		Counter->Add();
	}

	FORCEINLINE void RemoveReference()
	{
		if (Counter &&
			Counter->Remove() == 0)
		{
			Release();
		}
	}

	using Pointer::operator();
	using Pointer::operator->;
	using Pointer::operator*;
};

template<class Typename> class ConstPointer : public Pointer<Typename>
{
public:

	using Pointer::operator();
	using Pointer::operator->;
	using Pointer::operator*;

	FORCEINLINE operator bool() const
	{
		return ObjectPointer != NULL;
	}

	FORCEINLINE operator Typename* () const
	{
		return ObjectPointer;
	}

	FORCEINLINE void operator=
	(
		const Typename * Object
	)
	{
		ObjectPointer = const_cast<Typename*>(Object);
	}

	FORCEINLINE void operator=
	(
		const SharedPointer<Typename> & Object
	)
	{
		ObjectPointer = const_cast<Typename*>(Object.Get());
	}

	FORCEINLINE void operator=
	(
		const UniquePointer<Typename> & Object
	)
	{
		ObjectPointer = const_cast<Typename*>(Object.Get());
	}

	FORCEINLINE ConstPointer()
	{}
	FORCEINLINE ConstPointer
	(
		const Typename * Object
	)
	{
		ObjectPointer = const_cast<Typename*>(Object);
	}
};

template<class Typename> class WeakPointer : protected Pointer<Typename>
{
private:

	ReferenceCounter * Counter;

public:

	FORCEINLINE operator bool() const
	{
		return ObjectPointer != NULL;
	}

	FORCEINLINE operator Typename* ()
	{
		return Get();
	}

	FORCEINLINE Typename* operator->()
	{
		return
			Counter &&
			Counter->References() ? ObjectPointer : NULL;
	}

	FORCEINLINE Typename* Get()
	{
		return 
			Counter && 
			Counter->References() ? ObjectPointer : NULL;
	}

	FORCEINLINE void operator=
	(
		const SharedPointer<Typename> & Object
	)
	{
		ObjectPointer = Object.ObjectPointer;
		{
			Counter = Object.Counter;
			Counter->AddWeak();
		}
	}

	FORCEINLINE void operator=
	(
		const WeakPointer<Typename> & Object
	)
	{
		ObjectPointer = Object.ObjectPointer;
		{
			Counter = Object.Counter;
			Counter->AddWeak();
		}
	}

	FORCEINLINE void operator=
	(
		decltype(nullptr) Null
	)
	{
		Counter = NULL;
	}

	FORCEINLINE WeakPointer()
	{
		Counter = NULL;
	}

	FORCEINLINE WeakPointer
	(
		const SharedPointer<Typename> & Object
	)
	{
		*this = Object;
	}

	FORCEINLINE WeakPointer
	(
		decltype(nullptr) Null
	)
	{
		ObjectPointer = NULL;
	}

	FORCEINLINE ~WeakPointer()
	{
		unsigned long Count = Counter->RemoveWeak();

		if (Counter &&
			Counter->References() == Count == 0)
		{
			delete Counter;
		}
	}
};

template<class Typename> class Reference : public SharedPointer<Typename>
{
public:

	FORCEINLINE Typename & GetReference()
	{
		return *ObjectPointer;
	}
};

template<class Typename> class ComPointer : public Pointer<Typename>
{
private:

	FORCEINLINE void Set
	(
		Typename * const Object
	)
	{
		if (ObjectPointer)
		{
			ObjectPointer->Release();
		}

		ObjectPointer = Object;
	}

public:

	using Pointer::operator();
	using Pointer::operator->;
	using Pointer::Pointer;

	FORCEINLINE void SafeRelease()
	{
		if (ObjectPointer)
		{
			ObjectPointer->Release();
			ObjectPointer = NULL;
		}
	}

	FORCEINLINE ComPointer() {}

	FORCEINLINE ComPointer
	(
		Typename * const Object
	)
	{
		ObjectPointer = Object;
	}

	FORCEINLINE void Swap
	(
		ComPointer & Other
	)
	{
		Typename * Tmp = ObjectPointer;
		ObjectPointer = Other.Get();
		Other = Tmp;
	}

	FORCEINLINE ComPointer
	(
		const ComPointer & Other
	)
	{
		ObjectPointer = Other.ObjectPointer;
		ObjectPointer->AddRef();
	}

	FORCEINLINE ComPointer
	(
		decltype (nullptr) Null
	) 
	{
	}

	FORCEINLINE ~ComPointer()
	{
		SafeRelease();
	}

	FORCEINLINE Typename * Detach()
	{
		Typename * Tmp = ObjectPointer;

		ObjectPointer = NULL;

		return Tmp;
	}

	FORCEINLINE void Attach
	(
		Typename * const Object
	)
	{
		Set(Object);
	}

	FORCEINLINE ComPointer & operator=
	(
		decltype (nullptr) Null
	)
	{
		SafeRelease();
		return *this;
	}

	FORCEINLINE ComPointer & operator=
	(
		Typename * const Object
	)
	{
		Set(Object);
		return *this;
	}
};