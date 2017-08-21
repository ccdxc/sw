#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct tunnel_decap_copy_inner_k k;
struct tunnel_decap_copy_inner_d d;
struct phv_                      p;

k = {
  inner_ipv4_version = 4;
  inner_ipv4_srcAddr = 192.168.100.1;
  inner_ipv4_dstAddr = 0x0c0d0a0b;
  inner_udp_srcPort = 0xbaba;
  inner_udp_checksum = 0xc0c0;
  inner_ipv4_valid = 1;
  inner_udp_valid = 1;
};
