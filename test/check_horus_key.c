/*
 * Test Cases for Horus Key Utils
 *
 * Copyright (c) 2012, University of California, Santa Cruz, CA, USA.
 * Developers:
 *  Yan Li <yanli@ucsc.edu>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as 
 * published by the Free Software Foundation.
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
