
#include <openssl.h>
#include <minmax.h>

int
main (int argc, char **argv)
{
  int ret;
  EVP_PKEY *pkey;
  RSA *rsa;
  int keysize;
  unsigned char ibuf[4098], obuf[4098];
  int isize;
  FILE *fp;

  if (argc <= 2)
    {
      printf ("%s <key> <signed> [<pass>]\n", argv[0]);
      exit (1);
    }

  if (argc > 3)
    pkey = openssl_load_private_key (argv[1], argv[3]);
  else
    pkey = openssl_load_private_key (argv[1], NULL);

  rsa = EVP_PKEY_get1_RSA (pkey);
  EVP_PKEY_free (pkey);

  keysize = RSA_size (rsa);
  printf ("RSA_size: %d\n", keysize);

  isize = MIN (sizeof (ibuf), keysize);
  printf ("Read size: %d\n", isize);

  printf ("reading: %s\n", argv[2]);
  fp = fopen (argv[2], "r");
  fread (ibuf, isize, 1, fp);
  printf ("Input size: %d\n", isize);

  ret = RSA_public_decrypt (isize, ibuf, obuf, rsa, RSA_PKCS1_PADDING);
  if (ret < 0)
    {
      printf ("decrypt failed\n");
      ERR_print_errors_fp (stderr);
      exit (-1);
    }

  printf ("Decrypted: (%d bytes from %d bytes)\n", ret, isize);
  fwrite (obuf, ret, 1, stdout);
  printf ("\nEnd.\n");

  exit (ret);
}
