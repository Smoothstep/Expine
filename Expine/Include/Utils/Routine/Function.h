#pragma once

#include <tuple>

#include "Memory.h"

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>

/*----------------------------------------------------------------
	Fast function container.
----------------------------------------------------------------*/

class CFunction
{
private:

	template
	<
		int ...
	>
	struct seq 
	{};

	/*----------------------------------------------------------------*/

	template
	<
		int		N, 
		int ... S
	>
	struct gens : gens<N - 1, N - 1, S...> 
	{};

	/*----------------------------------------------------------------*/

	template
	<
		int ... S
	>
	struct gens<0, S ...>
	{
		typedef seq<S...> type;
	};

	/*----------------------------------------------------------------*/

	template 
	<
		typename T
	>
	struct Identity
	{
		typedef T type;
	};

	class CFunctionWrapper
	{
	public:

		virtual inline  ~CFunctionWrapper() {}

	public:

		virtual inline void Run() const = 0;
		virtual inline void operator()() const
		{
			Run();
		}
	};

	template
	<
		typename	 Return, 
		typename ... Args
	>
	class CFunctionTypes
	{
	public:

		typedef std::function<Return(Args...)>	TFunction;
		typedef std::tuple<Args...>				TTuple;
	};

	template
	<
		typename	 Return, 
		typename ... Args
	>
	class CFunctionHolder : public CFunctionWrapper, CFunctionTypes<Return, Args...>
	{
	public:

		TTuple		Tuple;
		TFunction	Function;

	public:

		inline CFunctionHolder
		(
			TFunction &	Bound,
			Args &&...	Arguments
		) :
			Tuple(TTuple(Arguments...)), Function(Bound)
		{}

		inline CFunctionHolder
		(
			Return		(*Bound)(Args &&...),
			Args &&...	Arguments
		) :
			Tuple(TTuple(Arguments...)), Function(Bound)
		{}

		inline CFunctionHolder
		(
			Return*		Bound,
			Args&&...	Arguments
		) :
			Tuple(TTuple(Arguments...)), Function(Bound)
		{}

	public:

		virtual inline void Run() const override
		{
			Dispatch(typename gens<sizeof...(Args)>::type());
		}

	private:

		template
		<
			int ... S
		> 
		inline void Dispatch
		(
			const seq<S...>
		)	const
		{
			Function(std::get<S>(Tuple) ...);
		}
	};

	template
	<
		typename	 Return, 
		typename	 Class, 
		typename ... Args
	>
	class CMemberFunctionHolder : public CFunctionWrapper, CFunctionTypes<Return, Args...>
	{
	public:

		TTuple		Tuple;
		TFunction	Function;

		class TMemberFunction
		{
		public:
			explicit TMemberFunction
			(
				Return(Class::*_Pm)(Args...)
			)
				: _Pmemfun(_Pm)
			{}

			Return operator()(Class *_Pleft, Args... _Right) const
			{
				return ((_Pleft->*_Pmemfun)(_Right...));
			}

		private:
			Return(Class::*_Pmemfun)(Args...);
		};

	public:

		inline CMemberFunctionHolder
		(
			Return				(Class::*MemberFunction)(Args&&...),
			Class		*		Instance,
			Args		&& ...	Arguments
		) :
			Tuple(TTuple(Arguments...)),
			Function(std::bind(MemberFunction, Instance, Arguments...))
		{}

	public:

		virtual void Run() const override
		{
			Dispatch(typename gens<sizeof...(Args)>::type());
		}

	private:

		template
		<
			int ... S
		> 
		inline void Dispatch
		(
			const seq<S...>
		)	const
		{
			Function(std::get<S>(Tuple) ...);
		}
	};

	class CServiceFunction : public CFunctionWrapper
	{
	private:

		boost::detail::thread_data_ptr Context;

	public:

		template
		<
			typename Signature
		>
		inline void Initialize
		(
			BOOST_THREAD_RV_REF(Signature) Bound
		)
		{
			Context = GetContext(boost::thread_detail::decay_copy(boost::forward<Signature>(Bound)));
		}

		template
		<
			typename Signature
		>
		explicit inline CServiceFunction
		(
			BOOST_THREAD_RV_REF(Signature) Bound
		)
		{
			Initialize(Bound);
		}

		template
		<
			class		Return, 
			class ...	Args
		>
		explicit inline CServiceFunction
		(
			Return		Function, 
			Args && ... Arguments
		)
		{
			Initialize(boost::bind(Function, std::forward<Args>(Arguments)...));
		}

		inline virtual ~CServiceFunction()
		{
		}

		template
		<
			class Signature
		>
		explicit CServiceFunction
		(
			boost::thread::attributes		&   Attributes, 
			BOOST_THREAD_RV_REF(Signature)		Bound
		) 
		{
			Context = boost::thread_detail::decay_copy(boost::forward<Signature>(GetContext(Bound)));
		}

		template
		<
			typename Signature
		>
		static inline boost::detail::thread_data_ptr GetContext
		(
			BOOST_THREAD_RV_REF(Signature) Bound
		)
		{
			return boost::detail::thread_data_ptr
			(
				boost::detail::heap_new<boost::detail::thread_data<typename boost::remove_reference<Signature>::type> >
				(
					boost::forward<Signature>(Bound)
				)
			);
		}

		inline void Run() const override
		{
			Context->run();
		}
	};

	CFunctionWrapper* FunctionWrapper;

public:

	~CFunction()
	{
		SafeRelease(FunctionWrapper);
	}

	CFunction()
	{
		FunctionWrapper = NULL;
	}


	/*----------------------------------------------------------------------------------------------------------------------------------
		Constructs a function wrapper with a specific signature. (ForEx: CFunction(Run))
	----------------------------------------------------------------------------------------------------------------------------------*/

	template
	<
		typename Signature
	>
	inline CFunction
	(
		BOOST_THREAD_RV_REF(Signature) Bound
	)
	{
		AssignFunction(Bound);
	}


	/*----------------------------------------------------------------------------------------------------------------------------------
		Constructs a function wrapper. (ForEx: CFunction(Run))
	----------------------------------------------------------------------------------------------------------------------------------*/

	template
	<
		typename	 Return, 
		typename ... Args
	>
	inline CFunction
	(
		Return			(*Function)(Args&&...),
		Args	&&...	Arguments
	)
	{
		AssignFunction(Function, Arguments...);
	}


	/*----------------------------------------------------------------------------------------------------------------------------------
	Constructs a function wrapper. (ForEx: CFunction(Run))
	----------------------------------------------------------------------------------------------------------------------------------*/

	template
	<
		typename	 Return, 
		typename ... Args
	>
	inline CFunction
	(
		Return		(*Function)(Args...),
		Args	...	Arguments
	)
	{
		AssignFunction(Function, Arguments...);
	}


	/*----------------------------------------------------------------------------------------------------------------------------------
		Constructs a void function lambda wrapper.
	----------------------------------------------------------------------------------------------------------------------------------*/

	template
	<
		typename ... Args
	>
	inline CFunction
	(
		typename Identity<std::function<void(Args&&...)>>::type Bound,
		Args&&...	Arguments
	)
	{
		AssignFunction(Bound, Arguments...);
	}


	/*----------------------------------------------------------------------------------------------------------------------------------
		Constructs a member function wrapper. (ForEx: CFunction(CFunction::Run, &Other))
	----------------------------------------------------------------------------------------------------------------------------------*/

	template
	<
		typename	Return, 
		typename	Class, 
		typename... Args
	> 
	inline CFunction
	(
		Return			(Class::*MemberFunction)(Args && ...),
		Class	*		Instance,
		Args	&& ...	Arguments
	)
	{
		AssignFunction(MemberFunction, Instance, Arguments...);
	}


	/*----------------------------------------------------------------------------------------------------------------------------------
		Assigns a member function wrapper. (ForEx: AssignFunction(CFunction::Run, &Other))
	----------------------------------------------------------------------------------------------------------------------------------*/

	template
	<
		typename	 Return, 
		typename	 Class, 
		typename ... Args
	> 
	inline void AssignFunction
	(
		Return			(Class::*MemberFunction)(Args&&...),
		Class	*		Instance,
		Args	&& ...	Arguments
	)
	{
		FunctionWrapper = new CMemberFunctionHolder<Return, Class, Args...>(MemberFunction, Instance, Arguments...);
	}


	/*----------------------------------------------------------------------------------------------------------------------------------
		Assigns a function with a specific signature. (ForEx: AssignFunction(Run))
	----------------------------------------------------------------------------------------------------------------------------------*/

	template
	<
		typename Signature
	> 
	inline void AssignFunction
	(
		BOOST_THREAD_RV_REF(Signature) Bound
	)
	{
		FunctionWrapper = new CServiceFunction(STATIC_CAST<BOOST_THREAD_RV_REF(Signature)>(Bound));
	}


	/*----------------------------------------------------------------------------------------------------------------------------------
		Assigns a void lambda function.
	----------------------------------------------------------------------------------------------------------------------------------*/

	template
	<
		typename ... Args
	>
	inline void AssignFunction
	(
		typename Identity<std::function<void(Args &&...)> >::type Bound,
		Args &&... Arguments
	)
	{
		FunctionWrapper = new CFunctionHolder<void, Args...>(Bound, Arguments...);
	}


	/*----------------------------------------------------------------------------------------------------------------------------------
		Assigns a function with a specific Function and argument specification. (ForEx: AssignFunction(Run, 1))
	----------------------------------------------------------------------------------------------------------------------------------*/

	template
	<
		typename	 Return,
		typename ... Args
	>
	inline void AssignFunction
	(
		Return (*Function)(Args...),
		Args... Arguments
	)
	{
		FunctionWrapper = new CFunctionHolder<Return, Args...>(Function, Arguments...);
	}


	/*----------------------------------------------------------------------------------------------------------------------------------
		Executes the function if not null.
	----------------------------------------------------------------------------------------------------------------------------------*/

	inline void Run() const
	{
		if (FunctionWrapper)
		{
			FunctionWrapper->Run();
		}
	}


	/*----------------------------------------------------------------------------------------------------------------------------------
		Executes the function if not null.
	----------------------------------------------------------------------------------------------------------------------------------*/

	inline void operator()() const
	{
		Run();
	}
};