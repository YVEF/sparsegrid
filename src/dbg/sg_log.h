#ifndef INCLUDE_DBG_SG_LOG_H_
#define INCLUDE_DBG_SG_LOG_H_

#include <iostream>

enum LogLevel
{
    Error = 0,
    Warn = 1,
    Info = 2,
    Verbose = 3
};

/* FOREGROUND */
#define RST  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define FRED(x) KRED << (x) << RST
#define FGRN(x) KGRN << (x) << RST
#define FYEL(x) KYEL << (x) << RST
#define FBLU(x) KBLU << (x) << RST
#define FMAG(x) KMAG << (x) << RST
#define FCYN(x) KCYN << (x) << RST
#define FWHT(x) KWHT << (x) << RST

#ifdef ENABLE_STRUCTURE_LOGGING 
#define SG_STR_LOG1(prefix, obj1, nest_lev) sg_structure_log(prefix, nest_lev, obj1)
#define SG_STR_LOG2(prefix, obj1, obj2, nest_lev) sg_structure_log(prefix, nest_lev, obj1, obj2)
#define SG_STR_LOG3(prefix, obj1, obj2, obj3, nest_lev) sg_structure_log(prefix, nest_lev, obj1, obj2, obj3)
#define SG_STR_LOG1_ROOT_LVL(prefix, obj1, nest_lev) sg_structure_log(prefix, 0, obj1)
#define SG_STR_LOG2_ROOT_LVL(prefix, obj1, obj2, nest_lev) sg_structure_log(prefix, 0, obj1, obj2)
#else
#define SG_STR_LOG1(prefix, obj1, nest_lev)
#define SG_STR_LOG2(prefix, obj1, obj2, nest_lev)
#define SG_STR_LOG3(prefix, obj1, obj2, obj3, nest_lev)
#define SG_STR_LOG1_ROOT_LVL(prefix, obj1, nest_lev)
#define SG_STR_LOG2_ROOT_LVL(prefix, obj1, obj2, nest_lev)
#endif

#ifdef ENABLE_LOGGING
#define SG_LOG(msg, level) sg_log(msg, level)
#else
#define SG_LOG(msg, level)
#endif


template<typename ...Args>
constexpr
void sg_structure_log(const char* prefix, int nesting_level, Args&&...) noexcept;

template<typename ...Args>
constexpr
void sg_print_log(Args&&... objs) noexcept;
constexpr
void sg_print_log(...) noexcept {}

void sg_log(const char* log, LogLevel level);

template<typename T1, typename ...Args>
constexpr
void to_stream_recs(char delim, std::ostream& stream, T1&& t1, Args&&... args);

template<typename ...Args>
constexpr
void sg_structure_log(const char* prefix, int nesting_level, Args&&... objs) noexcept
{
    while(nesting_level-- > 0)
	std::cout << "-";
    std::cout << prefix;
    to_stream_recs('-', std::cout, std::forward<Args>(objs)...);
    std::cout << '\n';
}

template<typename ...Args>
constexpr
void sg_print_log(Args&&... objs) noexcept
{
    to_stream_recs('-', std::cout, std::forward<Args>(objs)...);
    std::cout << '\n';
}


template<typename T1, typename ...Args>
constexpr
void to_stream_recs(char delim, std::ostream& stream, T1&& t1, Args&&... args)
{
    stream << std::forward<T1>(t1);
    if constexpr (sizeof...(args) > 0)
    {
    	stream << delim; 
    	to_stream_recs(delim, stream, std::forward<Args>(args)...);
    }
}


#endif  // INCLUDE_DBG_SG_LOG_H_
