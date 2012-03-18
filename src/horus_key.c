
#include <horus.h>

#include <horus_key.h>

unsigned int block_size[] = {
  MIN_CHUNK_SIZE << (BRANCH_FACTOR_BITS * 9),
  MIN_CHUNK_SIZE << (BRANCH_FACTOR_BITS * 8),
  MIN_CHUNK_SIZE << (BRANCH_FACTOR_BITS * 7),
  MIN_CHUNK_SIZE << (BRANCH_FACTOR_BITS * 6),
  MIN_CHUNK_SIZE << (BRANCH_FACTOR_BITS * 5),
  MIN_CHUNK_SIZE << (BRANCH_FACTOR_BITS * 4),
  MIN_CHUNK_SIZE << (BRANCH_FACTOR_BITS * 3),
  MIN_CHUNK_SIZE << (BRANCH_FACTOR_BITS * 2),
  MIN_CHUNK_SIZE << (BRANCH_FACTOR_BITS * 1),
  MIN_CHUNK_SIZE << (BRANCH_FACTOR_BITS * 0),
};

char *progname;

void
usage ()
{
  printf ("%s [offset] [master_key]\n", progname);
  exit (0);
}

int
main (int argc, char **argv)
{
  int i;
  int x, y;
  int offset = 4096;
  char *master = "master";

  char parent[SHA_DIGEST_LENGTH * 2 + 1];
  char key[SHA_DIGEST_LENGTH];
  int parent_len, key_len;

  progname = argv[0];

  /* Help */
  if (argc > 1)
    {
      if (! strcmp (argv[1], "-h"))
        usage ();
      if (! strcmp (argv[1], "--help"))
        usage ();
      if (! strcmp (argv[1], "-d"))
        debug++;
      if (! strcmp (argv[1], "--debug"))
        debug++;
    }

  /* Byte offset */
  if (argc > 1)
    offset = atoi (argv[1]);

  /* Master key */
  if (argc > 2)
    master = argv[2];
  snprintf (parent, sizeof (parent), "%s", master);
  parent_len = strlen (master);

  printf ("offset = %010d, master = %s(%d).\n",
          offset, print_key (parent, parent_len), parent_len);

  /* Out of bound offset. */
  if (offset >= block_size[0])
    {
      printf ("offset out of bound: %d >= %d\n", offset, block_size[0]);
      exit (1);
    }

  if (debug)
    {
      for (i = 0; i < 10; i++)
        printf ("block_size[%d] = %10d\n", i, block_size[i]);
    }

  x = 9;
  y = offset / block_size[9];

  horus_key_by_master (key, &key_len, x, y, parent, parent_len);

  printf ("K_(%d,%d) = %s(%d) [%010d-%010d]\n",
          x, y, print_key (key, key_len), key_len,
          block_size[x] * y, block_size[x] * (y + 1) -1);

  return 0;
}



