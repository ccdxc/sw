#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct input_mapping_tunneled_k k;
struct phv_                     p;

%%

nop:
  nop.e
  nop

.align
tunneled_ipv4_packet:
  xor         r6, -1, r0

  seq         c1, k.inner_ethernet_srcAddr, r0
  seq         c2, k.inner_ethernet_dstAddr, r0
  seq         c3, k.inner_ethernet_srcAddr[40], 1
  bcf         [c1|c2|c3], malformed_tunneled_packet

  sne         c1, k.inner_ipv4_version, 4
  seq         c2, k.inner_ipv4_ttl, 0
  bcf         [c1|c2], malformed_tunneled_packet

  seq         c1, k.inner_ethernet_dstAddr[40], 0
  add.c1      r7, r0, PACKET_TYPE_UNICAST
  seq         c2, k.inner_ethernet_dstAddr, r6[47:0]
  cmov.!c1    r7, c2, PACKET_TYPE_BROADCAST, PACKET_TYPE_MULTICAST
  phvwr       p.flow_lkp_metadata_pkt_type, r7

  add         r6, k.inner_ipv4_ihl, k.tcp_dataOffset
  sub         r7, k.inner_ipv4_totalLen, r6, 2
  phvwr       p.l4_metadata_tcp_data_len, r7

  phvwr       p.tunnel_metadata_tunnel_terminate, 1
  phvwr       p.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV4
  phvwr       p.flow_lkp_metadata_lkp_src, k.inner_ipv4_srcAddr
  phvwr       p.flow_lkp_metadata_lkp_dst, k.inner_ipv4_dstAddr
  phvwr       p.flow_lkp_metadata_lkp_proto, k.inner_ipv4_protocol
  phvwr       p.flow_lkp_metadata_ipv4_flags, k.inner_ipv4_flags
  phvwr       p.flow_lkp_metadata_ipv4_frag_offset, k.{inner_ipv4_fragOffset_sbit0_ebit4,inner_ipv4_fragOffset_sbit5_ebit12}
  phvwr       p.flow_lkp_metadata_ipv4_hlen, k.inner_ipv4_ihl
  phvwr       p.flow_lkp_metadata_ip_ttl, k.inner_ipv4_ttl
  phvwr       p.l3_metadata_ip_option_seen, k.l3_metadata_inner_ip_option_seen

  phvwr.e     p.flow_lkp_metadata_lkp_srcMacAddr, k.inner_ethernet_srcAddr
  phvwr       p.flow_lkp_metadata_lkp_dstMacAddr, k.inner_ethernet_dstAddr

.align
tunneled_ipv6_packet:
  seq         c1, k.inner_ethernet_srcAddr, r0
  seq         c2, k.inner_ethernet_dstAddr, r0
  seq         c3, k.inner_ethernet_srcAddr[40], 1
  bcf         [c1|c2|c3], malformed_tunneled_packet

  sne        c1, k.inner_ipv6_version, 6
  seq        c2, k.inner_ipv6_hopLimit, 0
  bcf        [c1|c2], malformed_tunneled_packet

  seq         c1, k.inner_ethernet_dstAddr[40], 0
  add.c1      r7, r0, PACKET_TYPE_UNICAST
  seq         c2, k.inner_ethernet_dstAddr, r6[47:0]
  cmov.!c1    r7, c2, PACKET_TYPE_BROADCAST, PACKET_TYPE_MULTICAST
  phvwr       p.flow_lkp_metadata_pkt_type, r7

  or          r7, k.inner_ipv6_payloadLen_sbit0_ebit7, \
                  k.inner_ipv6_payloadLen_sbit8_ebit15, 8
  sub         r6, r7, k.tcp_dataOffset, 2
  phvwr       p.l4_metadata_tcp_data_len, r6

  phvwr       p.tunnel_metadata_tunnel_terminate, 1
  phvwr       p.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV6
  phvwr       p.flow_lkp_metadata_lkp_proto, k.inner_ipv6_nextHdr
  phvwr       p.flow_lkp_metadata_ip_ttl, k.inner_ipv6_hopLimit
  add         r1, r7, 40
  phvwr       p.l4_metadata_tcp_data_len, r1

  phvwr.e     p.flow_lkp_metadata_lkp_srcMacAddr, k.inner_ethernet_srcAddr
  phvwr       p.flow_lkp_metadata_lkp_dstMacAddr, k.inner_ethernet_dstAddr

.align
tunneled_non_ip_packet:
  seq         c1, k.inner_ethernet_srcAddr, r0
  seq         c2, k.inner_ethernet_dstAddr, r0
  seq         c3, k.inner_ethernet_srcAddr[40], 1
  bcf         [c1|c2|c3], malformed_tunneled_packet

  seq         c1, k.inner_ethernet_dstAddr[40], 0
  add.c1      r7, r0, PACKET_TYPE_UNICAST
  seq         c2, k.inner_ethernet_dstAddr, r6[47:0]
  cmov.!c1    r7, c2, PACKET_TYPE_BROADCAST, PACKET_TYPE_MULTICAST
  phvwr       p.flow_lkp_metadata_pkt_type, r7

  phvwr       p.tunnel_metadata_tunnel_terminate, 1
  phvwr       p.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_MAC
  phvwr       p.flow_lkp_metadata_lkp_dst, k.inner_ethernet_dstAddr
  phvwr.e     p.flow_lkp_metadata_lkp_src, k.inner_ethernet_srcAddr
  phvwr       p.flow_lkp_metadata_lkp_sport, k.inner_ethernet_etherType

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tunneled_vm_bounce_packet:
  nop.e
  nop

malformed_tunneled_packet:
  phvwr.e     p.control_metadata_drop_reason[DROP_MALFORMED_PKT], 1
  phvwr       p.capri_intrinsic_drop, 1
