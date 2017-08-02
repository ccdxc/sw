#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct tunnel_encap_update_inner_k k;
struct phv_                        p;

k = {
  ipv4_version = 4;
  ipv4_srcAddr = 11.12.14.15;
  ipv4_dstAddr = 10.1.1.1;
  ipv4_totalLen = 0x256;
  udp_srcPort = 0xbaba;
  udp_dstPort = 0xdada;
  udp_checksum = 0x4545;
  ipv4_valid = 1;
  udp_valid = 1;
};
