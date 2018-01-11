#include "egress.h"
#include "EGRESS_p.h"
#include "CSUM_INGRESS.h"
#include "../../p4/nw/include/defines.h"

struct p4plus_app_prep_k k;
struct phv_              p;

%%

p4plus_app_prep:
  sne         c1, k.control_metadata_p4plus_app_id, P4PLUS_APPTYPE_CLASSIC_NIC
  nop.c1.e

  seq         c1, k.inner_ipv4_valid, TRUE
  seq         c2, k.inner_ipv6_valid, TRUE

  // l2 checksum (CHECKSUM_COMPLETE)
  phvwrpair   p.ipv4_l2csum, k.ipv4_valid, p.ipv6_l2csum, k.ipv6_valid
  phvwr       p.p4_to_p4plus_classic_nic_l2csum, TRUE

  // checksum level
  setcf       c3, [c1|c2]
  seq         c4, k.control_metadata_checksum_results[csum_hdr_udp], TRUE
  seq.!c3     c4, k.control_metadata_checksum_results[csum_hdr_tcp], TRUE
  phvwr.c4    p.p4_to_p4plus_classic_nic_csum_level, 1
  seq.c3      c5, k.control_metadata_checksum_results[csum_hdr_tcp], TRUE
  setcf       c4, [c4&c5]
  phvwr.c4    p.p4_to_p4plus_classic_nic_csum_level, 2

  seq         c3, k.inner_ethernet_valid, TRUE
  bcf         [c1|c2|c3], p4plus_app_classic_nic_tunneled
  seq         c4, k.ipv4_valid, TRUE
  bcf         [c4], p4plus_app_classic_nic_native_ipv4_tcp
  phvwr       p.p4_to_p4plus_classic_nic_ip_valid, TRUE
  seq         c4, k.ipv6_valid, TRUE
  bcf         [c4], p4plus_app_classic_nic_native_ipv6_tcp
  nop.!c4.e
  nop

p4plus_app_classic_nic_native_ipv4_tcp:
  seq         c1, k.tcp_valid, TRUE
  bcf         [!c1], p4plus_app_classic_nic_native_ipv4_udp
  phvwr.c1    p.{p4_to_p4plus_classic_nic_l4_sport, \
                 p4_to_p4plus_classic_nic_l4_dport}, k.{tcp_srcPort,tcp_dstPort}
  phvwr.e     p.p4_to_p4plus_classic_nic_header_flags, CLASSIC_NIC_HEADER_FLAGS_IPV4_TCP
  nop

p4plus_app_classic_nic_native_ipv4_udp:
  seq         c1, k.udp_valid, TRUE
  bcf         [!c1], p4plus_app_classic_nic_ipv4
  phvwr.c1    p.{p4_to_p4plus_classic_nic_l4_sport, \
                 p4_to_p4plus_classic_nic_l4_dport}, k.{udp_srcPort,udp_dstPort}
  phvwr.e     p.p4_to_p4plus_classic_nic_header_flags, CLASSIC_NIC_HEADER_FLAGS_IPV4_UDP
  nop

p4plus_app_classic_nic_native_ipv6_tcp:
  seq         c1, k.tcp_valid, TRUE
  bcf         [!c1], p4plus_app_classic_nic_native_ipv6_udp
  phvwr.c1    p.{p4_to_p4plus_classic_nic_l4_sport, \
                 p4_to_p4plus_classic_nic_l4_dport}, k.{tcp_srcPort,tcp_dstPort}
  phvwr.e     p.p4_to_p4plus_classic_nic_header_flags, CLASSIC_NIC_HEADER_FLAGS_IPV6_TCP
  nop

p4plus_app_classic_nic_native_ipv6_udp:
  seq         c1, k.udp_valid, TRUE
  bcf         [!c1], p4plus_app_classic_nic_ipv6
  phvwr.c1    p.{p4_to_p4plus_classic_nic_l4_sport, \
                 p4_to_p4plus_classic_nic_l4_dport}, k.{udp_srcPort,udp_dstPort}
  phvwr.e     p.p4_to_p4plus_classic_nic_header_flags, CLASSIC_NIC_HEADER_FLAGS_IPV6_UDP
  nop

p4plus_app_classic_nic_tunneled:
  bcf         [c1], p4plus_app_classic_nic_tunneled_ipv4_tcp
  phvwr       p.p4_to_p4plus_classic_nic_inner_ip_valid, TRUE
  bcf         [c2], p4plus_app_classic_nic_tunneled_ipv6_tcp
  nop.!c2.e
  nop

p4plus_app_classic_nic_tunneled_ipv4_tcp:
  seq         c1, k.tcp_valid, TRUE
  bcf         [!c1], p4plus_app_classic_nic_tunneled_ipv4_udp
  phvwr.c1    p.{p4_to_p4plus_classic_nic_l4_sport, \
                 p4_to_p4plus_classic_nic_l4_dport}, k.{tcp_srcPort,tcp_dstPort}
  phvwr.e     p.p4_to_p4plus_classic_nic_header_flags, CLASSIC_NIC_HEADER_FLAGS_IPV4_TCP
  nop

p4plus_app_classic_nic_tunneled_ipv4_udp:
  seq         c1, k.inner_udp_valid, TRUE
  bcf         [!c1], p4plus_app_classic_nic_ipv4
  phvwr.c1    p.{p4_to_p4plus_classic_nic_l4_sport, \
                 p4_to_p4plus_classic_nic_l4_dport}, k.{inner_udp_srcPort,inner_udp_dstPort}
  phvwr.e     p.p4_to_p4plus_classic_nic_header_flags, CLASSIC_NIC_HEADER_FLAGS_IPV4_UDP
  nop

p4plus_app_classic_nic_tunneled_ipv6_tcp:
  seq         c1, k.tcp_valid, TRUE
  bcf         [!c1], p4plus_app_classic_nic_tunneled_ipv6_udp
  phvwr.c1    p.{p4_to_p4plus_classic_nic_l4_sport, \
                 p4_to_p4plus_classic_nic_l4_dport}, k.{tcp_srcPort,tcp_dstPort}
  phvwr.e     p.p4_to_p4plus_classic_nic_header_flags, CLASSIC_NIC_HEADER_FLAGS_IPV6_TCP
  nop

p4plus_app_classic_nic_tunneled_ipv6_udp:
  seq         c1, k.inner_udp_valid, TRUE
  bcf         [!c1], p4plus_app_classic_nic_ipv6
  phvwr.c1    p.{p4_to_p4plus_classic_nic_l4_sport, \
                 p4_to_p4plus_classic_nic_l4_dport}, k.{inner_udp_srcPort,inner_udp_dstPort}
  phvwr.e     p.p4_to_p4plus_classic_nic_header_flags, CLASSIC_NIC_HEADER_FLAGS_IPV6_UDP
  nop

p4plus_app_classic_nic_ipv4:
  phvwr.e     p.p4_to_p4plus_classic_nic_header_flags, CLASSIC_NIC_HEADER_FLAGS_IPV4
  nop

p4plus_app_classic_nic_ipv6:
  phvwr.e     p.p4_to_p4plus_classic_nic_header_flags, CLASSIC_NIC_HEADER_FLAGS_IPV6
  nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
p4plus_app_prep_error:
  nop.e
  nop
