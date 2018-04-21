#include "egress.h"
#include "EGRESS_p.h"

struct egress_drop_stats_d d;
struct phv_                p;

d = {
  egress_drop_stats_d.drop_pkts = 0xf;
  egress_drop_stats_d.mirror_session_id = 0x08;
};
