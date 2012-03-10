/*
 * Horus Benchmark Utils
 *
 * Copyright (c) 2012, University of California, Santa Cruz, CA, USA.
 * Developers:
 *  Nakul Dhotre <nakul@soe.ucsc.edu>
 *  Yan Li <yanli@ucsc.edu>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as 
 * published by the Free Software Foundation.
 *
 * For Mac OS X, we use Mach Absolute Time Units as documented at:
 * https://developer.apple.com/library/mac/#qa/qa1398/_index.html
 *
 * For Linux, we use clock_gettime()'s CLOCK_MONOTONIC.
 */

#ifndef _HORUS_BENCHMARK_H
#define _HORUS_BENCHMARK_H

#ifdef __APPLE__
#  include <CoreServices/CoreServices.h>
#  include <mach/mach.h>
#  include <mach/mach_time.h>
#  include <unistd.h>
   uint64_t        start;
   uint64_t        end;
   uint64_t        elapsed;
   Nanoseconds     elapsedNano;
#else /* For Linux code follows */
#  include <sys/times.h>
#  include <stdio.h>
#  include <time.h>
   static struct timespec st_hires_time, en_hires_time;
#endif /* __APPLE__ */

void
start_clock ()
{
#ifdef __APPLE__
  start = mach_absolute_time ();
#else /* __APPLE__ */
  clock_gettime (CLOCK_MONOTONIC, &st_hires_time);
#endif /* __APPLE__ */
}

struct timespec
diff (struct timespec start, struct timespec end)
{
  struct timespec temp;
  if ((end.tv_nsec-start.tv_nsec)<0) {
    temp.tv_sec = end.tv_sec-start.tv_sec-1;
    temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
  } else {
    temp.tv_sec = end.tv_sec-start.tv_sec;
    temp.tv_nsec = end.tv_nsec-start.tv_nsec;
  }
  return temp;
}

void
end_clock (void)
{
  long sec, nanosec;
#ifdef __APPLE__
  end = mach_absolute_time ();
  elapsed = end - start;
  elapsedNano = AbsoluteToNanoseconds( *(AbsoluteTime *) &elapsed );
  nanosec = * (uint64_t *) &elapsedNano;
  sec = nanosec / 1000000000;
  nanosec = nanosec % 1000000000;
#else /* __APPLE__ */
  struct timespec result;
  clock_gettime (CLOCK_MONOTONIC, &en_hires_time);
  result = diff (st_hires_time, en_hires_time);
  sec = result.tv_sec;
  nanosec = result.tv_nsec;
#endif /* __APPLE__ */
  printf ("%ld,%ld", sec, nanosec);
}

#endif /* _HORUS_BENCHMARK_H */
