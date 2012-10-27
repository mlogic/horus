/*
 * Horus API Test
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
 * 1: set a master key, write two 4KB blocks into a test file using
 *    Horus API, close file
 * 2: re-open file, make sure they can be read correctly
 * 3: set a key for block 1 only
 * 4: make sure block 1 reads correctly and not block 2
 */

#include <check.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "horus.h"

const char *test_file = "/tmp/horus_test_api_tmpfile";
const char *master_key = "Horus Rocks!";
const int TEST_BLOCK_COUNT = 2;
#define KHT_DEPTH 2

START_TEST (test_horus_api)
{
  int i, j;
  int fd;
  char test_block_data[TEST_BLOCK_COUNT][HORUS_BLOCK_SIZE];
  void *buf;
  void *block_1_key = NULL;

  /* for this simple test, we use a KHT of 2 levels */
  // Keyed Hash Tree:
  // Master:        K(0,0)
  // L1:            K(1,0)
  // L2:     K(2,0)        K(2,1)
  
  const int kht_branching_factor[KHT_DEPTH-1] = { 2 };
  
  // init, generate some data for test_block_data
  for (i = 0; i < TEST_BLOCK_COUNT; ++i)
    for (j = 0; j < HORUS_BLOCK_SIZE; ++j)
      test_block_data[i][j] = (char)('0' + j % 10);
  
  // open the file, set a master key
  fd = open (test_file, O_CREAT | O_RDWR, 0644);
  if (-1 == fd)
    {
      perror ("Create test file error");
      abort ();
    }
  horus_set_kht (fd, 2, 4096, kht_branching_factor); 
  horus_add_key (fd, master_key, strnlen (master_key, HORUS_KEY_LEN), 2, 0);

  // write two blocks, keep checksum
  for (i = 0; i < TEST_BLOCK_COUNT; ++i)
    horus_write (fd, (void*)test_block_data[i], sizeof (test_block_data[i]));

  // get the key for block 1 for later use
  horus_get_leaf_block_key (fd, &block_1_key, 1);

  // close, then re-open the file
  close (fd);
  fd = open (test_file, O_RDONLY);
  if (-1 == fd)
    {
      printf("Error opening test file for the 2nd time!");
      abort ();
    }

  // set the master key
  horus_add_key (fd, master_key, strnlen (master_key, HORUS_KEY_LEN), 2, 0);

  // read and check two blocks are OK
  buf = malloc (HORUS_BLOCK_SIZE);
  if (NULL == buf)
    {
      perror (__func__);
      abort ();
    }
  for (i = 0; i < TEST_BLOCK_COUNT; ++i)
    {
      if (horus_read (fd, buf, HORUS_BLOCK_SIZE) != HORUS_BLOCK_SIZE)
	{
	  perror (__func__);
	  abort ();
	}
      fail_unless (memcmp (test_block_data[i], buf, HORUS_BLOCK_SIZE) == 0,
		   "Block data error");
    }
      
  // remove the master key, and key for block 1 only
  // TODO

  // make sure block 1 reads well
  // TODO

  // make sure block 2 doesn't read good
  // TODO
  close(fd);
}
END_TEST

Suite *
horus_api_test_suite (void)
{
  Suite *s = suite_create ("Horus API Test");

  /* Core test case */
  TCase *tc_core = tcase_create ("Core");
  tcase_add_test (tc_core, test_horus_api);
  suite_add_tcase (s, tc_core);

  return s;
}

int
main (void)
{
  int number_failed;
  Suite *s = horus_api_test_suite ();
  SRunner *sr = srunner_create (s);
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
