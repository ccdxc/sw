#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct p4plus_app_prep_k k;
struct phv_              p;

%%

p4plus_app_prep:
  sne         c1, k.control_metadata_p4plus_app_id, P4PLUS_APPTYPE_CLASSIC_NIC
  nop.c1.e

  // r1 : flags
  or          r1, r0, CLASSIC_NIC_FLAGS_FCS_OK
  seq         c1, k.inner_ipv4_valid, TRUE
  seq         c2, k.inner_ipv6_valid, TRUE
  bcf         [!c1&!c2], p4plus_app_classic_nic_native
  phvwr.c1    p.p4_to_p4plus_classic_nic_ip_proto, k.inner_ipv4_protocol
  phvwr       p.p4_to_p4plus_classic_nic_inner_ip_valid, TRUE
  bcf         [c1], p4plus_app_classic_nic_l4
  or.c1       r1, r1, (CLASSIC_NIC_FLAGS_IPV4_VALID|CLASSIC_NIC_FLAGS_TUNNELED)
  phvwr.c2    p.p4_to_p4plus_classic_nic_ip_proto, k.inner_ipv6_nextHdr
  b           p4plus_app_classic_nic_l4
  or.c2       r1, r1, (CLASSIC_NIC_FLAGS_IPV6_VALID|CLASSIC_NIC_FLAGS_TUNNELED)

p4plus_app_classic_nic_native:
  phvwr       p.p4_to_p4plus_classic_nic_ip_valid, TRUE
  seq         c1, k.ipv4_valid, TRUE
  phvwr.c1    p.p4_to_p4plus_classic_nic_ip_proto, k.ipv4_protocol
  bcf         [c1], p4plus_app_classic_nic_l4
  or.c1       r1, r1, CLASSIC_NIC_FLAGS_IPV4_VALID
  seq         c2, k.ipv6_valid, TRUE
  phvwr.c2    p.p4_to_p4plus_classic_nic_ip_proto, k.ipv6_nextHdr
  or.c2       r1, r1, CLASSIC_NIC_FLAGS_IPV6_VALID

p4plus_app_classic_nic_l4:
  seq         c1, k.tcp_valid, TRUE
  bcf         [!c1], p4plus_app_classic_nic_l4_inner_udp
  phvwr.c1    p.p4_to_p4plus_classic_nic_l4_sport, k.tcp_srcPort
  phvwr.c1    p.p4_to_p4plus_classic_nic_l4_dport, k.tcp_dstPort
  phvwr.e     p.p4_to_p4plus_classic_nic_l4_checksum, k.tcp_checksum
  phvwr       p.p4_to_p4plus_classic_nic_flags, r1

p4plus_app_classic_nic_l4_inner_udp:
  seq         c1, k.inner_udp_valid, TRUE
  bcf         [!c1], p4plus_app_classic_nic_l4_udp
  phvwr.c1    p.p4_to_p4plus_classic_nic_l4_sport, k.inner_udp_srcPort
  phvwr.c1    p.p4_to_p4plus_classic_nic_l4_dport, k.inner_udp_dstPort
  phvwr.e     p.p4_to_p4plus_classic_nic_l4_checksum, k.inner_udp_checksum
  phvwr       p.p4_to_p4plus_classic_nic_flags, r1

p4plus_app_classic_nic_l4_udp:
  seq         c1, k.udp_valid, TRUE
  bcf         [!c1], p4plus_app_classic_nic_l4_unknown
  phvwr.c1    p.p4_to_p4plus_classic_nic_l4_sport, k.udp_srcPort
  phvwr.c1    p.p4_to_p4plus_classic_nic_l4_dport, k.udp_dstPort
  phvwr.e     p.p4_to_p4plus_classic_nic_l4_checksum, k.udp_checksum
  phvwr       p.p4_to_p4plus_classic_nic_flags, r1

p4plus_app_classic_nic_l4_unknown:
  phvwr.e     p.p4_to_p4plus_classic_nic_flags, r1
  nop
