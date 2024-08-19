#ifndef INCLUDE_DBG_SG_ASSERT_H_
#define INCLUDE_DBG_SG_ASSERT_H_


#include "sg_log.h"
#include <cassert>

#ifdef ENABLE_SG_ASSERT
#ifdef _MSC_VER
#define SG_ASSERT(cond, ...) (void)(                                           \
    (!!(cond)) ||                                                              \
    (_wassert(_CRT_WIDE(#cond), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0) \
)
#else
#define SG_ASSERT(cond, ...)                                    \
if(!(cond)) { sg_print_log(__VA_ARGS__);                        \
    (__assert_fail(#cond, __builtin_FILE(), __builtin_LINE(),   \
    __extension__ __PRETTY_FUNCTION__)); }
#endif // _MSC_VER
#else
#define SG_ASSERT(cond, ...) {}// do { (void)sizeof(cond); } while(false)
#endif


#endif  // INCLUDE_DBG_SG_ASSERT_H_
