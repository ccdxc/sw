#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct registered_macs_otcam_k k;
struct registered_macs_otcam_d d;
struct phv_                    p;

d = {
  u.registered_macs_otcam_hit_d.dst_lif = 0x7aa;
};
