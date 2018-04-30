#pragma once

#include <boost/chrono.hpp>

namespace Time
{
	using Milliseconds = boost::chrono::milliseconds;
	using Microseconds = boost::chrono::microseconds;
	using Nanoseconds  = boost::chrono::nanoseconds;

	template<typename T> class UTimePoint
	{
	private:
		T TimePoint;

	public:
		UTimePoint() = default;
		UTimePoint(const T& TimePoint) :
			TimePoint(TimePoint)
		{}

		int64_t operator-(const UTimePoint& Other) const
		{
			return (TimePoint - Other.TimePoint).count();
		}

		template
		<
			typename Type, std::enable_if_t<Microseconds::period::den == Type::period::den, int64_t> = 0
		>
		int64_t operator-(const UTimePoint& Other) const
		{
			return (TimePoint - Other.TimePoint).count();
		}

		template
		<
			typename Type, std::enable_if_t<Milliseconds::period::den == Type::period::den, int64_t> = 0
		>
		int64_t operator-(const UTimePoint& Other) const
		{
			return (TimePoint - Other.TimePoint).count() / 1000;
		}

		inline void operator+=(const Milliseconds& MilS)
		{
			TimePoint += Nanoseconds(MilS);
		}

		inline void operator+=(const Microseconds& MicS)
		{
			TimePoint += Nanoseconds(MicS);
		}

		inline void operator+=(const Nanoseconds& NanS)
		{
			TimePoint += NanS;
		}

		inline bool operator<(const UTimePoint& Other) const
		{
			return this->operator-<Microseconds>(Other) < 0;
		}

		inline bool operator>(const UTimePoint& Other) const
		{
			return this->operator-<Microseconds>(Other) > 0;
		}
	};

	using TimePointSystem = UTimePoint<boost::chrono::system_clock::time_point>;
	using TimePointSteady = UTimePoint<boost::chrono::steady_clock::time_point>;

	class UHighResolutionClock
	{
	private:
		using TClock = boost::chrono::high_resolution_clock;
		
	public:
		static UTimePoint<TClock::time_point> Now()
		{
			return UTimePoint<TClock::time_point>(TClock::now());
		}
	};
}