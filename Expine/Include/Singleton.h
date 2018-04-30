#pragma once

#include <assert.h>

template<typename Object> class CSingleton
{
private:

	static Object * ms_pObject;

public:

	/**********************************************
	*
	*	Returns Null if an instance already exists.
	*
	***********************************************/

	template
	<
		typename... Args
	>
	static __forceinline Object * New
	(
		Args&&... Arguments
	)
	{
		if (!ms_pObject)
		{
			return ms_pObject = new Object(Arguments...);
		}

		return nullptr;
	}

	static __forceinline Object & Instance()
	{
		return (*ms_pObject);
	}

	virtual __forceinline ~CSingleton()
	{
		ms_pObject = NULL;
	}

	static __forceinline Object * Instance_Pointer()
	{
		return (ms_pObject);
	}

	__forceinline CSingleton(CSingleton const&)				= delete;
	__forceinline CSingleton(CSingleton&&)					= delete;
	__forceinline CSingleton& operator=(CSingleton const&)	= delete;
	__forceinline CSingleton& operator=(CSingleton &&)		= delete;

protected:

	__forceinline CSingleton()
	{
		assert(ms_pObject == NULL);

		ms_pObject =
			reinterpret_cast<Object*>(
				reinterpret_cast<uintptr_t>(reinterpret_cast<Object*>				(this)) +
				reinterpret_cast<uintptr_t>(reinterpret_cast<Object*>				(1)) -
				reinterpret_cast<uintptr_t>(reinterpret_cast<CSingleton<Object>*>	(1)));
	}
};

template <typename Object> Object * CSingleton<Object>::ms_pObject = NULL;

template <typename Object> class CThreadLocal
{
private:

	static thread_local Object * ms_pObject;

public:

	/**********************************************
	*
	*	Returns Null if an instance already exists.
	*
	***********************************************/

	template
	<
		typename... Args
	>
	static __forceinline Object * New
	(
		Args&&... Arguments
	)
	{
		if (!ms_pObject)
		{
			return ms_pObject = new Object(Arguments...);
		}

		return nullptr;
	}

	static __forceinline Object & Instance()
	{
		return (*ms_pObject);
	}

	virtual __forceinline ~CThreadLocal()
	{
		ms_pObject = NULL;
	}

	static __forceinline Object * Instance_Pointer()
	{
		return (ms_pObject);
	}

	__forceinline CThreadLocal(CThreadLocal const&)				= delete;
	__forceinline CThreadLocal(CThreadLocal&&)					= delete;
	__forceinline CThreadLocal& operator=(CThreadLocal const&)	= delete;
	__forceinline CThreadLocal& operator=(CThreadLocal &&)		= delete;

protected:

	__forceinline CThreadLocal()
	{
		assert(ms_pObject == NULL);

		ms_pObject =
			reinterpret_cast<Object*>(
				reinterpret_cast<uintptr_t>(reinterpret_cast<Object*>				(this)) +
				reinterpret_cast<uintptr_t>(reinterpret_cast<Object*>				(1)) -
				reinterpret_cast<uintptr_t>(reinterpret_cast<CSingleton<Object>*>	(1)));
	}
};

template <typename Object> thread_local Object * CThreadLocal<Object>::ms_pObject = NULL;