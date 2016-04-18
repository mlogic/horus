/*
 * Benchmark for Horus Read/Write
 *
 * Developers:
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
 */
#define DEBUG 0

#include <check.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "horus.h"
#include "log.h"
#include "horus_key.h"
#include "benchmark.h"

const char *test_file = "/tmp/horus_test_read_write_tmpfile";
const char *master_key = "Horus Rocks!";

void benchmark_key_calc_2gb (const int kht_depth,
			     const int branching_factor)
{
  int i;

  int fd;
  char test_block_data[HORUS_BLOCK_SIZE];
  size_t test_file_size = 2*1024*1024*1024L;
  int *kht_branching_factor;
  
  // generate branching factor arrays on-the-fly from branching_factor
  kht_branching_factor = malloc (sizeof (int) * (kht_depth-1));
  if (kht_branching_factor == NULL)
    {
      perror (__func__);
      abort ();
    }
  for (i = 0; i < kht_depth-1; ++i)
    {
      kht_branching_factor[i] = branching_factor;
    }

  // init, generate some data for test_block_data
  for (i = 0; i < HORUS_BLOCK_SIZE; ++i)
    test_block_data[i] = (char)('0' + i % 10);

  // open the file, set a master key
  unlink (test_file);
  fd = open (test_file, O_CREAT | O_RDWR, 0644);
  if (fd < 0)
    abort ();
  horus_set_kht (fd, kht_depth, HORUS_BLOCK_SIZE, kht_branching_factor); 
  horus_add_key (fd, master_key, strnlen (master_key, HORUS_KEY_LEN), 0, 0);

  start_clock ();

  // write 2GB data
  for (i = 0; i < test_file_size / HORUS_BLOCK_SIZE; ++i)
    {
      if (unlikely (HORUS_BLOCK_SIZE !=
		    horus_write (fd, (void*)test_block_data, HORUS_BLOCK_SIZE)))
	{
	  perror (__func__);
	  abort ();
	}
    }

  end_clock ();
  printf ("write_2gb,%d,%d,", kht_depth, branching_factor);
  print_last_clock_diff ();
  printf ("\n");
  close (fd);

  // Now read it back
  fd = open (test_file, O_CREAT | O_RDWR, 0644);
  if (fd < 0)
    abort ();
  horus_set_kht (fd, kht_depth, HORUS_BLOCK_SIZE, kht_branching_factor); 
  horus_add_key (fd, master_key, strnlen (master_key, HORUS_KEY_LEN), 0, 0);

  start_clock ();
  // run it again
  for (i = 0; i < test_file_size / HORUS_BLOCK_SIZE; ++i)
    {
      if (unlikely (HORUS_BLOCK_SIZE !=
		    horus_read (fd, (void*)test_block_data, HORUS_BLOCK_SIZE)))
	{
	  perror (__func__);
	  abort ();
	}
    }
  end_clock ();
  printf ("read_2gb,%d,%d,", kht_depth, branching_factor);
  print_last_clock_diff ();
  printf ("\n");
}

int
main (int argc, char **argv)
{
  if ( (argc >= 2) && strcmp (argv[1], "-d") == 0 )
    debug = 1;

  printf ("round,kht_depth,branching_factor,time_sec,time_nanosec\n");

  benchmark_key_calc_2gb (4, 2);
  benchmark_key_calc_2gb (4, 4); 
  benchmark_key_calc_2gb (4, 6);
  benchmark_key_calc_2gb (4, 8);
  benchmark_key_calc_2gb (2, 7);
  benchmark_key_calc_2gb (4, 7);
  benchmark_key_calc_2gb (6, 7);
  benchmark_key_calc_2gb (8, 7);
  return 0;
}
