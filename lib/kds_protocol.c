
#include <stdio.h>
#include <sys/types.h>
#include <kds_protocol.h>
#include <assert.h>

char *horus_error_strings [] = {
  "horus: no error",
  "horus: error in opening file",
  "horus: error in getting config",
  "horus: error in setting config",
  "horus: no such client",
  "horus: request out of range",
  "horus: request not allowed",
  "horus: unknown error",
};

char *
horus_strerror (u_int16_t err)
{
  assert (err < HORUS_ERR_MAX);
  return horus_error_strings[err];
}


