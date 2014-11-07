#ifndef MACROS_INCLUDED
#define MACROS_INCLUDED

/************************************************************************/
/*   be used head file                                                                               */
/************************************************************************/

/* ---------------------------------------------------------- Build macros */


/* Mask out GCC __attribute__ extension for non- GCC/llvm-clang compilers. */
#if (! (defined(__GNUC__) || defined(__clang__)))
#define __attribute__(x)
#endif


#if defined(_MSC_VER) && !defined(inline)
#define inline __inline
#endif


#if defined(_MSC_VER) && !defined(__func__)
#define __func__ __FUNCTION__
#endif


/************************************************************************/
/*   be used Implementation file                                                               */
/************************************************************************/
#ifdef _WIN32
#include <stdint.h>
#include <inttypes.h>
#endif


#if defined(_MSC_VER) && !defined(snprintf)
#define snprintf _snprintf
#endif


#if defined(_MSC_VER) && !defined(gmtime_r)
#include <time.h>
#define gmtime_r(a, b) gmtime_s((b), (a))
#endif


#if defined(_MSC_VER) && !defined(suseconds_t)
#define suseconds_t long
#endif


//#if !defined(sleep) && !defined(sleep)
//#include <windows.h>    /* Sleep */
//#define sleep Sleep
//#endif


/*
* Structure used in select() call, taken from the BSD file sys/time.h.
*/
//#if defined(_MSC_VER) && !defined(_WINSOCK2API_)
//struct timeval {
//    long    tv_sec;         /* seconds */
//    long    tv_usec;        /* and microseconds */
//};
//#endif


#if defined(_MSC_VER)
#include <WinSock2.h>               /* struct timeval */
//#pragma comment(lib, "ws2_32.lib")  /* release need select(0, 0, 0, 0, &t);  */

//#include <time.h>                   /* struct timezone */
struct timezone
{
    int  tz_minuteswest; /* minutes west of Greenwich */
    int  tz_dsttime;     /* type of DST correction */
};

int gettimeofday(struct timeval *tv, struct timezone *tz);
#endif


#endif //!MACROS_INCLUDED
