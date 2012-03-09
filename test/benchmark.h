#ifndef _HORUS_BENCHMARK_H
#define _HORUS_BENCHMARK_H

#include <sys/times.h>
#include <stdio.h>

#include <time.h>

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

#ifdef HIRES_TIME

static struct timespec st_hires_time, en_hires_time;

void start_clock_hires ()
{
  clock_gettime(CLOCK_MONOTONIC, &st_hires_time);
}

struct timespec diff(struct timespec start, struct timespec end)
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

void end_clock_hires(char *msg)
{
  struct timespec result;
  clock_gettime(CLOCK_MONOTONIC, &en_hires_time);
  fputs (msg, stdout);
  result = diff(st_hires_time,en_hires_time);
  printf ("%d %ld\n",result.tv_sec,
          result.tv_nsec);
}



#endif
#endif
