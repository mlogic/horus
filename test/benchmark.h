#ifndef _HORUS_BENCHMARK_H
#define _HORUS_BENCHMARK_H

#include <sys/times.h>
#include <stdio.h>

void start_clock(void);
void end_clock(char *msg);

static clock_t st_time;
static clock_t en_time;
static struct tms st_cpu;
static struct tms en_cpu;

inline long
tick_per_second ()
{
  return sysconf(_SC_CLK_TCK);
}

void
start_clock ()
{
  st_time = times(&st_cpu);
}

intmax_t
get_real_time ()
{
  return (intmax_t)(en_time - st_time);
}


/* This example assumes that the result of each subtraction
   is within the range of values that can be represented in
   an integer type. */
void
end_clock(char *msg)
{
  en_time = times(&en_cpu);

  fputs (msg, stdout);
  printf ("%jd,%jd,%jd\n",
	  (intmax_t)get_real_time (),
	  (intmax_t)(en_cpu.tms_utime - st_cpu.tms_utime),
	  (intmax_t)(en_cpu.tms_stime - st_cpu.tms_stime)
	  );
}

#endif
