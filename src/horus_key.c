
#include <horus.h>

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

#ifdef DEBUG
  for (i = 0; i < 10; i++)
    printf ("block_size[%d] = %10d\n", i, block_size[i]);
#endif /*DEBUG*/

  for (i = 0; i < 10; i++)
    {
      x = i;
      y = offset / block_size[x];

#ifdef NON_SHA1_ROOT_KEY
      if (x != 0)
        {
#endif /*NON_SHA1_ROOT_KEY*/

          /* calculate K_(x,y). */
          block_key (key, &key_len, parent, parent_len, x, y);

#ifdef NON_SHA1_ROOT_KEY
        }
#endif /*NON_SHA1_ROOT_KEY*/

      printf ("K_(%d,%d) = %s(%d) [%010d-%010d]\n",
               x, y, print_key (key, key_len), key_len,
               block_size[x] * y, block_size[x] * (y + 1) -1);

      /* parent = str(key) */
      snprintf (parent, sizeof (parent), "%s", print_key (key, key_len));
      parent_len = strlen (parent); // will be 40.
    }

  return 0;
}


