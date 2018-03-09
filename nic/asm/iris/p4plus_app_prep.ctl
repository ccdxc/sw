#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/iris/include/defines.h"

struct p4plus_app_prep_k k;
struct phv_              p;

k = {
  ipv4_valid = 1;
  udp_valid = 1;
  inner_ethernet_valid = 1;
  inner_ipv6_valid = 1;
  inner_udp_valid = 1;
  inner_udp_srcPort = 0x64;
  inner_udp_dstPort = 0x8080;
  control_metadata_p4plus_app_id = P4PLUS_APPTYPE_CLASSIC_NIC;
};
