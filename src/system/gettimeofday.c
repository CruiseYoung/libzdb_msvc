#include "Macros.h"

#if defined(_MSC_VER) && !defined(gettimeofday)

#include <time.h>       /* _timezone, _daylight */
#include <WinSock2.h>   /* timeval */
#include <windows.h>    /* I've ommited this line. */

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

//struct timezone
//{
//    int  tz_minuteswest; /* minutes west of Greenwich */
//    int  tz_dsttime;     /* type of DST correction */
//};

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    // Define a structure to receive the current Windows filetime
    FILETIME ft;

    // Initialize the present time to 0 and the timezone to UTC
    unsigned __int64 tmpres = 0;
    static int tzflag = 0;

    if (NULL != tv)
    {
        GetSystemTimeAsFileTime(&ft);

        // The GetSystemTimeAsFileTime returns the number of 100 nanosecond
        // intervals since Jan 1, 1601 in a structure. Copy the high bits t
        // the 64 bit tmpres, shift it left by 32 then or in the low 32 bit
        tmpres |= ft.dwHighDateTime;
        tmpres <<= 32;
        tmpres |= ft.dwLowDateTime;

        // Convert to microseconds by dividing by 10
        tmpres /= 10;
        // The Unix epoch starts on Jan 1 1970.  Need to subtract the diffe
        // in seconds from Jan 1 1601.
        tmpres -= DELTA_EPOCH_IN_MICROSECS;

        // Finally change microseconds to seconds and place in the seconds 
        // The modulus picks up the microseconds.
        tv->tv_sec = (long)(tmpres / 1000000UL);
        tv->tv_usec = (long)(tmpres % 1000000UL);
    }

    if (NULL != tz)
    {
        if (!tzflag)
        {
            _tzset();
            tzflag++;
        }
        tz->tz_minuteswest = _timezone / 60;
        tz->tz_dsttime = _daylight;
    }

    return 0;
}

#endif
