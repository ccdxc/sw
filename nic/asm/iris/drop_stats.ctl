#include "ingress.h"
#include "INGRESS_p.h"

struct drop_stats_d d;
struct phv_         p;

d = {
  drop_stats_d.drop_pkts = 0xf;
  drop_stats_d.mirror_session_id = 0x08;
};
