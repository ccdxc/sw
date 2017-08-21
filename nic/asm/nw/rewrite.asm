#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct rewrite_k k;
struct rewrite_d d;
struct phv_      p;

%%

nop:
  nop.e
  nop

.align
l3_rewrite:
  seq         c1, k.vlan_tag_valid, 1
  phvwr.c1    p.ethernet_etherType, k.vlan_tag_etherType
  phvwr       p.vlan_tag_valid, 0

  seq         c2, k.rewrite_metadata_mac_sa_rewrite, 1
  phvwr       p.ethernet_srcAddr, d.u.l3_rewrite_d.mac_sa
  seq         c2, k.rewrite_metadata_mac_da_rewrite, 1
  phvwr.c2    p.ethernet_dstAddr, d.u.l3_rewrite_d.mac_da

  add         r6, r0, k.ipv4_valid
  or          r6, r6, k.ipv6_valid, 1
  seq         c1, k.rewrite_metadata_ttl_dec, 1
  .brbegin
  br          r6[1:0]
  seq         c2, k.qos_metadata_dscp_en, 1
  .brcase 0
  nop.e
  nop
  .brcase 1
  phvwr.c2    p.ipv4_diffserv, k.qos_metadata_dscp
  sub.e       r7, k.ipv4_ttl, 1
  phvwr.c1    p.ipv4_ttl, r7
  .brcase 2
  phvwr.c2    p.ipv6_trafficClass, k.qos_metadata_dscp
  sub.e       r7, k.ipv6_hopLimit, 1
  phvwr.c1    p.ipv6_hopLimit, r7
  .brcase 3
  nop.e
  nop
  .brend

.align
ipv4_nat_src_rewrite:
  b           l3_rewrite
  phvwr       p.ipv4_srcAddr, k.nat_metadata_nat_ip

.align
ipv4_nat_dst_rewrite:
  b           l3_rewrite
  phvwr       p.ipv4_dstAddr, k.nat_metadata_nat_ip

.align
ipv4_nat_src_udp_rewrite:
  phvwr       p.ipv4_srcAddr, k.nat_metadata_nat_ip
  b           l3_rewrite
  phvwr       p.udp_srcPort, k.nat_metadata_nat_l4_port

.align
ipv4_nat_dst_udp_rewrite:
  phvwr       p.ipv4_dstAddr, k.nat_metadata_nat_ip
  b           l3_rewrite
  phvwr       p.udp_dstPort, k.nat_metadata_nat_l4_port

.align
ipv4_nat_src_tcp_rewrite:
  phvwr       p.ipv4_srcAddr, k.nat_metadata_nat_ip
  b           l3_rewrite
  phvwr       p.tcp_srcPort, k.nat_metadata_nat_l4_port

.align
ipv4_nat_dst_tcp_rewrite:
  phvwr       p.ipv4_dstAddr, k.nat_metadata_nat_ip
  b           l3_rewrite
  phvwr       p.tcp_dstPort, k.nat_metadata_nat_l4_port

.align
ipv4_twice_nat_rewrite:
  phvwr       p.ipv4_srcAddr, k.nat_metadata_nat_ip
  b           l3_rewrite
  phvwr       p.ipv4_dstAddr, k.nat_metadata_twice_nat_ip

.align
ipv4_twice_nat_udp_rewrite:
  phvwr       p.ipv4_srcAddr, k.nat_metadata_nat_ip
  phvwr       p.udp_srcPort, k.nat_metadata_nat_l4_port
  phvwr       p.ipv4_dstAddr, k.nat_metadata_twice_nat_ip
                                
  b           l3_rewrite
  phvwr       p.udp_dstPort, k.nat_metadata_twice_nat_l4_port

.align
ipv4_twice_nat_tcp_rewrite:
  phvwr       p.ipv4_srcAddr, k.nat_metadata_nat_ip
  phvwr       p.tcp_srcPort, k.nat_metadata_nat_l4_port
  phvwr       p.ipv4_dstAddr, k.nat_metadata_twice_nat_ip
  b           l3_rewrite
  phvwr       p.tcp_dstPort, k.nat_metadata_twice_nat_l4_port

.align
ipv6_nat_src_rewrite:
  b           l3_rewrite
  phvwr       p.ipv6_srcAddr, k.nat_metadata_nat_ip

.align
ipv6_nat_dst_rewrite:
  b           l3_rewrite
  phvwr       p.ipv6_dstAddr, k.nat_metadata_nat_ip

.align
ipv6_nat_src_udp_rewrite:
  phvwr       p.ipv6_srcAddr, k.nat_metadata_nat_ip
  b           l3_rewrite
  phvwr       p.udp_srcPort, k.nat_metadata_nat_l4_port

.align
ipv6_nat_dst_udp_rewrite:
  phvwr       p.ipv6_dstAddr, k.nat_metadata_nat_ip
  b           l3_rewrite
  phvwr       p.udp_dstPort, k.nat_metadata_nat_l4_port

.align
ipv6_nat_src_tcp_rewrite:
  phvwr       p.ipv6_srcAddr, k.nat_metadata_nat_ip
  b           l3_rewrite
  phvwr       p.tcp_srcPort, k.nat_metadata_nat_l4_port

.align
ipv6_nat_dst_tcp_rewrite:
  phvwr       p.ipv6_dstAddr, k.nat_metadata_nat_ip
  b           l3_rewrite
  phvwr       p.tcp_dstPort, k.nat_metadata_nat_l4_port

.align
ipv6_twice_nat_rewrite:
  phvwr       p.ipv6_srcAddr, k.nat_metadata_nat_ip
  b           l3_rewrite
  phvwr       p.ipv4_dstAddr, k.nat_metadata_twice_nat_ip

.align
ipv6_twice_nat_udp_rewrite:
  phvwr       p.ipv6_srcAddr, k.nat_metadata_nat_ip
  phvwr       p.udp_srcPort, k.nat_metadata_nat_l4_port
  phvwr       p.ipv4_dstAddr, k.nat_metadata_twice_nat_ip
  b           l3_rewrite
  phvwr       p.udp_dstPort, k.nat_metadata_twice_nat_l4_port

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
ipv6_twice_nat_tcp_rewrite:
  phvwr       p.ipv6_srcAddr, k.nat_metadata_nat_ip
  phvwr       p.tcp_srcPort, k.nat_metadata_nat_l4_port
  phvwr       p.ipv4_dstAddr, k.nat_metadata_twice_nat_ip
  b           l3_rewrite
  phvwr       p.tcp_dstPort, k.nat_metadata_twice_nat_l4_port
