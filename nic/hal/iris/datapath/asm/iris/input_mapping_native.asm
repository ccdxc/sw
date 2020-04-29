#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_input_mapping_native_k.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nw.h"

struct input_mapping_native_k_ k;
struct phv_ p;

%%

input_mapping_miss:
  phvwr.e     p.control_metadata_drop_reason[DROP_INPUT_MAPPING], 1
  phvwr.f     p.capri_intrinsic_drop, 1

.align
nop:
  nop.e
  nop

.align
native_ipv4_packet:
  bbeq          k.ethernet_dstAddr[40], 0, native_ipv4_packet_common
  phvwr         p.flow_lkp_metadata_pkt_type, PACKET_TYPE_UNICAST
  xor           r6, -1, r0
  seq           c2, k.ethernet_dstAddr, r6[47:0]
  cmov          r7, c2, PACKET_TYPE_BROADCAST, PACKET_TYPE_MULTICAST
  phvwr         p.flow_lkp_metadata_pkt_type, r7

native_ipv4_packet_common:
  phvwrpair     p.flow_lkp_metadata_lkp_dst[31:0], k.ipv4_dstAddr, \
                    p.flow_lkp_metadata_lkp_src[31:0], k.ipv4_srcAddr

  or            r1, k.ipv4_ttl, k.ethernet_dstAddr, 8
  phvwrpair     p.flow_lkp_metadata_lkp_srcMacAddr, k.ethernet_srcAddr, \
                p.{flow_lkp_metadata_lkp_dstMacAddr,flow_lkp_metadata_ip_ttl}, \
                    r1

  phvwrpair     p.flow_lkp_metadata_ipv4_flags, k.ipv4_flags, \
                    p.flow_lkp_metadata_ipv4_hlen, k.ipv4_ihl

  bbeq          k.esp_valid, TRUE, native_ipv4_esp_packet
  phvwr         p.{tunnel_metadata_tunnel_type,tunnel_metadata_tunnel_vni}, r0

  seq           c1, k.roce_bth_valid, TRUE
  cmov          r1, c1, r0, k.udp_srcPort
  or            r1, r1, k.udp_dstPort, 16
  seq           c1, k.ipv4_protocol, IP_PROTO_UDP
  phvwr.c1      p.{flow_lkp_metadata_lkp_dport,flow_lkp_metadata_lkp_sport}, r1
  seq.!c1       c1, k.ipv4_protocol, IP_PROTO_TCP
  seq.!c1       c1, k.ipv4_protocol, IP_PROTO_ICMP
  phvwr.!c1     p.{flow_lkp_metadata_lkp_dport,flow_lkp_metadata_lkp_sport}, r0
  phvwr.e       p.flow_lkp_metadata_lkp_proto, k.ipv4_protocol
  phvwr.f       p.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV4

native_ipv4_esp_packet:
  phvwr.e       p.flow_lkp_metadata_lkp_proto, IP_PROTO_IPSEC_ESP
  nop

.align
native_ipv6_packet:
  bbeq          k.ethernet_dstAddr[40], 0, native_ipv6_packet_common
  phvwr         p.flow_lkp_metadata_pkt_type, PACKET_TYPE_UNICAST
  xor           r6, -1, r0
  seq           c2, k.ethernet_dstAddr, r6[47:0]
  cmov          r7, c2, PACKET_TYPE_BROADCAST, PACKET_TYPE_MULTICAST
  phvwr         p.flow_lkp_metadata_pkt_type, r7

native_ipv6_packet_common:
  phvwr         p.{tunnel_metadata_tunnel_type,tunnel_metadata_tunnel_vni}, r0
  seq           c1, k.roce_bth_valid, TRUE
  cmov          r1, c1, r0, k.udp_srcPort
  or            r1, r1, k.udp_dstPort, 16
  seq           c1, k.l3_metadata_ipv6_ulp, IP_PROTO_UDP
  phvwr.c1      p.{flow_lkp_metadata_lkp_dport,flow_lkp_metadata_lkp_sport}, r1
  seq.!c1       c1, k.l3_metadata_ipv6_ulp, IP_PROTO_TCP
  seq.!c1       c1, k.l3_metadata_ipv6_ulp, IP_PROTO_ICMPV6
  phvwr.!c1     p.{flow_lkp_metadata_lkp_dport,flow_lkp_metadata_lkp_sport}, r0
  phvwr         p.flow_lkp_metadata_lkp_proto, k.l3_metadata_ipv6_ulp
  phvwr         p.flow_lkp_metadata_lkp_srcMacAddr, k.ethernet_srcAddr
  phvwr         p.flow_lkp_metadata_lkp_dstMacAddr, k.ethernet_dstAddr
  phvwr.e       p.flow_lkp_metadata_ip_ttl, k.ipv6_hopLimit
  phvwr.f       p.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV6

.align
native_non_ip_packet:
  bbeq          k.ethernet_dstAddr[40], 0, native_non_ip_packet_common
  phvwr         p.flow_lkp_metadata_pkt_type, PACKET_TYPE_UNICAST
  xor           r6, -1, r0
  seq           c2, k.ethernet_dstAddr, r6[47:0]
  cmov          r7, c2, PACKET_TYPE_BROADCAST, PACKET_TYPE_MULTICAST
  phvwr         p.flow_lkp_metadata_pkt_type, r7

native_non_ip_packet_common:
  phvwr         p.{tunnel_metadata_tunnel_type,tunnel_metadata_tunnel_vni}, r0
  phvwr         p.flow_lkp_metadata_lkp_type, FLOW_KEY_LOOKUP_TYPE_MAC
  seq           c1, k.vlan_tag_valid, 1
  phvwr.c1      p.flow_lkp_metadata_lkp_dport, k.vlan_tag_etherType
  phvwr.!c1     p.flow_lkp_metadata_lkp_dport, k.ethernet_etherType
  phvwrpair     p.flow_lkp_metadata_lkp_dst[47:0], k.ethernet_dstAddr, \
                    p.flow_lkp_metadata_lkp_src[47:0], k.ethernet_srcAddr
  phvwr.e       p.flow_lkp_metadata_lkp_srcMacAddr, k.ethernet_srcAddr
  phvwr.f       p.flow_lkp_metadata_lkp_dstMacAddr, k.ethernet_dstAddr

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
input_mapping_native_error:
  nop.e
  nop
