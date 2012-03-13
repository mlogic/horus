/*
 * Benchmark for Horus Key Utils
 *
 * Copyright (c) 2012, University of California, Santa Cruz, CA, USA.
 * Developers:
 *  Yan Li <yanli@ucsc.edu>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as 
 * published by the Free Software Foundation.
 *
 * Test scheme:
 *
 * 1: set a master key, write a 2 GB file in 4KB block
 *
 */

#include <check.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "horus.h"
#include "horus_key.h"
#include "benchmark.h"

int debug = 0;
const char *master_key = "Horus Rocks!";

void henchmark_key_calc_2gb (const int kht_depth,
			  const int branching_factor)
{
  int i;
  size_t s;

  int fake_fd = 9;
  size_t test_file_size = 2*1024*1024*1024L;
  void *block_key = NULL;
  int *kht_branching_factor;

  /* for this simple test, we use a KHT of 2 levels */
  // Keyed Hash Tree:
  // Master:        K(0,0)
  // L1:            K(1,0)
  // L2:     K(2,0)        K(2,1)
  
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
      if (NULL == block_key)
	{
	  printf ("Error getting block key for block %zu\n", s);
	  abort ();
	}
      if (debug)
	printf ("block %04d key = %s\n", i, print_key (block_key, HORUS_KEY_LEN));
    }

  printf ("key_calc_2gb_first_round,%d,%d,", kht_depth, branching_factor);
  end_clock ();
  printf ("\n");

  start_clock ();
  // run it again
  for (s = 0; s < test_file_size / 4096; ++s)
    {
      horus_get_leaf_block_key (fake_fd, &block_key, s);
      if (NULL == block_key)
	{
	  printf ("Error getting block key for block %zu\n", s);
	  abort ();
	}
      if (debug)
	printf ("block %04d key = %s\n", i, print_key (block_key, HORUS_KEY_LEN));
    }
  printf ("key_calc_2gb_second_round,%d,%d,", kht_depth, branching_factor);
  end_clock ();
  printf ("\n");
}

int
main (int argc, char **argv)
{
  if ( (argc >= 2) && strcmp (argv[1], "-d") == 0 )
    debug = 1;

  printf ("round,kht_depth,branching_factor,time_sec,time_nanosec\n");

  henchmark_key_calc_2gb (2, 2);
  henchmark_key_calc_2gb (3, 2);
  henchmark_key_calc_2gb (4, 2);
  henchmark_key_calc_2gb (5, 2);
  henchmark_key_calc_2gb (6, 2);
  henchmark_key_calc_2gb (7, 2);
  henchmark_key_calc_2gb (8, 2);
  henchmark_key_calc_2gb (9, 2);
  henchmark_key_calc_2gb (10, 2);
  henchmark_key_calc_2gb (4, 3);
  henchmark_key_calc_2gb (4, 4);
  henchmark_key_calc_2gb (4, 5);
  henchmark_key_calc_2gb (4, 6);
  henchmark_key_calc_2gb (4, 7);
  henchmark_key_calc_2gb (4, 8);
  henchmark_key_calc_2gb (4, 9);
  henchmark_key_calc_2gb (4, 10);
  henchmark_key_calc_2gb (4, 11);
  return 0;
}
