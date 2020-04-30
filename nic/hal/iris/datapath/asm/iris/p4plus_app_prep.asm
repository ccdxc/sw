#include "egress.h"
#include "EGRESS_p.h"
#include "CSUM_INGRESS.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"

struct p4plus_app_prep_k k;
struct phv_              p;

%%

// c6 : IP fragment?

p4plus_app_prep:
  bbeq        k.control_metadata_same_if_check_failed, FALSE, p4plus_app_prep2
  seq         c1, k.control_metadata_p4plus_app_id, P4PLUS_APPTYPE_RDMA
  seq.c1      c1, k.flow_lkp_metadata_pkt_type, PACKET_TYPE_MULTICAST
  // XXX: The following line to be removed when RDMA multicast test is fixed
  sne         c1, r0, r0
  phvwr.!c1   p.capri_intrinsic_lif, 0
  phvwr.!c1.e p.control_metadata_egress_drop_reason[EGRESS_DROP_PRUNE_SRC_PORT], 1
  phvwr.!c1   p.capri_intrinsic_drop, 1

p4plus_app_prep2:
  sne         c1, k.control_metadata_p4plus_app_id, P4PLUS_APPTYPE_CLASSIC_NIC
  bcf         [c1], p4plus_app_non_classic
  phvwr.!c1   p.p4_to_p4plus_classic_nic_l2_pkt_type, \
                k.flow_lkp_metadata_pkt_type

  seq         c1, k.inner_ipv4_valid, TRUE
  seq         c2, k.inner_ipv6_valid, TRUE
  seq         c6, k.control_metadata_i2e_flags[P4_I2E_FLAGS_IP_FRAGMENT], TRUE

  seq         c3, k.inner_ethernet_valid, TRUE
  bcf         [c1|c2|c3], p4plus_app_classic_nic_tunneled
  seq         c4, k.ipv4_valid, TRUE
  bcf         [c4], p4plus_app_classic_nic_native_ipv4_tcp
  phvwr       p.p4_to_p4plus_classic_nic_ip_valid, TRUE
  seq         c4, k.ipv6_valid, TRUE
  bcf         [c4], p4plus_app_classic_nic_native_ipv6_tcp
  add         r7, r0, r0
  nop.e
  nop

p4plus_app_classic_nic_native_ipv4_tcp:
  seq         c7, k.control_metadata_checksum_results[csum_hdr_ipv4], TRUE
  cmov        r7, c7, 0x20, 0x10
  seq         c1, k.tcp_valid, TRUE
  bcf         [!c1], p4plus_app_classic_nic_native_ipv4_udp
  seq         c7, k.control_metadata_checksum_results[csum_hdr_tcp], TRUE
  cmov        r6, c7, 0x2, 0x1
  or.!c6      r7, r7, r6
  phvwr       p.{p4_to_p4plus_classic_nic_csum_ip_bad...p4_to_p4plus_classic_nic_csum_tcp_ok}, r7
  phvwr.e     p.{p4_to_p4plus_classic_nic_l4_sport, \
                 p4_to_p4plus_classic_nic_l4_dport}, k.{tcp_srcPort,tcp_dstPort}
  phvwr.f     p.p4_to_p4plus_classic_nic_pkt_type, CLASSIC_NIC_PKT_TYPE_IPV4_TCP

p4plus_app_classic_nic_native_ipv4_udp:
  seq         c1, k.udp_valid, TRUE
  bcf         [!c1], p4plus_app_classic_nic_ipv4
  seq         c7, k.control_metadata_checksum_results[csum_hdr_udp], TRUE
  cmov        r6, c7, 0x2, 0x1
  or.!c6      r7, r7, r6, 2
  phvwr       p.{p4_to_p4plus_classic_nic_csum_ip_bad...p4_to_p4plus_classic_nic_csum_tcp_ok}, r7
  phvwr.e     p.{p4_to_p4plus_classic_nic_l4_sport, \
                 p4_to_p4plus_classic_nic_l4_dport}, k.{udp_srcPort,udp_dstPort}
  phvwr.f     p.p4_to_p4plus_classic_nic_pkt_type, CLASSIC_NIC_PKT_TYPE_IPV4_UDP

p4plus_app_classic_nic_native_ipv6_tcp:
  seq         c1, k.tcp_valid, TRUE
  bcf         [!c1], p4plus_app_classic_nic_native_ipv6_udp
  seq         c7, k.control_metadata_checksum_results[csum_hdr_tcp], TRUE
  cmov        r7, c7, 0x2, 0x1
  phvwr.!c6   p.{p4_to_p4plus_classic_nic_csum_tcp_bad...p4_to_p4plus_classic_nic_csum_tcp_ok}, r7
  phvwr.e     p.{p4_to_p4plus_classic_nic_l4_sport, \
                 p4_to_p4plus_classic_nic_l4_dport}, k.{tcp_srcPort,tcp_dstPort}
  phvwr.f     p.p4_to_p4plus_classic_nic_pkt_type, CLASSIC_NIC_PKT_TYPE_IPV6_TCP

p4plus_app_classic_nic_native_ipv6_udp:
  seq         c1, k.udp_valid, TRUE
  bcf         [!c1], p4plus_app_classic_nic_ipv6
  seq         c7, k.control_metadata_checksum_results[csum_hdr_udp], TRUE
  cmov        r7, c7, 0x2, 0x1
  phvwr.!c6   p.{p4_to_p4plus_classic_nic_csum_udp_bad,p4_to_p4plus_classic_nic_csum_udp_ok}, r7
  phvwr.e     p.{p4_to_p4plus_classic_nic_l4_sport, \
                 p4_to_p4plus_classic_nic_l4_dport}, k.{udp_srcPort,udp_dstPort}
  phvwr.f     p.p4_to_p4plus_classic_nic_pkt_type, CLASSIC_NIC_PKT_TYPE_IPV6_UDP

p4plus_app_classic_nic_tunneled:
  phvwr       p.p4_to_p4plus_classic_nic_encap_pkt, TRUE
  bcf         [c1], p4plus_app_classic_nic_tunneled_ipv4_tcp
  phvwr       p.p4_to_p4plus_classic_nic_inner_ip_valid, TRUE
  bcf         [c2], p4plus_app_classic_nic_tunneled_ipv6_tcp
  add         r7, r0, r0
  nop.e
  nop

p4plus_app_classic_nic_tunneled_ipv4_tcp:
  seq         c7, k.control_metadata_checksum_results[csum_hdr_inner_ipv4], TRUE
  seq.!c7     c7, k.control_metadata_checksum_results[csum_hdr_ipv4], TRUE
  cmov        r7, c7, 0x20, 0x10
  seq         c1, k.tcp_valid, TRUE
  bcf         [!c1], p4plus_app_classic_nic_tunneled_ipv4_udp
  seq         c7, k.control_metadata_checksum_results[csum_hdr_tcp], TRUE
  cmov        r6, c7, 0x2, 0x1
  or.!c6      r7, r7, r6
  phvwr       p.{p4_to_p4plus_classic_nic_csum_ip_bad...p4_to_p4plus_classic_nic_csum_tcp_ok}, r7
  phvwr.e     p.{p4_to_p4plus_classic_nic_l4_sport, \
                 p4_to_p4plus_classic_nic_l4_dport}, k.{tcp_srcPort,tcp_dstPort}
  phvwr.f     p.p4_to_p4plus_classic_nic_pkt_type, CLASSIC_NIC_PKT_TYPE_IPV4_TCP

p4plus_app_classic_nic_tunneled_ipv4_udp:
  seq         c1, k.inner_udp_valid, TRUE
  seq         c7, k.control_metadata_checksum_results[csum_hdr_inner_udp], TRUE
  cmov        r6, c7, 0x2, 0x1
  or.!c6      r7, r7, r6, 2
  phvwr       p.{p4_to_p4plus_classic_nic_csum_ip_bad...p4_to_p4plus_classic_nic_csum_tcp_ok}, r7
  bcf         [!c1], p4plus_app_classic_nic_ipv4
  phvwr.c1.e  p.{p4_to_p4plus_classic_nic_l4_sport, \
                 p4_to_p4plus_classic_nic_l4_dport}, k.{inner_udp_srcPort,inner_udp_dstPort}
  phvwr.f     p.p4_to_p4plus_classic_nic_pkt_type, CLASSIC_NIC_PKT_TYPE_IPV4_UDP

p4plus_app_classic_nic_tunneled_ipv6_tcp:
  seq         c7, k.control_metadata_checksum_results[csum_hdr_ipv4], TRUE
  cmov.c4     r7, c7, 0x20, 0x10
  seq         c1, k.tcp_valid, TRUE
  bcf         [!c1], p4plus_app_classic_nic_tunneled_ipv6_udp
  seq         c7, k.control_metadata_checksum_results[csum_hdr_tcp], TRUE
  cmov        r6, c7, 0x2, 0x1
  or.!c6      r7, r7, r6
  phvwr       p.{p4_to_p4plus_classic_nic_csum_ip_bad...p4_to_p4plus_classic_nic_csum_tcp_ok}, r7
  phvwr.e     p.{p4_to_p4plus_classic_nic_l4_sport, \
                 p4_to_p4plus_classic_nic_l4_dport}, k.{tcp_srcPort,tcp_dstPort}
  phvwr.f     p.p4_to_p4plus_classic_nic_pkt_type, CLASSIC_NIC_PKT_TYPE_IPV6_TCP

p4plus_app_classic_nic_tunneled_ipv6_udp:
  seq         c1, k.inner_udp_valid, TRUE
  bcf         [!c1], p4plus_app_classic_nic_ipv6
  seq         c7, k.control_metadata_checksum_results[csum_hdr_inner_udp], TRUE
  cmov        r6, c7, 0x2, 0x1
  or.!c6      r7, r7, r6, 2
  phvwr       p.{p4_to_p4plus_classic_nic_csum_ip_bad...p4_to_p4plus_classic_nic_csum_tcp_ok}, r7
  phvwr.e     p.{p4_to_p4plus_classic_nic_l4_sport, \
                 p4_to_p4plus_classic_nic_l4_dport}, k.{inner_udp_srcPort,inner_udp_dstPort}
  phvwr.f     p.p4_to_p4plus_classic_nic_pkt_type, CLASSIC_NIC_PKT_TYPE_IPV6_UDP

p4plus_app_classic_nic_ipv4:
  phvwr.e     p.{p4_to_p4plus_classic_nic_csum_ip_bad...p4_to_p4plus_classic_nic_csum_tcp_ok}, r7
  phvwr.f     p.p4_to_p4plus_classic_nic_pkt_type, CLASSIC_NIC_PKT_TYPE_IPV4

p4plus_app_classic_nic_ipv6:
  phvwr.e     p.{p4_to_p4plus_classic_nic_csum_ip_bad...p4_to_p4plus_classic_nic_csum_tcp_ok}, r7
  phvwr.f     p.p4_to_p4plus_classic_nic_pkt_type, CLASSIC_NIC_PKT_TYPE_IPV6

p4plus_app_non_classic:
  sne         c1, k.control_metadata_checksum_results, r0
  phvwr.c1    p.capri_intrinsic_drop, TRUE
  nop.e
  phvwr.c1    p.control_metadata_egress_drop_reason[EGRESS_DROP_CHECKSUM_ERR], 1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
p4plus_app_prep_error:
  nop.e
  nop
