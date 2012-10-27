/*
 * Benchmark for Horus Key Utils
 *
 * Developers:
 *  Yan Li <yanli@cs.ucsc.edu>
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
 * Test scheme:
 *
 * 1: set a master key, write a 2 GB file in 4KB block
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


const char *master_key = "Horus Rocks!";

void benchmark_key_calc_2gb (const int kht_depth,
			  const int branching_factor)
{
  int i;
  size_t s;

  int fake_fd = 9;
  size_t test_file_size = 2*1024*1024*1024L;
  void *block_key = NULL;
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
  
  // set a master key
  horus_set_kht (fake_fd, kht_depth, 4096, kht_branching_factor); 
  horus_add_key (fake_fd, master_key, strnlen (master_key, HORUS_KEY_LEN), 0, 0);

  start_clock ();

  // get the key for block 1 for later use
  for (s = 0; s < test_file_size / 4096; ++s)
    {
      horus_get_leaf_block_key (fake_fd, &block_key, s);
      if (unlikely (NULL == block_key))
	{
	  printf ("Error getting block key for block %zu\n", s);
	  abort ();
	}
      debug_print ("block %04d key = %s\n", i, print_key (block_key, HORUS_KEY_LEN));
    }

  end_clock ();
  printf ("key_calc_2gb_first_round,%d,%d,", kht_depth, branching_factor);
  print_last_clock_diff ();
  printf ("\n");

  start_clock ();
  // run it again
  for (s = 0; s < test_file_size / 4096; ++s)
    {
      horus_get_leaf_block_key (fake_fd, &block_key, s);
      if (unlikely (NULL == block_key))
	{
	  printf ("Error getting block key for block %zu\n", s);
	  abort ();
	}
      debug_print ("block %04d key = %s\n", i, print_key (block_key, HORUS_KEY_LEN));
    }
  end_clock ();
  printf ("key_calc_2gb_second_round,%d,%d,", kht_depth, branching_factor);
  print_last_clock_diff ();
  printf ("\n");
}

int
main (int argc, char **argv)
{
  if ( (argc >= 2) && strcmp (argv[1], "-d") == 0 )
    debug = 1;

  printf ("round,kht_depth,branching_factor,time_sec,time_nanosec\n");

  benchmark_key_calc_2gb (2, 2);
  benchmark_key_calc_2gb (3, 2);
  benchmark_key_calc_2gb (4, 2);
  benchmark_key_calc_2gb (5, 2);
  benchmark_key_calc_2gb (6, 2);
  benchmark_key_calc_2gb (7, 2);
  benchmark_key_calc_2gb (8, 2);
  benchmark_key_calc_2gb (9, 2);
  benchmark_key_calc_2gb (10, 2);
  benchmark_key_calc_2gb (4, 3);
  benchmark_key_calc_2gb (4, 4);
  benchmark_key_calc_2gb (4, 5);
  benchmark_key_calc_2gb (4, 6);
  benchmark_key_calc_2gb (4, 7);
  benchmark_key_calc_2gb (4, 8);
  benchmark_key_calc_2gb (4, 9);
  benchmark_key_calc_2gb (4, 10);
  benchmark_key_calc_2gb (4, 11);
  return 0;
}
