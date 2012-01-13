
#include <stdio.h>
#include <stdlib.h>

#include <openssl/err.h>
#include <openssl/pem.h>

#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif /*MIN*/

int
load_openssl_private_key (char *filename, char *pass)
{
  int ret;
  EVP_PKEY *pkey;
  FILE *fp;

  if (filename)
    fprintf (stderr, "filename: %s\n", filename);

  fp = fopen (filename, "r");
  if (fp == NULL)
    {
      fprintf (stderr, "failed to open private key %s\n", filename);
      return -1;
    }

  if (pass)
    fprintf (stderr, "pass: %s\n", pass);

  pkey = PEM_read_PrivateKey (fp, NULL, NULL, pass);
  if (pkey == NULL)
    {
      fprintf (stderr, "failed to read private key %s\n", filename);
      ERR_print_errors_fp (stderr);
      return -1;
    }

  ret = PEM_write_PrivateKey (stdout, pkey, NULL, NULL, 0, 0, NULL);
  if (ret == 0)
    {
      fprintf (stderr, "failed to write private key %s\n", filename);
      return -1;
    }

  return 0;
}

int
main (int argc, char **argv)
{
  int ret;

  if (argc <= 1)
    {
      printf ("%s <key_filename> [<pass>]\n", argv[0]);
      exit (1);
    }

  if (argc > 2)
    ret = load_openssl_private_key (argv[1], argv[2]);
  else
    ret = load_openssl_private_key (argv[1], NULL);

  exit (ret);
}

