/*
 * Test Cases for Horus Key Utils
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

#include <check.h>
#include "horus.h"

START_TEST (test_horus_set_get_key)
{
  // We can safely use a fake fd as long as we don't do real I/O
  const int fd = 9;
  const char key_2_0[] = "key_2_0";
  const char key_3_3[] = "key_3_3";
  char key_buf[HORUS_KEY_LEN];
  const int kht_branching_factor[2] = { 2, 2 };

  // Set a KHT with 3 levels
  #define KHT_DEPTH 3
  // Keyed Hash Tree:
  // Master:        K(0,0)
  // L1:            K(1,0)
  // L2:     K(2,0)        K(2,1)
  // L3: K(3,0) K(3,1) K(3,2) K(3,3)

  horus_set_kht (fd, 3, 4096, kht_branching_factor);
  // set K(2,0)
  horus_add_key (fd, key_2_0, strnlen (key_2_0, HORUS_KEY_LEN), 2, 0);

  // make sure Horus get the block size at each level right
  fail_unless (4096 == horus_get_block_size (fd, 3),
	       "block_size at level 3 not equals 4096");
  fail_unless (4096*2 == horus_get_block_size (fd, 2),
	       "block_size at level 2 not equals 4096*2");
  fail_unless (4096*2*2 == horus_get_block_size (fd, 1),
	       "block_size at level 1 not equals 4096*2*2");

  // we should be able to get K(3,0), K(3,1), but not K(3,2) nor K(3,3)
  fail_unless (0 == horus_get_key (fd, key_buf, 3, 0),
	       "Can't get K(3,0)");
  fail_unless (0 == horus_get_key (fd, key_buf, 3, 1),
	       "Can't get K(3,1)");
  fail_unless (0 != horus_get_key (fd, key_buf, 3, 2),
	       "Shouldn't be able to get K(3,2)");
  fail_unless (0 != horus_get_key (fd, key_buf, 3, 3),
	       "Shouldn't be able to get K(3,3)");

  // Add K(3,3), then we should be able to access K(3,3), K(3,2)
  horus_add_key (fd, key_3_3, strnlen (key_3_3, HORUS_KEY_LEN), 3, 3);
  fail_unless (0 == horus_get_key (fd, key_buf, 3, 0),
	       "Can't get K(3,0)");
  fail_unless (0 == horus_get_key (fd, key_buf, 3, 1),
	       "Can't get K(3,1)");
  fail_unless (0 != horus_get_key (fd, key_buf, 3, 2),
	       "Shouldn't be able to get K(3,2)");
  fail_unless (0 == horus_get_key (fd, key_buf, 3, 3),
	       "Can't get K(3,3) even after set 3,3");
  
  // Test horus_get_leaf_block_key
  fail_unless (0 == horus_get_leaf_block_key (fd, key_buf, 0),
	       "Can't get block key of leaf 0");
  fail_unless (0 == horus_get_leaf_block_key (fd, key_buf, 1),
	       "Can't get block key of leaf 1");
  fail_unless (0 != horus_get_leaf_block_key (fd, key_buf, 2),
	       "Shouldn't be able to get block key of leaf 2");
  fail_unless (0 == horus_get_leaf_block_key (fd, key_buf, 3),
	       "Can't get block key of leaf 3");
}
END_TEST

Suite *
horus_key_suite (void)
{
  Suite *s = suite_create ("Horus Key");

  /* Core test case */
  TCase *tc_core = tcase_create ("Core");
  tcase_add_test (tc_core, test_horus_set_get_key);
  suite_add_tcase (s, tc_core);

  return s;
}

int
main (void)
{
  int number_failed;
  Suite *s = horus_key_suite ();
  SRunner *sr = srunner_create (s);
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
