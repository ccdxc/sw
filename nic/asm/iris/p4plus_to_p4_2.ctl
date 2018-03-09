#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/iris/include/defines.h"

struct p4plus_to_p4_2_k k;
struct phv_             p;

k = {
  p4plus_to_p4_update_udp_len = 1;
  ipv4_ihl = 5;
  ipv4_valid = 1;
};
