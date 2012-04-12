
#include <sys/types.h>
#include <kds_protocol.h>
#include <horus_stats.h>

void
horus_stats_record (struct horus_stats *stats, int err, int suberr)
{
  stats->total++;
  if (err < HORUS_ERR_MAX)
    stats->errors[err]++;
  else
    stats->errors[HORUS_ERR_UNKNOWN]++;
}

void
horus_stats_print (struct horus_stats *stats)
{
  printf (" total: %llu", stats->total);
  printf (" success: %llu", stats->errors[HORUS_ERR_NONE]);
  printf (" eopen: %llu", stats->errors[HORUS_ERR_OPEN]);
  printf (" egetconfig: %llu", stats->errors[HORUS_ERR_CONFIG_GET]);
  printf (" esetconfig: %llu", stats->errors[HORUS_ERR_CONFIG_SET]);
  printf ("\n");
  printf (" enosuchclient: %llu", stats->errors[HORUS_ERR_NO_SUCH_CLIENT]);
  printf (" ereqoutofrange: %llu", stats->errors[HORUS_ERR_REQ_OUT_OF_RANGE]);
  printf (" ereqnotallowed: %llu", stats->errors[HORUS_ERR_REQ_NOT_ALLOWED]);
  printf (" eunknown: %llu", stats->errors[HORUS_ERR_UNKNOWN]);
  printf ("\n");
}

