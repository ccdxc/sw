#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/iris/include/defines.h"

struct twice_nat_k k;
struct twice_nat_d d;
struct phv_        p;

d = {
  u.twice_nat_rewrite_info_d.ip = 192.168.100.100;
  u.twice_nat_rewrite_info_d.l4_port = 0x8080;
};
