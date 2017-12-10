#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct p4plus_to_p4_2_k k;
struct phv_             p;

k = {
  p4plus_to_p4_flags = (P4PLUS_TO_P4_FLAGS_UPDATE_IP_LEN | P4PLUS_TO_P4_FLAGS_UPDATE_UDP_LEN);
  ipv4_ihl = 5;
  ipv4_valid = 1;
};
