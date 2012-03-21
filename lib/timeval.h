#ifndef _TIMEVAL_H_
#define _TIMEVAL_H_

#include <sys/time.h>

/* operation on timeval structure */
#ifndef timeval_clear
#define timeval_clear(tvp) ((tvp)->tv_sec = (tvp)->tv_usec = 0)
#endif /*timevalclear*/

#ifndef timeval_sub
#define timeval_sub(a, b, res)                        \
  do {                                                \
    (res)->tv_sec = (a)->tv_sec - (b)->tv_sec;        \
    (res)->tv_usec = (a)->tv_usec - (b)->tv_usec;     \
    if ((res)->tv_usec < 0)                           \
      {                                               \
        (res)->tv_sec--;                              \
        (res)->tv_usec += 1000000;                    \
      }                                               \
  } while (0)
#endif /*timeval_sub*/

#ifndef snprint_timeval
#define snprint_timeval(buf,size,tv)                      \
  do {                                                    \
    long int days, hours, minutes, secs, usecs;           \
    days = (tv)->tv_sec / 60 / 60 / 24;                   \
    hours = (tv)->tv_sec / 60 / 60 % 24;                  \
    minutes = (tv)->tv_sec / 60 % 60;                     \
    secs = (tv)->tv_sec % 60;                             \
    usecs = (tv)->tv_usec;                                \
    if (days)                                             \
      snprintf (buf, size, "%ldd%02ld:%02ld:%02ld.%06ld", \
                days, hours, minutes, secs, usecs);       \
    else if (hours)                                       \
      snprintf (buf, size, "%02ld:%02ld:%02ld.%06ld",     \
                hours, minutes, secs, usecs);             \
    else if (minutes)                                     \
      snprintf (buf, size, "%02ld:%02ld.%06ld",           \
                minutes, secs, usecs);                    \
    else                                                  \
      snprintf (buf, size, "%ld.%06ld", secs, usecs);     \
  } while (0)
#endif /*snprint_timeval*/

#ifndef snprint_localtime
#define snprint_localtime(buf,size,tv)                    \
  do {                                                    \
    int ret;                                              \
    struct tm *tm;                                        \
    tm = localtime (&(tv)->tv_sec);                       \
    ret = strftime (buf, size, "%Y/%m/%d %H:%M:%S", tm);  \
    if (ret == 0)                                         \
      fprintf (stderr, "strftime error");                 \
  } while (0)
#endif /*snprint_localtime*/

#endif /*_TIMEVAL_H_*/

