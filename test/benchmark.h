/*
 * Horus Benchmark Utils
 *
 * Developers:
 *  Nakul Dhotre <nakul@soe.ucsc.edu>
 *  Yan Li <yanli@ascar.io>
 *
 * Copyright (c) 2012, The Regents of the University of California
 * All rights reserved.
 * Created by the Storage Systems Research Center, http://www.ssrc.ucsc.edu
 * Department of Computer Science
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Storage Systems Research Center, the
 *       University of California, nor the names of its contributors
 *       may be used to endorse or promote products derived from this
 *       software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * REGENTS OF THE UNIVERSITY OF CALIFORNIA BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * For Mac OS X, we use Mach Absolute Time Units as documented at:
 * https://developer.apple.com/library/mac/#qa/qa1398/_index.html
 *
 * For Linux, we use clock_gettime()'s CLOCK_MONOTONIC.
 */

#ifndef _HORUS_BENCHMARK_H
#define _HORUS_BENCHMARK_H

static inline struct timespec
diff (struct timespec start, struct timespec end);

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
  struct timespec st_hires_time, en_hires_time;

#endif /* __APPLE__ */

struct horus_clock
{
  struct timespec start;
  struct timespec end;
};

inline void start_horus_clock(struct horus_clock *p)
{
#ifdef __APPLE__
  ;
#else
  clock_gettime (CLOCK_MONOTONIC, &p->start);
#endif
};

inline void stop_horus_clock(struct horus_clock *p)
{
#ifdef __APPLE__
  ;
#else
  clock_gettime (CLOCK_MONOTONIC, &p->end);
#endif
};

void
print_horus_clock_time (struct horus_clock *p)
{
#ifdef __APPLE__
  ;
#else
  struct timespec result;
  result = diff (p->start, p->end);
  printf ("%ld,%ld", result.tv_sec, result.tv_nsec);
#endif
}


inline void
start_clock ()
{
#ifdef __APPLE__
  start = mach_absolute_time ();
#else /* __APPLE__ */
  clock_gettime (CLOCK_MONOTONIC, &st_hires_time);
#endif /* __APPLE__ */
}

static inline struct timespec
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

/* end_clock must be quick, and doesn't affect the timing much */
inline void
end_clock (void)
{
#ifdef __APPLE__
  end = mach_absolute_time ();
#else /* __APPLE__ */
  clock_gettime (CLOCK_MONOTONIC, &en_hires_time);
#endif /* __APPLE__ */
}

void
print_last_clock_diff (void)
{
  long sec, nanosec;
#ifdef __APPLE__
  elapsed = end - start;
  elapsedNano = AbsoluteToNanoseconds( *(AbsoluteTime *) &elapsed );
  nanosec = * (uint64_t *) &elapsedNano;
  sec = nanosec / 1000000000;
  nanosec = nanosec % 1000000000;
#else /* __APPLE__ */
  struct timespec result;
  result = diff (st_hires_time, en_hires_time);
  sec = result.tv_sec;
  nanosec = result.tv_nsec;
#endif /* __APPLE__ */
  printf ("%ld,%ld", sec, nanosec);
}

#endif /* _HORUS_BENCHMARK_H */
