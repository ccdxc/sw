#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct p4plus_app_prep_k k;
struct phv_              p;

k = {
  ipv4_valid = 1;
  tcp_valid = 1;
  tcp_srcPort = 0x64;
  tcp_dstPort = 0x8080;
};

p = {
  ethernet_valid = 1;
  ipv4_valid = 1;
  tcp_valid = 1;
};
