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
  bbeq          k.inner_ethernet_dstAddr[40], 0, tunneled_ipv4_packet_common
  phvwr         p.flow_lkp_metadata_pkt_type, PACKET_TYPE_UNICAST
  xor           r6, -1, r0
  seq           c2, k.inner_ethernet_dstAddr, r6[47:0]
  cmov          r7, c2, PACKET_TYPE_BROADCAST, PACKET_TYPE_MULTICAST
  phvwr         p.flow_lkp_metadata_pkt_type, r7

tunneled_ipv4_packet_common:
  add           r6, k.inner_ipv4_ihl, k.tcp_dataOffset
  sub           r7, k.inner_ipv4_totalLen, r6, 2
  phvwr         p.l4_metadata_tcp_data_len, r7

  phvwr         p.tunnel_metadata_tunnel_terminate, 1
  phvwr         p.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV4
  phvwrpair     p.flow_lkp_metadata_lkp_src[31:0], k.inner_ipv4_srcAddr, \
                    p.flow_lkp_metadata_lkp_dst[31:0], k.inner_ipv4_dstAddr
  phvwr         p.flow_lkp_metadata_lkp_proto, k.inner_ipv4_protocol
  phvwrpair     p.flow_lkp_metadata_ipv4_flags, k.inner_ipv4_flags, \
                    p.flow_lkp_metadata_ip_ttl, k.inner_ipv4_ttl
  phvwrpair     p.l3_metadata_ip_option_seen, k.l3_metadata_inner_ip_option_seen, \
                    p.l3_metadata_ip_frag, k.l3_metadata_inner_ip_frag
  phvwr         p.flow_lkp_metadata_ipv4_hlen, k.inner_ipv4_ihl

  phvwr.e       p.flow_lkp_metadata_lkp_srcMacAddr, k.inner_ethernet_srcAddr
  phvwr         p.flow_lkp_metadata_lkp_dstMacAddr, k.inner_ethernet_dstAddr

.align
tunneled_ipv6_packet:
  bbeq          k.inner_ethernet_dstAddr[40], 0, tunneled_ipv6_packet_common
  phvwr         p.flow_lkp_metadata_pkt_type, PACKET_TYPE_UNICAST
  xor           r6, -1, r0
  seq           c2, k.inner_ethernet_dstAddr, r6[47:0]
  cmov          r7, c2, PACKET_TYPE_BROADCAST, PACKET_TYPE_MULTICAST
  phvwr         p.flow_lkp_metadata_pkt_type, r7

tunneled_ipv6_packet_common:
  sub           r6, k.inner_ipv6_payloadLen, k.tcp_dataOffset, 2
  phvwr         p.l4_metadata_tcp_data_len, r6

  phvwr         p.tunnel_metadata_tunnel_terminate, 1
  phvwr         p.flow_lkp_metadata_lkp_dstMacAddr, k.inner_ethernet_dstAddr
  phvwr         p.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV6
  phvwr         p.flow_lkp_metadata_ip_ttl, k.inner_ipv6_hopLimit

  phvwrpair.e   p.flow_lkp_metadata_lkp_proto, k.l3_metadata_inner_ipv6_ulp, \
                    p.flow_lkp_metadata_lkp_srcMacAddr, k.inner_ethernet_srcAddr
  phvwr         p.flow_lkp_metadata_lkp_dstMacAddr, k.inner_ethernet_dstAddr

.align
tunneled_non_ip_packet:
  bbeq          k.inner_ethernet_dstAddr[40], 0, tunneled_non_ip_packet_common
  phvwr         p.flow_lkp_metadata_pkt_type, PACKET_TYPE_UNICAST
  xor           r6, -1, r0
  seq           c2, k.inner_ethernet_dstAddr, r6[47:0]
  cmov          r7, c2, PACKET_TYPE_BROADCAST, PACKET_TYPE_MULTICAST
  phvwr         p.flow_lkp_metadata_pkt_type, r7

tunneled_non_ip_packet_common:
  phvwr         p.tunnel_metadata_tunnel_terminate, 1
  phvwr         p.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_MAC
  phvwrpair     p.flow_lkp_metadata_lkp_src[47:0], k.inner_ethernet_srcAddr, \
                    p.flow_lkp_metadata_lkp_dst[47:0], k.inner_ethernet_dstAddr
  phvwrpair.e   p.flow_lkp_metadata_lkp_dport, k.inner_ethernet_etherType, \
                    p.flow_lkp_metadata_lkp_srcMacAddr, k.inner_ethernet_srcAddr
  phvwr         p.flow_lkp_metadata_lkp_dstMacAddr, k.inner_ethernet_dstAddr

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tunneled_vm_bounce_packet:
  nop.e
  nop
