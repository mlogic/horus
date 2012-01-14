
#include <openssl.h>

#include <minmax.h>

struct cb_data
{
  char *password;
  char *filename;
};

int
password_callback (char *buf, int bufsiz, int verify, void *data)
{
  int ret;
  struct cb_data *cbd = (struct cb_data *) data;

  if (cbd && cbd->password)
    {
      ret = MIN (strlen (cbd->password), bufsiz);
      memcpy (buf, cbd->password, ret);
      return ret;
    }

  return 0;
}

EVP_PKEY *
openssl_load_private_key (char *filename, char *pass)
{
  //int ret;
  EVP_PKEY *pkey;
  FILE *fp;
  struct cb_data cb_data;

  if (filename)
    fprintf (stderr, "filename: %s\n", filename);

  fp = fopen (filename, "r");
  if (fp == NULL)
    {
      fprintf (stderr, "failed to open private key %s\n", filename);
      return NULL;
    }

  if (pass)
    fprintf (stderr, "pass: %s\n", pass);

  cb_data.password = pass;
  cb_data.filename = filename;

  pkey = PEM_read_PrivateKey (fp, NULL, password_callback, &cb_data);
  if (pkey == NULL)
    {
      fprintf (stderr, "failed to read private key %s\n", filename);
      ERR_print_errors_fp (stderr);
      return NULL;
    }

#if 0
  ret = PEM_write_PrivateKey (stdout, pkey, NULL, NULL, 0, 0, NULL);
  if (ret == 0)
    fprintf (stderr, "failed to write private key %s\n", filename);
#endif /*0*/

  return pkey;
}


