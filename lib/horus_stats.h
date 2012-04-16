
#ifndef _HORUS_STATS_H_
#define _HORUS_STATS_H_

#include <kds_protocol.h>

struct horus_stats {
  unsigned long long total;
  unsigned long long errors[HORUS_ERR_MAX];
  unsigned long long sendfail;
  unsigned long long sendretry;
  unsigned long long recvfail;
  unsigned long long recvretry;;
  unsigned long long success;
  unsigned long long giveup;
  unsigned long long resmismatch;
  unsigned long long keycalculated;
};

void horus_stats_record (struct horus_stats *stats, int err, int suberr);
void horus_stats_merge (struct horus_stats *res, struct horus_stats *delta);
void horus_stats_print (struct horus_stats *stats);

#endif /*_HORUS_STATS_H_*/

