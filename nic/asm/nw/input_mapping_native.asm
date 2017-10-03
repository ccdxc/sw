#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct input_mapping_native_k k;
struct phv_                   p;

%%

input_mapping_miss:
  phvwr.e     p.control_metadata_drop_reason[DROP_INPUT_MAPPING], 1
  phvwr       p.capri_intrinsic_drop, 1

.align
nop:
  nop.e
  nop

.align
native_ipv4_packet:
  or          r5, k.ipv4_ihl_sbit1_ebit3, k.ipv4_ihl_sbit0_ebit0, 3
  or          r1, k.ethernet_srcAddr_sbit40_ebit47, \
                  k.ethernet_srcAddr_sbit0_ebit39, 8
  seq         c1, r1, r0
  seq         c2, k.ethernet_dstAddr, r0
  seq         c3, r1[40], 1
  bcf         [c1|c2|c3], malformed_native_packet

  sne         c1, k.ipv4_version, 4
  seq         c2, k.ipv4_ttl, 0
  bcf         [c1|c2], malformed_native_packet

  seq         c1, k.ethernet_dstAddr[40], 0
  add.c1      r7, r0, PACKET_TYPE_UNICAST
  xor         r6, -1, r0
  seq         c2, k.ethernet_dstAddr, r6[47:0]
  cmov.!c1    r7, c2, PACKET_TYPE_BROADCAST, PACKET_TYPE_MULTICAST
  phvwr       p.flow_lkp_metadata_pkt_type, r7

  add         r6, r5, k.tcp_dataOffset
  sub         r7, k.ipv4_totalLen, r6, 2
  phvwr       p.l4_metadata_tcp_data_len, r7

  seq         c1, k.ipv4_protocol, IP_PROTO_UDP
  phvwr.c1    p.flow_lkp_metadata_lkp_sport, k.udp_srcPort
  phvwr.c1    p.flow_lkp_metadata_lkp_dport, k.udp_dstPort

  phvwr       p.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV4
  phvwr       p.flow_lkp_metadata_lkp_src, k.ipv4_srcAddr
  phvwr       p.flow_lkp_metadata_lkp_dst, k.ipv4_dstAddr
  phvwr       p.flow_lkp_metadata_lkp_proto, k.ipv4_protocol
  phvwr       p.flow_lkp_metadata_ipv4_flags, k.ipv4_flags
  phvwr       p.flow_lkp_metadata_ipv4_frag_offset, \
                  k.{ipv4_fragOffset_sbit0_ebit4,ipv4_fragOffset_sbit5_ebit12}
  phvwr       p.flow_lkp_metadata_ipv4_hlen, r5
  phvwr       p.flow_lkp_metadata_ip_ttl, k.ipv4_ttl

  phvwr.e     p.flow_lkp_metadata_lkp_srcMacAddr, r1
  phvwr       p.flow_lkp_metadata_lkp_dstMacAddr, k.ethernet_dstAddr

.align
native_ipv6_packet:
  or          r1, k.ethernet_srcAddr_sbit40_ebit47, \
                  k.ethernet_srcAddr_sbit0_ebit39, 8
  seq         c1, r1, r0
  seq         c2, k.ethernet_dstAddr, r0
  seq         c3, r1[40], 1
  bcf         [c1|c2|c3], malformed_native_packet

  sne         c1, k.ipv6_version, 6
  seq         c2, k.ipv6_hopLimit, 0
  bcf         [c1|c2], malformed_native_packet

  seq         c1, k.ethernet_dstAddr[40], 0
  add.c1      r7, r0, PACKET_TYPE_UNICAST
  xor         r6, -1, r0
  seq         c2, k.ethernet_dstAddr, r6[47:0]
  cmov.!c1    r7, c2, PACKET_TYPE_BROADCAST, PACKET_TYPE_MULTICAST
  phvwr       p.flow_lkp_metadata_pkt_type, r7

  sub         r7, k.ipv6_payloadLen, k.tcp_dataOffset, 2
  phvwr       p.l4_metadata_tcp_data_len, r7

  seq         c1, k.ipv6_nextHdr, IP_PROTO_UDP
  phvwr.c1    p.flow_lkp_metadata_lkp_sport, k.udp_srcPort
  phvwr.c1    p.flow_lkp_metadata_lkp_dport, k.udp_dstPort

  phvwr       p.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV6
  phvwr       p.flow_lkp_metadata_lkp_src, k.{ipv6_srcAddr_sbit0_ebit31, \
                                              ipv6_srcAddr_sbit32_ebit127}
  phvwr       p.flow_lkp_metadata_lkp_dst, k.{ipv6_dstAddr_sbit0_ebit31, \
                                              ipv6_dstAddr_sbit32_ebit127}
  phvwr       p.flow_lkp_metadata_lkp_proto, k.ipv6_nextHdr
  phvwr       p.flow_lkp_metadata_ip_ttl, k.ipv6_hopLimit

  phvwr.e     p.flow_lkp_metadata_lkp_srcMacAddr, r1
  phvwr       p.flow_lkp_metadata_lkp_dstMacAddr, k.ethernet_dstAddr

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
native_non_ip_packet:
  or          r1, k.ethernet_srcAddr_sbit40_ebit47, \
                  k.ethernet_srcAddr_sbit0_ebit39, 8
  seq         c1, r1, r0
  seq         c2, k.ethernet_dstAddr, r0
  seq         c3, r1[40], 1
  bcf         [c1|c2|c3], malformed_native_packet

  seq         c1, k.ethernet_dstAddr[40], 0
  add.c1      r7, r0, PACKET_TYPE_UNICAST
  xor         r6, -1, r0
  seq         c2, k.ethernet_dstAddr, r6[47:0]
  cmov.!c1    r7, c2, PACKET_TYPE_BROADCAST, PACKET_TYPE_MULTICAST
  phvwr       p.flow_lkp_metadata_pkt_type, r7

  phvwr       p.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_MAC
  seq         c1, k.vlan_tag_valid, 1
  phvwr.c1    p.flow_lkp_metadata_lkp_sport, k.vlan_tag_etherType
  phvwr.!c1   p.flow_lkp_metadata_lkp_sport, k.ethernet_etherType
  phvwr.e     p.flow_lkp_metadata_lkp_dst, k.ethernet_dstAddr
  phvwr       p.flow_lkp_metadata_lkp_src, r1

malformed_native_packet:
  phvwr.e     p.control_metadata_drop_reason[DROP_MALFORMED_PKT], 1
  phvwr       p.capri_intrinsic_drop, 1
