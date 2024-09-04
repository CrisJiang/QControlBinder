#ifndef _CRIS_LOG_
#define _CRIS_LOG_
#pragma once
#include <format>
#include <iostream>
#include <source_location>
#include <chrono>

#define TICK(name) auto name##_start = std::chrono::high_resolution_clock::now();

#define TOCK(name) auto name##_end = std::chrono::high_resolution_clock::now(); \
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(name##_end- name##_start); \
    LOG_INFO(("" #name "_cost_time={}.{}"), duration.count()/1000, duration.count()%1000);

#define LOG_DEBUG(...) log_generic(log_level::debug, __VA_ARGS__)
#define LOG_INFO(...) log_generic(log_level::info, __VA_ARGS__)

#define FOREACH_log_level(f) \
	f(trace) \
	f(debug) \
	f(info) \
	f(error) \
	f(fatal)

enum class log_level {
#define _FUNCTION_(name)  name,
	FOREACH_log_level(_FUNCTION_)
#undef _FUNCTION_
};

inline static std::string log_level_name(log_level lev) {
	switch (lev)
	{
#define _FUNCTION_(name) case log_level::name: return #name;
		FOREACH_log_level(_FUNCTION_)
#undef  _FUNCTION_
	}
	return "nonelevel";
}

inline static log_level g_log_level = log_level::info;

// 1. 变参函数要放在function参数最后面,而默认参数也要放在最后面
//   所以这里我们玩了一个技巧,利用with_source_location包装默认参数
//   source_location 使得log_info能够满足我们的需求
template <class T> struct with_source_location
{
private:
	T                    inner_;
	std::source_location loc;

public:
	template <class U>
	requires std::constructible_from<T, U>
		consteval with_source_location(U&& inner, std::source_location loc = std::source_location::current())
		: inner_(std::forward<U>(inner)), loc(loc) {};
	T const& format() const
	{
		return inner_;
	};
	std::source_location const& location() const
	{
		return loc;
	};
};

template <class... _Types> 
void log_generic(log_level lev, with_source_location<std::format_string<_Types...>> fmt, _Types &&...args) {
	if (lev >= g_log_level) {
		auto const& loc = fmt.location();
		std::cout << loc.file_name() << ":" << loc.line() << " [" << log_level_name(lev) << "] "
                  << ::std::vformat(fmt.format().get(), std::make_format_args(args...)) << '\n';
	}
}
//
//template <class... _Types>
//void log_debug(with_source_location<std::_Fmt_string<_Types...>> fmt, _Types &&...args) {
//	return log_generic(log_level::debug, std::move(fmt), std::forward<_Types>(args)...);
//}

#define _FUNCTION_(name) \
	template <class... _Types> \
	void log_##name(with_source_location<std::format_string<_Types...>> fmt, _Types &&...args) { \
		return log_generic(log_level::name, std::move(fmt), std::forward<_Types>(args)...); \
	}
FOREACH_log_level(_FUNCTION_)
#undef _FUNCTION_

template <class... _Types>
void log(const std::format_string<_Types...> fmt, _Types &&..._Args,
	std::source_location              loc = std::source_location::current())
{
	std::cout << loc.file_name() << ":" << loc.line() << " [Info] "
		<< vformat(fmt._Str, _STD make_format_args(_Args...)) << '\n';
}

template <typename... Args> inline void println(const std::format_string<Args...> fmt, Args &&...args) {
	std::cout << std::vformat(fmt.get(), std::make_format_args(args...)) << '\n';
}
#endif