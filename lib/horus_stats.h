
#ifndef _HORUS_STATS_H_
#define _HORUS_STATS_H_

#include <kds_protocol.h>

struct horus_stats {
  unsigned long long total;
  unsigned long long errors[HORUS_ERR_MAX];
};

void
horus_stats_record (struct horus_stats *stats, int err, int suberr);
void
horus_stats_print (struct horus_stats *stats);

#endif /*_HORUS_STATS_H_*/

