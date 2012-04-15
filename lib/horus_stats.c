
#include <stdio.h>
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
horus_stats_merge (struct horus_stats *res, struct horus_stats *delta)
{
  int i;
  res->total += delta->total;
  for (i = 0; i < HORUS_ERR_MAX; i++)
    res->errors[i] += delta->errors[i];
  res->sendfail += delta->sendfail;
  res->sendretry += delta->sendretry;
  res->recvfail += delta->recvfail;
  res->recvretry += delta->recvretry;
  res->success += delta->success;
  res->giveup += delta->giveup;
  res->resmismatch += delta->resmismatch;
}

void
horus_stats_print (struct horus_stats *stats)
{
  printf (" total: %llu", stats->total);
  printf (" noerror: %llu", stats->errors[HORUS_ERR_NONE]);
  printf (" eopen: %llu", stats->errors[HORUS_ERR_OPEN]);
  printf (" egetconfig: %llu", stats->errors[HORUS_ERR_CONFIG_GET]);
  printf (" esetconfig: %llu", stats->errors[HORUS_ERR_CONFIG_SET]);
  printf ("\n");
  printf (" enosuchclient: %llu", stats->errors[HORUS_ERR_NO_SUCH_CLIENT]);
  printf (" ereqoutofrange: %llu", stats->errors[HORUS_ERR_REQ_OUT_OF_RANGE]);
  printf (" ereqnotallowed: %llu", stats->errors[HORUS_ERR_REQ_NOT_ALLOWED]);
  printf (" eajustedx: %llu", stats->errors[HORUS_ERR_X_ADJUSTING]);
  printf (" eunknown: %llu", stats->errors[HORUS_ERR_UNKNOWN]);
  printf ("\n");
  printf (" sendfail: %llu", stats->sendfail);
  printf (" sendretry: %llu", stats->sendretry);
  printf (" recvfail: %llu", stats->recvfail);
  printf (" recvretry: %llu", stats->recvretry);
  printf ("\n");
  printf (" success: %llu", stats->success);
  printf (" giveup: %llu", stats->giveup);
  printf (" resmismatch: %llu", stats->resmismatch);
  printf ("\n");
}

