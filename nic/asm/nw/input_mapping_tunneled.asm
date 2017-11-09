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
  seq         c1, k.inner_ethernet_dstAddr[40], 0
  bcf         [c1], tunneled_ipv4_packet_common
  phvwr.c1    p.flow_lkp_metadata_pkt_type, PACKET_TYPE_UNICAST
  xor         r6, -1, r0
  seq         c2, k.inner_ethernet_dstAddr, r6[47:0]
  cmov.!c1    r7, c2, PACKET_TYPE_BROADCAST, PACKET_TYPE_MULTICAST
  phvwr       p.flow_lkp_metadata_pkt_type, r7

tunneled_ipv4_packet_common:
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
  seq         c1, k.inner_ethernet_dstAddr[40], 0
  bcf         [c1], tunneled_ipv6_packet_common
  phvwr.c1    p.flow_lkp_metadata_pkt_type, PACKET_TYPE_UNICAST
  xor         r6, -1, r0
  seq         c2, k.inner_ethernet_dstAddr, r6[47:0]
  cmov.!c1    r7, c2, PACKET_TYPE_BROADCAST, PACKET_TYPE_MULTICAST
  phvwr       p.flow_lkp_metadata_pkt_type, r7

tunneled_ipv6_packet_common:
  or          r7, k.inner_ipv6_payloadLen, \
                  k.inner_ipv6_payloadLen, 0
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
  seq         c1, k.inner_ethernet_dstAddr[40], 0
  bcf         [c1], tunneled_non_ip_packet_common
  phvwr.c1    p.flow_lkp_metadata_pkt_type, PACKET_TYPE_UNICAST
  xor         r6, -1, r0
  seq         c2, k.inner_ethernet_dstAddr, r6[47:0]
  cmov.!c1    r7, c2, PACKET_TYPE_BROADCAST, PACKET_TYPE_MULTICAST
  phvwr       p.flow_lkp_metadata_pkt_type, r7

tunneled_non_ip_packet_common:
  phvwr       p.tunnel_metadata_tunnel_terminate, 1
  phvwr       p.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_MAC
  phvwr       p.flow_lkp_metadata_lkp_dst, k.inner_ethernet_dstAddr
  phvwr       p.flow_lkp_metadata_lkp_src, k.inner_ethernet_srcAddr
  phvwr       p.flow_lkp_metadata_lkp_sport, k.inner_ethernet_etherType
  phvwr.e     p.flow_lkp_metadata_lkp_srcMacAddr, k.inner_ethernet_srcAddr
  phvwr       p.flow_lkp_metadata_lkp_dstMacAddr, k.inner_ethernet_dstAddr

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tunneled_vm_bounce_packet:
  nop.e
  nop
