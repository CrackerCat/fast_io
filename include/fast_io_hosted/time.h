#pragma once

namespace fast_io
{

namespace details
{
#if __has_cpp_attribute(gnu::pure)
[[gnu::pure]]
#endif
inline constexpr auto posix_clock_id_to_native_value(posix_clock_id pcid)
{
#ifdef _WIN32
	switch(pcid)
	{
	case posix_clock_id::realtime:
	case posix_clock_id::tai:
	return posix_clock_id::tai;
	break;
	case posix_clock_id::monotonic:
	return posix_clock_id::monotonic;
	break;
	default:
		throw_win32_error(0x00000057);
	};
#else
	switch(pcid)
	{
#ifdef CLOCK_REALTIME
	case posix_clock_id::realtime:
	return CLOCK_REALTIME;
	break;
#endif
#ifdef CLOCK_REALTIME_ALARM
	case posix_clock_id::realtime_alarm:
	return CLOCK_REALTIME_ALARM;
	break;
#elif defined(CLOCK_REALTIME)
	case posix_clock_id::realtime_alarm:
	return CLOCK_REALTIME;
	break;
#endif
#ifdef CLOCK_REALTIME_COARSE
	case posix_clock_id::realtime_coarse:
	return CLOCK_REALTIME_COARSE;
	break;
#elif defined(CLOCK_REALTIME)
	case posix_clock_id::realtime_coarse:
	return CLOCK_REALTIME;
	break;
#endif
#ifdef CLOCK_TAI
	case posix_clock_id::tai:
	return CLOCK_TAI;
	break;
#elif defined(CLOCK_REALTIME)
	case posix_clock_id::tai:
	return CLOCK_REALTIME;
	break;
#endif
#ifdef CLOCK_MONOTONIC
	case posix_clock_id::monotonic:
	return CLOCK_MONOTONIC;
	break;
#endif
#ifdef CLOCK_MONOTONIC_COARSE
	case posix_clock_id::monotonic_coarse:
	return CLOCK_MONOTONIC_COARSE;
	break;
#endif
#ifdef CLOCK_MONOTONIC_RAW
	case posix_clock_id::monotonic_raw:
	return CLOCK_MONOTONIC_RAW;
	break;
#endif
#ifdef CLOCK_MONOTONIC_RAW_APPROX
	case posix_clock_id::monotonic_raw_approx:
	return CLOCK_MONOTONIC_RAW_APPROX;
	break;
#endif
#ifdef CLOCK_BOOTTIME
	case posix_clock_id::boottime:
	return CLOCK_BOOTTIME;
	break;
#else
#ifdef CLOCK_MONOTONIC
	case posix_clock_id::boottime:
	return CLOCK_MONOTONIC;
	break;
#endif
#endif
#ifdef CLOCK_BOOTTIME_ALARM
	case posix_clock_id::boottime_alarm:
	return CLOCK_BOOTTIME_ALARM;
	break;
#endif
#ifdef CLOCK_UPTIME_RAW
	case posix_clock_id::uptime_raw:
	return CLOCK_UPTIME_RAW;
	break;
#endif
#ifdef CLOCK_UPTIME_RAW_APPROX
	case posix_clock_id::uptime_raw_approx:
	return CLOCK_UPTIME_RAW_APPROX;
	break;
#endif
#ifdef CLOCK_PROCESS_CPUTIME_ID
	case posix_clock_id::process_cputime_id:
	return CLOCK_PROCESS_CPUTIME_ID;
	break;
#endif
#ifdef CLOCK_THREAD_CPUTIME_ID
	case posix_clock_id::thread_cputime_id:
	return CLOCK_THREAD_CPUTIME_ID;
	break;
#endif
	default:
		throw_posix_error(EINVAL);
	};
#endif
}
#ifdef _WIN32
#if __has_cpp_attribute(gnu::pure)
[[gnu::pure]]
#endif
inline std::int64_t win32_query_performance_frequency()
{
	std::int64_t val{};
	if(!::fast_io::win32::QueryPerformanceFrequency(__builtin_addressof(val)))
		throw_win32_error();
	if(val<=0)
		throw_win32_error(0x00000057);
	return val;
}

#if __has_cpp_attribute(gnu::pure)
[[gnu::pure]]
#endif
inline unix_timestamp win32_query_performance_frequency_to_unix_timestamp()
{
	return {0,uintiso_subseconds_per_second/static_cast<std::uint64_t>(win32_query_performance_frequency())};
}
#endif
}

inline
#if defined(_WIN32) || defined(__MSDOS__)
constexpr
#endif
unix_timestamp posix_clock_getres([[maybe_unused]] posix_clock_id pclk_id)
{
#ifdef _WIN32
	switch(pclk_id)
	{
	case posix_clock_id::realtime:
	case posix_clock_id::realtime_alarm:
	case posix_clock_id::realtime_coarse:
	case posix_clock_id::tai:
	case posix_clock_id::process_cputime_id:
	case posix_clock_id::thread_cputime_id:
	{
		constexpr uintiso_t mul_factor{uintiso_subseconds_per_second/10000000u};
		constexpr unix_timestamp constexpr_stamp{0,mul_factor};
		return constexpr_stamp;
	}
	case posix_clock_id::monotonic:
	case posix_clock_id::monotonic_coarse:
	case posix_clock_id::monotonic_raw:
	case posix_clock_id::boottime:
		return ::fast_io::details::win32_query_performance_frequency_to_unix_timestamp();
	break;
	default:
		throw_win32_error(0x00000057);
	}
#elif defined(__MSDOS__)
	switch(pclk_id)
	{
	case posix_clock_id::realtime:
	case posix_clock_id::realtime_alarm:
	case posix_clock_id::realtime_coarse:
	case posix_clock_id::tai:
	case posix_clock_id::monotonic:
	case posix_clock_id::monotonic_coarse:
	case posix_clock_id::monotonic_raw:
	case posix_clock_id::boottime:
	{
		constexpr uintiso_t mul_factor{uintiso_subseconds_per_second/100u};
		return {0,mul_factor};
	}
	case posix_clock_id::process_cputime_id:
	case posix_clock_id::thread_cputime_id:
	{
		constexpr uintiso_t mul_factor{uintiso_subseconds_per_second/UCLOCKS_PER_SEC};
		return {0,mul_factor};
	}
	default:
		throw_posix_error(EINVAL);
	};
#elif !defined(__NEWLIB__) || defined(_POSIX_TIMERS)
	auto clk{details::posix_clock_id_to_native_value(pclk_id)};
	struct timespec res;
//vdso
	if(::clock_getres(clk,__builtin_addressof(res))<0)
		throw_posix_error();
	constexpr uintiso_t mul_factor{uintiso_subseconds_per_second/1000000000u};
	return {static_cast<intiso_t>(res.tv_sec),static_cast<uintiso_t>(res.tv_nsec)*mul_factor};
#else
	throw_posix_error(EINVAL);
#endif
}
#ifdef _WIN32
namespace win32::details
{

inline unix_timestamp win32_posix_clock_gettime_tai_impl() noexcept
{
	::fast_io::win32::filetime ftm;
#if (defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0602) && !defined(_WIN32_WINDOWS)
	::fast_io::win32::GetSystemTimePreciseAsFileTime(__builtin_addressof(ftm));
#else
	::fast_io::win32::GetSystemTimeAsFileTime(__builtin_addressof(ftm));
#endif
	return static_cast<unix_timestamp>(to_win32_timestamp(ftm));
}

inline unix_timestamp win32_posix_clock_gettime_boottime_impl()
{
	std::uint64_t ftm{};
	if(!::fast_io::win32::QueryUnbiasedInterruptTime(__builtin_addressof(ftm)))
		throw_win32_error();
	constexpr uintiso_t mul_factor{uintiso_subseconds_per_second/10000000u};
	std::uint64_t seconds{ftm/10000000ULL};
	std::uint64_t subseconds{ftm%10000000ULL};
	return {static_cast<intiso_t>(seconds),static_cast<uintiso_t>(subseconds*mul_factor)};
}

template<bool is_thread>
inline unix_timestamp win32_posix_clock_gettime_process_or_thread_time_impl()
{
	::fast_io::win32::filetime creation_time,exit_time,kernel_time,user_time;
	if constexpr(is_thread)
	{
		auto hthread{bit_cast<void*>(std::intptr_t(-2))};
		if(!::fast_io::win32::GetThreadTimes(hthread,__builtin_addressof(creation_time),
		__builtin_addressof(exit_time),
		__builtin_addressof(kernel_time),
		__builtin_addressof(user_time)))
			throw_win32_error();
	}
	else
	{
		auto hprocess{bit_cast<void*>(std::intptr_t(-1))};
		if(!::fast_io::win32::GetProcessTimes(hprocess,__builtin_addressof(creation_time),
		__builtin_addressof(exit_time),
		__builtin_addressof(kernel_time),
		__builtin_addressof(user_time)))
			throw_win32_error();
	}
	std::uint64_t ftm{::fast_io::win32::filetime_to_uint64_t(kernel_time)+
		::fast_io::win32::filetime_to_uint64_t(user_time)};
	std::uint64_t seconds{ftm/10000000ULL};
	std::uint64_t subseconds{ftm%10000000ULL};
	constexpr uintiso_t mul_factor{uintiso_subseconds_per_second/10000000u};
	return {static_cast<intiso_t>(seconds),static_cast<uintiso_t>(subseconds*mul_factor)};
}

inline unix_timestamp win32_posix_clock_gettime_boottime_xp_impl()
{
	std::int64_t freq{::fast_io::details::win32_query_performance_frequency()};
	std::int64_t counter{};
	if(!::fast_io::win32::QueryPerformanceCounter(__builtin_addressof(counter)))
		throw_win32_error();
	std::int64_t val(uintiso_subseconds_per_second/freq);
	std::int64_t dv{counter/freq};
	std::uint64_t md{static_cast<std::uint64_t>(counter%freq)};
	return unix_timestamp{dv,md*val};
}

}
#endif

#ifdef __MSDOS__
namespace details
{

struct my_dos_date_t
{
	std::uint8_t day;
	std::uint8_t month;
	std::uint16_t year;
	std::uint8_t dayofweek;
};

struct my_dos_time_t
{
	std::uint8_t hour;     /* 0-23 */
	std::uint8_t minute;   /* 0-59 */
	std::uint8_t second;   /* 0-59 */
	std::uint8_t hsecond;  /* 0-99 */
};

extern void my_dos_getdate(my_dos_date_t*) noexcept __asm__("__dos_getdate");
extern void my_dos_gettime(my_dos_time_t*) noexcept __asm__("__dos_gettime");

extern unsigned int my_dos_settime(my_dos_time_t*) noexcept __asm__("__dos_settime");
extern unsigned int my_dos_setdate(my_dos_date_t*) noexcept __asm__("__dos_setdate");

inline iso8601_timestamp get_dos_iso8601_timestamp() noexcept
{
	my_dos_date_t dos_date;
	my_dos_time_t dos_time;
	my_dos_date_t dos_date_temp;
	for(std::size_t i{};i!=100;++i)
	{
		my_dos_getdate(&dos_date);
		my_dos_gettime(&dos_time);
		my_dos_getdate(&dos_date_temp);
		if(dos_date.day==dos_date_temp.day&&
		dos_date.month==dos_date_temp.month&&
		dos_date.year==dos_date_temp.year&&
		dos_date.dayofweek==dos_date_temp.dayofweek)
		{
			constexpr uintiso_t factor{uintiso_subseconds_per_second/100u};
			return {static_cast<intiso_t>(dos_date.year),dos_date.month,dos_date.day,
				dos_time.hour,dos_time.minute,dos_time.second,
				static_cast<uintiso_t>(dos_time.hsecond)*factor,0};
		}
	}
	fast_terminate();
}

inline unix_timestamp get_dos_unix_timestamp() noexcept
{
	return to_timestamp(get_dos_iso8601_timestamp());
}

inline void set_dos_unix_timestamp(unix_timestamp tsp)
{
	iso8601_timestamp iso8601{utc(tsp)};
	if(iso8601.year>static_cast<intiso_t>(UINT16_MAX)||iso8601.year<0)
		throw_posix_error(EINVAL);
	std::uint16_t year{static_cast<std::uint16_t>(iso8601.year)};
	my_dos_date_t dos_date{static_cast<std::uint8_t>(iso8601.day),static_cast<std::uint8_t>(iso8601.month),year,0};
	constexpr uintiso_t precision{uintiso_subseconds_per_second/100ULL};
	my_dos_time_t dos_time{static_cast<std::uint8_t>(iso8601.hours),
		static_cast<std::uint8_t>(iso8601.minutes),
		static_cast<std::uint8_t>(iso8601.seconds),
		static_cast<std::uint8_t>(iso8601.subseconds/precision)};
	if(!my_dos_setdate(&dos_date))
		throw_posix_error();
	if(!my_dos_settime(&dos_time))
		throw_posix_error();
	my_dos_date_t dos_date_temp;
	for(std::size_t i{};i!=100;++i)
	{
		my_dos_getdate(&dos_date_temp);
		if(dos_date.day==dos_date_temp.day&&
		dos_date.month==dos_date_temp.month&&
		dos_date.year==dos_date_temp.year&&
		dos_date.dayofweek==dos_date_temp.dayofweek)
			return;
		if(!my_dos_setdate(&dos_date))
			throw_posix_error();
		if(!my_dos_settime(&dos_time))
			throw_posix_error();
	}
	throw_posix_error(EINVAL);
}

}
#endif

#if 0

inline unix_timestamp dos_posix_clock_gettime([[maybe_unused]] posix_clock_id pclk_id)
{
	switch(pclk_id)
	{
	case posix_clock_id::realtime:
	case posix_clock_id::realtime_alarm:
	case posix_clock_id::realtime_coarse:
	case posix_clock_id::tai:
	case posix_clock_id::monotonic:
	case posix_clock_id::monotonic_coarse:
	case posix_clock_id::monotonic_raw:
	case posix_clock_id::boottime:
	{
		timeval tv;
		constexpr uintiso_t mul_factor{uintiso_subseconds_per_second/1000000u};
		if(::gettimeofday(__builtin_addressof(tv), nullptr)<0)
			throw_posix_error();
		return {static_cast<intiso_t>(tv.tv_sec),static_cast<uintiso_t>(tv.tv_usec)*mul_factor};
	}
	case posix_clock_id::process_cputime_id:
	case posix_clock_id::thread_cputime_id:
	{
		std::make_unsigned_t<decltype(uclock())> u(uclock());
		uintiso_t seconds(u/UCLOCKS_PER_SEC);
		uintiso_t subseconds(u%UCLOCKS_PER_SEC);
		constexpr uintiso_t mul_factor{uintiso_subseconds_per_second/UCLOCKS_PER_SEC};
		return {static_cast<intiso_t>(seconds),static_cast<uintiso_t>(subseconds)*mul_factor};
	}
	default:
		throw_posix_error(EINVAL);
	}
}

#endif

inline unix_timestamp posix_clock_gettime([[maybe_unused]] posix_clock_id pclk_id)
{
#ifdef _WIN32
	switch(pclk_id)
	{
	case posix_clock_id::realtime:
	case posix_clock_id::realtime_alarm:
	case posix_clock_id::realtime_coarse:
	case posix_clock_id::tai:
		return win32::details::win32_posix_clock_gettime_tai_impl();
	case posix_clock_id::monotonic:
	case posix_clock_id::monotonic_coarse:
	case posix_clock_id::monotonic_raw:
	case posix_clock_id::boottime:
//		return win32::details::win32_posix_clock_gettime_boottime_impl();
		return win32::details::win32_posix_clock_gettime_boottime_xp_impl();
	case posix_clock_id::process_cputime_id:
		return win32::details::win32_posix_clock_gettime_process_or_thread_time_impl<false>();
	case posix_clock_id::thread_cputime_id:
		return win32::details::win32_posix_clock_gettime_process_or_thread_time_impl<true>();
	default:
		throw_win32_error(0x00000057);
	}
#elif defined(__MSDOS__)
	switch(pclk_id)
	{
	case posix_clock_id::realtime:
	case posix_clock_id::realtime_alarm:
	case posix_clock_id::realtime_coarse:
	case posix_clock_id::tai:
	case posix_clock_id::monotonic:
	case posix_clock_id::monotonic_coarse:
	case posix_clock_id::monotonic_raw:
	case posix_clock_id::boottime:
	{
		return details::get_dos_unix_timestamp();
	}
	case posix_clock_id::process_cputime_id:
	case posix_clock_id::thread_cputime_id:
	{
		std::make_unsigned_t<decltype(uclock())> u(uclock());
		uintiso_t seconds(u/UCLOCKS_PER_SEC);
		uintiso_t subseconds(u%UCLOCKS_PER_SEC);
		constexpr uintiso_t mul_factor{uintiso_subseconds_per_second/UCLOCKS_PER_SEC};
		return {static_cast<intiso_t>(seconds),static_cast<uintiso_t>(subseconds)*mul_factor};
	}
	default:
		throw_posix_error(EINVAL);
	}
#elif !defined(__NEWLIB__) || defined(_POSIX_TIMERS)
	struct timespec res;
	auto clk{details::posix_clock_id_to_native_value(pclk_id)};
//vdso
	if(::clock_gettime(clk,__builtin_addressof(res))<0)
		throw_posix_error();
	constexpr uintiso_t mul_factor{uintiso_subseconds_per_second/1000000000u};
	return {static_cast<intiso_t>(res.tv_sec),static_cast<uintiso_t>(res.tv_nsec)*mul_factor};
#else
	throw_posix_error(EINVAL);
#endif
}

#if defined(_WIN32) || defined(__CYGWIN__)
template<nt_family family,intiso_t off_to_epoch>
requires (family==nt_family::nt||family==nt_family::zw)
inline basic_timestamp<off_to_epoch> nt_family_clock_settime(posix_clock_id pclk_id,basic_timestamp<off_to_epoch> timestamp)
{
	if constexpr(std::same_as<win32_timestamp,basic_timestamp<off_to_epoch>>)
	{
		switch(pclk_id)
		{
			case posix_clock_id::realtime:
			case posix_clock_id::realtime_alarm:
			case posix_clock_id::realtime_coarse:
			case posix_clock_id::tai:
			{
				constexpr uintiso_t mul_factor{uintiso_subseconds_per_second/10000000u};
				std::uint64_t tms(static_cast<uintiso_t>(timestamp.seconds)*10000000ULL+timestamp.subseconds/mul_factor);
				std::uint64_t old_tms{};
				auto ntstatus{win32::nt::nt_set_system_time<family==nt_family::zw>(__builtin_addressof(tms),__builtin_addressof(old_tms))};
				if(ntstatus)
					throw_nt_error(ntstatus);
				return win32::to_win32_timestamp_ftu64(old_tms);
			}
			default:
				throw_nt_error(0xC00000EF);
		};
	}
	else
	{
		return static_cast<basic_timestamp<off_to_epoch>>(nt_family_clock_settime<family>(pclk_id,static_cast<win32_timestamp>(timestamp)));
	}
}
template<intiso_t off_to_epoch>
inline basic_timestamp<off_to_epoch> nt_clock_settime(posix_clock_id pclk_id,basic_timestamp<off_to_epoch> timestamp)
{
	return nt_family_clock_settime<nt_family::nt>(pclk_id,timestamp);
}

template<intiso_t off_to_epoch>
inline basic_timestamp<off_to_epoch> zw_clock_settime(posix_clock_id pclk_id,basic_timestamp<off_to_epoch> timestamp)
{
	return nt_family_clock_settime<nt_family::zw>(pclk_id,timestamp);
}

#endif

namespace details
{
#ifndef _WIN32
template<bool local_tm>
inline struct tm unix_timestamp_to_tm_impl(intiso_t seconds)
{
#if defined(_TIME64_T)
	time64_t val{static_cast<time64_t>(seconds)};
	struct tm t;
	if constexpr(local_tm)
	{
	if(localtime64_r(&val,&t)==0)
		throw_posix_error();
	}
	else
	{
	if(gmtime64_r(&val,&t)==0)
		throw_posix_error();
	}
	return t;
#else
	time_t val{static_cast<time_t>(seconds)};
	struct tm t;
	if constexpr(local_tm)
	{
	if(localtime_r(&val,&t)==0)
		throw_posix_error();
	}
	else
	{
	if(gmtime_r(&val,&t)==0)
		throw_posix_error();
	}
	return t;
#endif
}
#endif


inline iso8601_timestamp to_iso8601_local_impl(intiso_t seconds,uintiso_t subseconds)
{
#ifdef __MSDOS__
	return unix_timestamp_to_iso8601_tsp_impl_internal(seconds,subseconds,0);
#elif defined(_WIN32) || defined(__linux__)
	long tm_gmtoff{};
#if defined(_MSC_VER) || defined(_UCRT)
	{
		auto errn{_get_timezone(&tm_gmtoff)};
		if(errn)
			throw_posix_error(static_cast<int>(errn));
	}
#elif defined(_WIN32)
	tm_gmtoff=_timezone;
#else
	tm_gmtoff=timezone;
#endif
	long bias{};
#if defined(_MSC_VER) || defined(_UCRT)
	{
		auto errn{_get_dstbias(&bias)};
		if(errn)
			throw_posix_error(static_cast<int>(errn));
	}
#elif defined(_WIN32)
	bias=_dstbias;
#else
	bias=daylight?-3600L:0;
#endif
	std::uint32_t dst_timezone{static_cast<std::uint32_t>(static_cast<unsigned long>(tm_gmtoff)+static_cast<unsigned long>(bias))};
	return unix_timestamp_to_iso8601_tsp_impl_internal(sub_overflow(seconds,static_cast<intiso_t>(static_cast<uintiso_t>(dst_timezone))),
		subseconds,static_cast<std::int32_t>(static_cast<std::uint32_t>(0)-dst_timezone));
#else
	auto res{unix_timestamp_to_tm_impl<true>(seconds)};
	long tm_gmtoff{};
#if defined(_WIN32)
	#if defined(_MSC_VER) || defined(_UCRT)
	{
		auto errn{_get_timezone(&tm_gmtoff)};
		if(errn)
			throw_posix_error(static_cast<int>(errn));
	}
	#else
		tm_gmtoff=_timezone;
	#endif
	unsigned long ulong_tm_gmtoff{static_cast<unsigned long>(tm_gmtoff)};
	long seconds{};
#if defined(_MSC_VER) || defined(_UCRT)
	{
		auto errn{_get_dstbias(&seconds)};
		if(errn)
			throw_posix_error(static_cast<int>(errn));
	}
#else
	seconds=_dstbias;
#endif
	unsigned long ulong_seconds{static_cast<unsigned>(seconds)};
	ulong_tm_gmtoff+=ulong_seconds;
	ulong_tm_gmtoff=0ul-ulong_tm_gmtoff;
	tm_gmtoff=static_cast<long>(ulong_tm_gmtoff);
#elif defined(__TM_GMTOFF)
	tm_gmtoff=res.__TM_GMTOFF;
#elif defined(__NEWLIB__)
	tm_gmtoff=0;
#else
	tm_gmtoff=res.tm_gmtoff;
#endif
	std::uint8_t month{static_cast<std::uint8_t>(res.tm_mon)};
	auto year{res.tm_year+1900};
	if(month==0)
	{
		--year;
		month=12;
	}
	else
	{
		++month;
	}
	return {year,
		month,
		static_cast<std::uint8_t>(res.tm_mday),
		static_cast<std::uint8_t>(res.tm_hour),
		static_cast<std::uint8_t>(res.tm_min),
		static_cast<std::uint8_t>(res.tm_sec),subseconds,static_cast<std::int32_t>(tm_gmtoff)};
#endif
}

#ifdef __NEWLIB__
#if __has_cpp_attribute(gnu::dllimport)&&defined(__CYGWIN__)
[[gnu::dllimport]]
#endif
extern void m_tzset() noexcept asm("tzset");
#endif
}

inline void posix_tzset() noexcept
{
#if _WIN32
	_tzset();
#elif __NEWLIB__
	details::m_tzset();
#elif !defined(__wasi__) || defined(__wasilibc_unmodified_upstream)
	tzset();
#endif
}


template<intiso_t off_to_epoch>
inline iso8601_timestamp local(basic_timestamp<off_to_epoch> timestamp)
{
#ifdef __MSDOS__
	return utc(timestamp);
#if 0
	return details::to_iso8601_local_impl(static_cast<unix_timestamp>(timestamp));
#endif
#else
	if constexpr(off_to_epoch==0)
	{
		return details::to_iso8601_local_impl(timestamp.seconds,timestamp.subseconds);
	}
	else
	{
		unix_timestamp stmp(static_cast<unix_timestamp>(timestamp));
		return details::to_iso8601_local_impl(stmp.seconds,stmp.subseconds);
	}
#endif
}

#if defined(_WIN32) && (defined(_UCRT) || defined(_MSC_VER))
struct win32_timezone_t
{
	std::size_t tz_name_len{};
	bool is_dst{};
};

inline int posix_daylight() noexcept
{
	int hours;
	if(_get_daylight(&hours))
		return false;
	return hours;
}

inline win32_timezone_t timezone_name(bool is_dst=posix_daylight())
{
	win32_timezone_t tzt{.is_dst=is_dst};
	auto errn{_get_tzname(__builtin_addressof(tzt.tz_name_len),nullptr,0,is_dst)};
	if(errn)
		throw_posix_error(static_cast<int>(errn));
	return tzt;
}

inline constexpr std::size_t print_reserve_size(io_reserve_type_t<char,win32_timezone_t>,win32_timezone_t tzt) noexcept
{
	return tzt.tz_name_len;
}

namespace details
{

inline std::size_t print_reserve_define_impl(char* first,win32_timezone_t tzt)
{
	auto errn{_get_tzname(&tzt.tz_name_len,first,tzt.tz_name_len,tzt.is_dst)};
	if(errn)
		throw_posix_error(static_cast<int>(errn));
	if(tzt.tz_name_len)
		return tzt.tz_name_len-1;
	else
		return 0;
}

}

template<::fast_io::freestanding::contiguous_iterator Iter>
requires std::same_as<::fast_io::freestanding::iter_value_t<Iter>,char>
inline constexpr Iter print_reserve_define(io_reserve_type_t<char,win32_timezone_t>,Iter first,win32_timezone_t tzt)
{
	return details::print_reserve_define_impl(::fast_io::freestanding::to_address(first),tzt)+first;
}

#else

#ifdef __NEWLIB__
extern char *m_tzname[2] __asm__("_tzname");
extern int m_daylight __asm__("_daylight");
#endif

inline int posix_daylight()
{
#if defined(__MSDOS__) || (defined(__wasi__) &&!defined(__wasilibc_unmodified_upstream))
	return 0;
#elif defined(_WIN32)
	return _daylight;
#elif defined(__NEWLIB__)
	return m_daylight;
#elif defined(__BSD_VISIBLE) || defined(__DARWIN_C_LEVEL)
	std::time_t t{};
	struct tm stm{};
	if(localtime_r(&t,&stm)==0)
		throw_posix_error();
	return stm.tm_isdst*3600;
#else
	return daylight;
#endif
}

inline basic_io_scatter_t<char> timezone_name([[maybe_unused]] bool dst=posix_daylight()) noexcept
{
#if defined(__MSDOS__) || (defined(__wasi__) &&!defined(__wasilibc_unmodified_upstream))
	return {reinterpret_cast<char const*>(u8"UTC"),3}; 
#else
#if defined(__NEWLIB__)
	auto nm{m_tzname[dst]};
#else
	auto nm{tzname[dst]};
#endif
	return {nm,::fast_io::cstr_len(nm)};
#endif
}

#endif

inline void posix_clock_settime([[maybe_unused]] posix_clock_id pclk_id,[[maybe_unused]] unix_timestamp timestamp)
{
#ifdef _WIN32
	nt_clock_settime(pclk_id,timestamp);
#elif defined(__MSDOS__)
	switch(pclk_id)
	{
	case posix_clock_id::realtime:
	case posix_clock_id::realtime_alarm:
	case posix_clock_id::realtime_coarse:
	case posix_clock_id::tai:
	{
#if 0
		if constexpr(sizeof(std::time_t)<sizeof(intiso_t))
		{
			if(static_cast<intiso_t>(std::numeric_limits<std::time_t>::max())<timestamp.seconds)
				throw_posix_error(EOVERFLOW);
		}
		constexpr uintiso_t mul_factor{uintiso_subseconds_per_second/1000000u};
		timeval tv{static_cast<std::time_t>(timestamp.seconds),
		static_cast<long>(timestamp.subseconds/mul_factor)};
		if(::settimeofday(__builtin_addressof(tv), nullptr)<0)
			throw_posix_error();
#else
		details::set_dos_unix_timestamp(timestamp);
#endif
	}
	default:
		throw_posix_error(EINVAL);
	}
#elif (!defined(__NEWLIB__) || defined(_POSIX_TIMERS)) && (!defined(__wasi__) || defined(__wasilibc_unmodified_upstream))
	constexpr uintiso_t mul_factor{uintiso_subseconds_per_second/1000000000u};
	struct timespec res{static_cast<std::time_t>(timestamp.seconds),
	static_cast<long>(timestamp.subseconds/mul_factor)};
	auto clk{details::posix_clock_id_to_native_value(pclk_id)};
#ifdef __linux__
	system_call_throw_error(system_call<__NR_clock_settime,int>(clk,__builtin_addressof(res)));
#else
	if(::clock_settime(clk,__builtin_addressof(res))<0)
		throw_posix_error();
#endif
#else
	throw_posix_error(EINVAL);
#endif
}


#if 0
#ifdef _WIN32


namespace details
{

inline void 

}

inline void posix_clock_sleep_abstime_complete(posix_clock_id pclk_id,unix_timestamp timestamp)
{
	switch(pclk_id)
	{
	case posix_clock_id::realtime:
	case posix_clock_id::realtime_alarm:
	case posix_clock_id::realtime_coarse:
	case posix_clock_id::tai:
		return win32::details::win32_posix_clock_gettime_tai_impl();
	case posix_clock_id::monotonic:
	case posix_clock_id::monotonic_coarse:
	case posix_clock_id::monotonic_raw:
	case posix_clock_id::boottime:
//		return win32::details::win32_posix_clock_gettime_boottime_impl();
		return win32::details::win32_posix_clock_gettime_boottime_xp_impl();
	case posix_clock_id::process_cputime_id:
		return win32::details::win32_posix_clock_gettime_process_or_thread_time_impl<false>();
	case posix_clock_id::thread_cputime_id:
		return win32::details::win32_posix_clock_gettime_process_or_thread_time_impl<true>();
	default:
		throw_win32_error(0x00000057);
	}
}

inline [[nodiscard]] bool posix_clock_sleep_abstime(posix_clock_id pclk_id,unix_timestamp timestamp)
{
	posix_clock_sleep_abstime_complete(pclk_id,timestamp);
	return false;
}

#else

inline [[nodiscard]] bool posix_clock_sleep_abstime(posix_clock_id pclk_id,unix_timestamp timestamp)
{
	constexpr uintiso_t mul_factor{uintiso_subseconds_per_second/1000000000u};
	struct timespec timestamp_spec{static_cast<std::time_t>(timestamp.seconds),static_cast<long>(timestamp.subseconds/mul_factor)};
	auto ret{::clock_nanosleep(pclk_id,TIMER_ABSTIME,__builtin_addressof(timestamp_spec),ptr,nullptr)};
	if(ret<0)
	{
		auto ern{errno};
		if(ern!=EINTR)
			throw_posix_error(ern);
		return true;
	}
	return false;
}

inline void posix_clock_sleep_abstime_complete(posix_clock_id pclk_id,unix_timestamp timestamp)
{
	constexpr uintiso_t mul_factor{uintiso_subseconds_per_second/1000000000u};
	struct timespec timestamp_spec{static_cast<std::time_t>(timestamp.seconds),static_cast<long>(timestamp.subseconds/mul_factor)}
	for(;;)
	{
		auto ret{::clock_nanosleep(pclk_id,TIMER_ABSTIME,__builtin_addressof(timestamp_spec),nullptr)};
		if(ret==0)[[likely]]
			return;
		auto ern{errno};
		if(ern!=EINTR)
			throw_posix_error(ern);
	}
}

#endif
#endif
}