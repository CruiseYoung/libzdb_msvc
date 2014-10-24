#ifndef MACROS_INCLUDED
#define MACROS_INCLUDED

/* ---------------------------------------------------------- Build macros */


/* Mask out GCC __attribute__ extension for non- GCC/llvm-clang compilers. */
#if (! (defined(__GNUC__) || defined(__clang__)))
#define __attribute__(x)
#endif

#if defined(_MSC_VER) && !defined(__func__)
#define __func__ __FUNCTION__
#endif

#if defined(_MSC_VER) && !defined(snprintf)
#define snprintf _snprintf
#endif

#if defined(_MSC_VER) && !defined(inline)
#define inline __inline
#endif

#ifdef _WIN32
#include <stdint.h>
#include <inttypes.h>
#endif

#if defined(_MSC_VER) && !defined(gmtime_r)
#include <time.h>
#define gmtime_r(a, b) gmtime_s(b, a)
#endif

//#if defined(_MSC_VER) && !defined(_WINSOCK2API_)
///*
//* Structure used in select() call, taken from the BSD file sys/time.h.
//*/
//struct timeval {
//    long    tv_sec;         /* seconds */
//    long    tv_usec;        /* and microseconds */
//};
//#endif

#if defined(_MSC_VER) && !defined(suseconds_t)
#define suseconds_t long
#endif

#if defined(_MSC_VER)
#include <time.h>
#include <WinSock2.h>
//#pragma comment(lib, "ws2_32.lib") // select(0, 0, 0, 0, &t);
#include <windows.h> //I've ommited this line.


//#if !defined(sleep)
//#define sleep Sleep
//#endif

struct timezone
{
    int  tz_minuteswest; /* minutes west of Greenwich */
    int  tz_dsttime;     /* type of DST correction */
};

int gettimeofday(struct timeval *tv, struct timezone *tz);
#endif

#endif //!MACROS_INCLUDED
