#ifndef _UTIL_H_
#define _UTIL_H_

#include <string.h>
#include <stdbool.h>


#undef unlikely
#undef likely
#if defined(__GNUC__) && (__GNUC__ > 2) && defined(__OPTIMIZE__)
#define unlikely(expr) (__builtin_expect(!!(expr), 0))
#define likely(expr) (__builtin_expect(!!(expr), 1))
#else
#define unlikely(expr) (expr)
#define likely(expr) (expr)
#endif


bool hex2bin(unsigned char *p, char *hexstr, size_t len);
char *bin2hex(unsigned char *p, size_t len);


























#endif

