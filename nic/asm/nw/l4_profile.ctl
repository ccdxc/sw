#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct l4_profile_k k;
struct l4_profile_d d;
struct phv_         p;

k = {
  inner_ipv4_srcAddr = 192.168.1.1;
  inner_ipv4_dstAddr = 192.168.1.2;
  tunnel_metadata_tunnel_terminate = 1;
};
