#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct registered_macs_k k;
struct registered_macs_d d;
struct phv_              p;

d = {
  u.registered_macs_hit_d.dst_lif = 0x7aa;
};
