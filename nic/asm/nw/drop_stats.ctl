#include "ingress.h"
#include "INGRESS_p.h"

struct drop_stats_d d;
struct phv_         p;

d = {
  u.drop_stats_d.stats_idx = 0xd;
  u.drop_stats_d.drop_pkts = 0xf;
  u.drop_stats_d.mirror_session_id = 0x08;
};

r5 = 0x7f001238;
