#pragma once

#include "Defines.h"
#include <type_traits>
#include <Windows.h>

#include <tbb/scalable_allocator.h>

template<class Typename> class UniquePointer;
template<class Typename> class WeakPointer;
template<class Typename> class SharedPointer;
template<class Typename> class ComPointer;
template<class Typename> class Pointer
{
protected:

	mutable Typename * Pointer<Typename>::ObjectPointer = NULL;

public:

	FORCEINLINE Pointer
	(
		Typename * Object
	)
	{
		Pointer<Typename>::ObjectPointer = Object;
	}

	FORCEINLINE Pointer
	(
		decltype (nullptr) Null
	) noexcept
	{
	}

	FORCEINLINE Pointer() noexcept
	{}

	FORCEINLINE operator bool() const noexcept
	{
		return Pointer<Typename>::ObjectPointer != NULL;
	}

	FORCEINLINE Typename * operator->() const noexcept
	{
		return Pointer<Typename>::ObjectPointer;
	}

	FORCEINLINE Typename * operator()() const noexcept
	{
		return Pointer<Typename>::ObjectPointer;
	}

	FORCEINLINE bool Nil() const noexcept
	{
		return Pointer<Typename>::ObjectPointer == nullptr;
	}

	FORCEINLINE Typename * Get() const noexcept
	{
		return Pointer<Typename>::ObjectPointer;
	}

	FORCEINLINE Typename * Copy() const
	{
		return new Typename(*Pointer<Typename>::ObjectPointer);
	}

	FORCEINLINE const Typename & GetRef() const
	{
		return *Pointer<Typename>::ObjectPointer;
	}

	FORCEINLINE Typename ** operator&() noexcept
	{
		Typename * Object = Pointer<Typename>::ObjectPointer;
		{
			return &Object;
		}
	}

	FORCEINLINE Typename * operator*() const noexcept
	{
		return Pointer<Typename>::ObjectPointer;
	}

	FORCEINLINE bool operator==
	(
		const decltype(nullptr)
	)	const noexcept
	{
		return Pointer<Typename>::ObjectPointer == nullptr;
	}

	FORCEINLINE bool operator==
	(
		const decltype(NULL)
	)	const noexcept
	{
		return Pointer<Typename>::ObjectPointer == nullptr;
	}

	FORCEINLINE bool operator==
	(
		const Typename * Other
	)	const noexcept
	{
		return Pointer<Typename>::ObjectPointer == Other;
	}

	FORCEINLINE bool operator==
	(
		const Pointer<Typename> & Other
	)	const noexcept
	{
		return Pointer<Typename>::ObjectPointer == Other.ObjectPointer;
	}
};

template<class Typename> class UniquePointer : public Pointer<Typename>
{
public:

	using Pointer<Typename>::operator();
	using Pointer<Typename>::operator->;
	using Pointer<Typename>::Pointer;

	FORCEINLINE void SafeRelease()
	{
		if (Pointer<Typename>::ObjectPointer)
		{
			delete Pointer<Typename>::ObjectPointer;
			Pointer<Typename>::ObjectPointer = NULL;
		}
	}

	FORCEINLINE UniquePointer
	(
		const UniquePointer<Typename> & Other
	) noexcept
	{
		*this = Other.Detach();
	}

	FORCEINLINE UniquePointer()
	{}

	FORCEINLINE UniquePointer
	(
		UniquePointer<Typename> && Other
	) noexcept
	{
		*this = Other.Detach();
	}

	FORCEINLINE UniquePointer
	(
		Typename * const Object
	) noexcept
	{
		Pointer<Typename>::ObjectPointer = Object;
	}
	
	FORCEINLINE ~UniquePointer()
	{
		SafeRelease();
	}

	FORCEINLINE Typename * Detach() const noexcept
	{
		Typename * Tmp = Pointer<Typename>::ObjectPointer;

		Pointer<Typename>::ObjectPointer = NULL;

		return Tmp;
	}

	FORCEINLINE UniquePointer & operator=
	(
		Typename * const Object
	) noexcept
	{
		Pointer<Typename>::ObjectPointer = Object;
		return *this;
	}

	FORCEINLINE UniquePointer & operator=
	(
		const UniquePointer<Typename> & Other
	) noexcept
	{
		*this = Other.Detach();
		return *this;
	}
};

__declspec(align(8)) class ReferenceCounter
{
private:

	unsigned long ReferenceCount;
	unsigned long ReferenceCountWeak;

public:

	constexpr ReferenceCounter() : ReferenceCount(1), ReferenceCountWeak(1) {}

	FORCEINLINE unsigned long Add() noexcept
	{
		return InterlockedIncrement(&ReferenceCount);
	}

	FORCEINLINE unsigned long Remove() noexcept
	{
		return InterlockedDecrement(&ReferenceCount);
	}

	FORCEINLINE unsigned long AddWeak() noexcept
	{
		return InterlockedIncrement(&ReferenceCountWeak);
	}

	FORCEINLINE unsigned long RemoveWeak() noexcept
	{
		return InterlockedDecrement(&ReferenceCountWeak);
	}

	FORCEINLINE unsigned long WeakReferences() const noexcept
	{
		return ReferenceCountWeak;
	}

	FORCEINLINE unsigned long References() const noexcept
	{
		return ReferenceCount;
	}
};

template<class Typename>
__declspec(align(16)) struct RefCounterObject : public ReferenceCounter
{
	constexpr static size_t GetAlign()
	{
		if constexpr (sizeof(Typename) <= 8)
		{
			return 8;
		}
		else
		{
			return 16;
		}
	}

	FORCEINLINE RefCounterObject() noexcept {}
	FORCEINLINE RefCounterObject(RefCounterObject&& Other) = delete;
	FORCEINLINE RefCounterObject(const RefCounterObject& Other) = delete;
	
	template<class... Args>
	FORCEINLINE RefCounterObject(Args&&... Arguments)
	{
		new (&Storage) Typename(std::forward<Args>(Arguments)...);
	
	}
	FORCEINLINE Typename * Object() noexcept
	{
		return reinterpret_cast<Typename*>(&Storage);
	}

	FORCEINLINE void * operator new(const size_t Count)
	{
		return scalable_malloc(sizeof(RefCounterObject));
	}

	FORCEINLINE void operator delete(void * Where, const size_t Count)
	{
		scalable_free(Where);
	}
	
	std::aligned_storage_t<sizeof(Typename), GetAlign()> Storage;
};

template<class Typename> class SharedFromThis : public ReferenceCounter
{
protected:
	constexpr SharedFromThis() = default;

	SharedPointer<Typename> GetSharedPointer()
	{
		return SharedPointer<Typename>(this);
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
		
		Counter								= Other.Counter;
		Pointer<Typename>::ObjectPointer	= Other.ObjectPointer;

		if (Counter)
		{
			AddReference();
		}
	}

public:

	FORCEINLINE void Release()
	{
		if (reinterpret_cast<ptrdiff_t>(Pointer<Typename>::ObjectPointer) -
			reinterpret_cast<ptrdiff_t>(Counter) == sizeof(ReferenceCounter))
		{
			if (Counter->RemoveWeak() == 0)
			{
				scalable_free(Counter);
			}
		}
		else
		{
			if (Counter->RemoveWeak() == 0)
			{
				delete Counter;
			}

			delete Pointer<Typename>::ObjectPointer;
		}
	}

	FORCEINLINE unsigned long GetReferenceCount() const noexcept
	{
		return Counter->References();
	}

	FORCEINLINE void SafeRelease()
	{
		if (Counter)
		{
			if (reinterpret_cast<ptrdiff_t>(Pointer<Typename>::ObjectPointer) -
				reinterpret_cast<ptrdiff_t>(Counter) == sizeof(ReferenceCounter))
			{
				scalable_free(Counter);
			}
			else
			{
				delete Pointer<Typename>::ObjectPointer;
				delete Counter;
			}
			Counter = NULL;
			Pointer<Typename>::ObjectPointer = NULL;
		}
	}

	template<class... Args>
	FORCEINLINE void Construct(Args&&... Arguments) noexcept
	{
		RemoveReference();
		RefCounterObject<Typename> * CounterObject = new RefCounterObject<Typename>(std::forward<Args>(Arguments)...);
		Pointer<Typename>::ObjectPointer = CounterObject->Object();
		Counter = CounterObject;
	}

	FORCEINLINE SharedPointer() noexcept :
		Counter(NULL)
	{}

	FORCEINLINE SharedPointer
	(
		const SharedPointer<Typename> & Other
	)
	{
		Set(Other);
	}

	FORCEINLINE SharedPointer
	(
		decltype(nullptr) Null
	) 
		: Pointer<Typename>(Null)
		, Counter(NULL)
	{}

	FORCEINLINE SharedPointer
	(
		SharedPointer<Typename> && Other
	) noexcept
	{
		Counter = Other.Counter;
		Pointer<Typename>::ObjectPointer = Other.ObjectPointer;

		Other.Counter = nullptr;
		Other.ObjectPointer = nullptr;
	}

	FORCEINLINE SharedPointer
	(
		Typename * const Object
	) 
		: Pointer<Typename>(Object)
		, Counter(new ReferenceCounter())
	{}

	FORCEINLINE SharedPointer
	(
		SharedFromThis<Typename> * SFromThis
	) noexcept
		: Pointer<Typename>(static_cast<Typename*>(SFromThis))
		, Counter(SFromThis)
	{}

	template<class... Args>
	FORCEINLINE SharedPointer
	(
		Args&&... Arguments
	)
	{
		if constexpr (!std::is_constructible<Typename, Args...>::value)
		{
			Counter = NULL;
			Pointer<Typename>::ObjectPointer = NULL;
		}
		else
		{
			RefCounterObject<Typename> * CounterObject = new RefCounterObject<Typename>(std::forward<Args>(Arguments)...);
			Pointer<Typename>::ObjectPointer = CounterObject->Object();
			Counter = CounterObject;
		}
	}

	~SharedPointer()
	{
		RemoveReference();
	}

	template
	<
		typename Derived
	>
	FORCEINLINE SharedPointer<Derived> & As() noexcept
	{
		return reinterpret_cast<SharedPointer<Derived> &> (*this);
	}

	FORCEINLINE SharedPointer & operator=
	(
		Typename * const Object
	)
	{
		if (Pointer<Typename>::ObjectPointer)
		{
			RemoveReference();
		}

		Pointer<Typename>::ObjectPointer = Object;
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

	FORCEINLINE void AddReference() noexcept
	{
		Counter->Add();
		Counter->AddWeak();
	}

	FORCEINLINE void RemoveReference() 
	{
		if (Counter && Counter->Remove() == 0)
		{
			Release();
		}
	}

	using Pointer<Typename>::operator();
	using Pointer<Typename>::operator->;
	using Pointer<Typename>::operator*;
};

template<class Typename> class ConstPointer : public Pointer<Typename>
{
public:

	using Pointer<Typename>::operator();
	using Pointer<Typename>::operator->;
	using Pointer<Typename>::operator*;

	FORCEINLINE operator bool() const noexcept
	{
		return Pointer<Typename>::ObjectPointer != NULL;
	}

	FORCEINLINE operator Typename* () const noexcept
	{
		return Pointer<Typename>::ObjectPointer;
	}

	FORCEINLINE void operator=
	(
		const Typename * Object
	) noexcept
	{
		Pointer<Typename>::ObjectPointer = const_cast<Typename*>(Object);
	}

	FORCEINLINE void operator=
	(
		const SharedPointer<Typename> & Object
	) noexcept
	{
		Pointer<Typename>::ObjectPointer = const_cast<Typename*>(Object.Get());
	}

	FORCEINLINE void operator=
	(
		const UniquePointer<Typename> & Object
	) noexcept
	{
		Pointer<Typename>::ObjectPointer = const_cast<Typename*>(Object.Get());
	}

	FORCEINLINE ConstPointer() noexcept
	{}
	FORCEINLINE ConstPointer
	(
		const Typename * Object
	) noexcept
	{
		Pointer<Typename>::ObjectPointer = const_cast<Typename*>(Object);
	}
};

template<class Typename> class WeakPointer : protected Pointer<Typename>
{
private:

	ReferenceCounter * Counter;

public:

	FORCEINLINE operator bool() const noexcept
	{
		return Pointer<Typename>::ObjectPointer != NULL;
	}

	FORCEINLINE operator Typename* () noexcept
	{
		return Get();
	}

	FORCEINLINE Typename* operator->() noexcept
	{
		return
			Counter &&
			Counter->References() ? Pointer<Typename>::ObjectPointer : NULL;
	}

	FORCEINLINE Typename* Get() noexcept
	{
		return 
			Counter && 
			Counter->References() ? Pointer<Typename>::ObjectPointer : NULL;
	}

	FORCEINLINE void operator=
	(
		const SharedPointer<Typename> & Object
	)
	{
		Pointer<Typename>::ObjectPointer = Object.Pointer<Typename>::ObjectPointer;
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
		Pointer<Typename>::ObjectPointer = Object.Pointer<Typename>::ObjectPointer;
		{
			Counter = Object.Counter;
			Counter->AddWeak();
		}
	}

	FORCEINLINE void operator=
	(
		decltype(nullptr) Null
	) noexcept
	{
		Counter = NULL;
	}

	FORCEINLINE WeakPointer() noexcept
	{
		Counter = NULL;
	}

	FORCEINLINE WeakPointer
	(
		const SharedPointer<Typename> & Object
	) noexcept
	{
		*this = Object;
	}

	FORCEINLINE WeakPointer
	(
		decltype(nullptr) Null
	) noexcept :
		Pointer<Typename>(Null)
	{
		Counter = NULL;
	}

	FORCEINLINE ~WeakPointer()
	{
		if (Counter->RemoveWeak() == 0)
		{
			if (reinterpret_cast<ptrdiff_t>(Pointer<Typename>::ObjectPointer) -
				reinterpret_cast<ptrdiff_t>(Counter) == sizeof(ReferenceCounter))
			{
				scalable_free(Counter);
			}
			else
			{
				delete Counter;
			}
		}
	}
};

template<class Typename> class Reference : public SharedPointer<Typename>
{
public:

	FORCEINLINE Typename & GetReference()
	{
		return *Pointer<Typename>::ObjectPointer;
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
		if (Pointer<Typename>::ObjectPointer)
		{
			Pointer<Typename>::ObjectPointer->Release();
		}

		Pointer<Typename>::ObjectPointer = Object;
	}

public:

	using Pointer<Typename>::operator();
	using Pointer<Typename>::operator->;
	using Pointer<Typename>::Pointer;

	FORCEINLINE void SafeRelease()
	{
		if (Pointer<Typename>::ObjectPointer)
		{
			Pointer<Typename>::ObjectPointer->Release();
			Pointer<Typename>::ObjectPointer = NULL;
		}
	}

	FORCEINLINE ComPointer() {}

	FORCEINLINE ComPointer
	(
		Typename * const Object
	)
	{
		Pointer<Typename>::ObjectPointer = Object;
	}

	FORCEINLINE void Swap
	(
		ComPointer & Other
	)
	{
		Typename * Tmp = Pointer<Typename>::ObjectPointer;
		Pointer<Typename>::ObjectPointer = Other.Get();
		Other = Tmp;
	}

	FORCEINLINE ComPointer
	(
		const ComPointer & Other
	)
	{
		Pointer<Typename>::ObjectPointer = Other.ObjectPointer;
		Pointer<Typename>::ObjectPointer->AddRef();
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
		Typename * Tmp = Pointer<Typename>::ObjectPointer;

		Pointer<Typename>::ObjectPointer = NULL;

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