#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct tunnel_rewrite_k k;
struct tunnel_rewrite_d d;
struct phv_             p;

%%

nop:
  nop.e
  nop

.align
encap_vxlan:
  phvwr       p.inner_ethernet_dstAddr[47:32], k.ethernet_dstAddr_sbit0_ebit15
  phvwr       p.{inner_ethernet_dstAddr[31:0]...inner_ethernet_etherType}, k.{ethernet_dstAddr_sbit16_ebit47...ethernet_etherType}
  phvwr       p.ethernet_dstAddr, d.u.encap_vxlan_d.mac_da
  phvwr       p.ethernet_srcAddr, d.u.encap_vxlan_d.mac_sa

  phvwr       p.udp_srcPort, k.rewrite_metadata_entropy_hash
  phvwr       p.udp_dstPort, UDP_PORT_VXLAN
  phvwr       p.udp_checksum, 0
  add         r7, k.l3_metadata_payload_length, 30
  phvwr       p.udp_len, r7

  phvwri      p.{vxlan_flags,vxlan_reserved}, 0x80000000
  phvwr       p.vxlan_vni, k.rewrite_metadata_tunnel_vnid
  phvwr       p.vxlan_reserved2, 0

  phvwr       p.inner_ethernet_valid, 1
  phvwr       p.udp_valid, 1
  phvwr       p.vxlan_valid, 1

  seq         c1, d.u.encap_vxlan_d.ip_type, 0
  cmov        r6, c1, ETHERTYPE_IPV4, ETHERTYPE_IPV6
  seq         c2, d.u.encap_vxlan_d.vlan_valid, 1
  bal.c2      r1, f_encap_vlan
  phvwr.!c2   p.ethernet_etherType, r6

  bcf         [c1],  f_insert_ipv4_header
  add         r6, r0, 0x4011
	b.!c1       f_insert_ipv6_header
  add         r6, r0, 0x1140

.align
encap_vxlan_gpe:
  phvwr       p.inner_ethernet_dstAddr[47:32], k.ethernet_dstAddr_sbit0_ebit15
  phvwr       p.{inner_ethernet_dstAddr[31:0]...inner_ethernet_etherType}, k.{ethernet_dstAddr_sbit16_ebit47...ethernet_etherType}
  phvwr       p.ethernet_dstAddr, d.u.encap_vxlan_d.mac_da
  phvwr       p.ethernet_srcAddr, d.u.encap_vxlan_d.mac_sa

  phvwr       p.udp_srcPort, k.rewrite_metadata_entropy_hash
  phvwr       p.udp_dstPort, UDP_PORT_VXLAN_GPE
  phvwr       p.udp_checksum, 0
  add         r7, k.l3_metadata_payload_length, 30
  phvwr       p.udp_len, r7

  phvwri      p.{vxlan_gpe_flags,vxlan_gpe_reserved}, 0x90000000
  phvwr       p.vxlan_gpe_vni, k.rewrite_metadata_tunnel_vnid
  phvwr       p.vxlan_gpe_reserved2, 0

  phvwr       p.inner_ethernet_valid, 1
  phvwr       p.udp_valid, 1
  phvwr       p.vxlan_gpe_valid, 1

  seq         c1, d.u.encap_vxlan_d.ip_type, 0
  cmov        r6, c1, ETHERTYPE_IPV4, ETHERTYPE_IPV6
  seq         c2, d.u.encap_vxlan_d.vlan_valid, 1
  bal.c2      r1, f_encap_vlan
  phvwr.!c2   p.ethernet_etherType, r6

  bcf         [c1],  f_insert_ipv4_header
  add         r6, r0, 0x4011
  b.!c1       f_insert_ipv6_header
  add         r6, r0, 0x1140

.align
encap_genv:
  phvwr       p.inner_ethernet_dstAddr[47:32], k.ethernet_dstAddr_sbit0_ebit15
  phvwr       p.{inner_ethernet_dstAddr[31:0]...inner_ethernet_etherType}, k.{ethernet_dstAddr_sbit16_ebit47...ethernet_etherType}
  phvwr       p.ethernet_dstAddr, d.u.encap_vxlan_d.mac_da
  phvwr       p.ethernet_srcAddr, d.u.encap_vxlan_d.mac_sa

  phvwr       p.udp_srcPort, k.rewrite_metadata_entropy_hash
  phvwr       p.udp_dstPort, UDP_PORT_GENV
  phvwr       p.udp_checksum, 0
  add         r7, k.l3_metadata_payload_length, 30
  phvwr       p.udp_len, r7

  phvwri      p.{genv_ver...genv_protoType}, ETHERTYPE_ETHERNET
  phvwr       p.genv_vni, k.rewrite_metadata_tunnel_vnid
  phvwr       p.genv_reserved2, 0

  phvwr       p.inner_ethernet_valid, 1
  phvwr       p.udp_valid, 1
  phvwr       p.genv_valid, 1

  seq         c1, d.u.encap_vxlan_d.ip_type, 0
  cmov        r6, c1, ETHERTYPE_IPV4, ETHERTYPE_IPV6
  seq         c2, d.u.encap_vxlan_d.vlan_valid, 1
  bal.c2      r1, f_encap_vlan
  phvwr.!c2   p.ethernet_etherType, r6

  bcf         [c1],  f_insert_ipv4_header
  add         r6, r0, 0x4011
	b.!c1       f_insert_ipv6_header
  add         r6, r0, 0x1140

.align
encap_nvgre:
  phvwr       p.inner_ethernet_dstAddr[47:32], k.ethernet_dstAddr_sbit0_ebit15
  phvwr       p.{inner_ethernet_dstAddr[31:0]...inner_ethernet_etherType}, k.{ethernet_dstAddr_sbit16_ebit47...ethernet_etherType}
  phvwr       p.ethernet_dstAddr, d.u.encap_vxlan_d.mac_da
  phvwr       p.ethernet_srcAddr, d.u.encap_vxlan_d.mac_sa

  phvwri      p.{gre_C...gre_proto}, GRE_PROTO_NVGRE
  phvwr       p.nvgre_tni, k.rewrite_metadata_tunnel_vnid
  phvwr       p.nvgre_flow_id, k.rewrite_metadata_entropy_hash

  phvwr       p.inner_ethernet_valid, 1
  phvwr       p.gre_valid, 1
  phvwr       p.nvgre_valid, 1

  seq         c1, d.u.encap_vxlan_d.ip_type, 0
  cmov        r6, c1, ETHERTYPE_IPV4, ETHERTYPE_IPV6
  seq         c2, d.u.encap_vxlan_d.vlan_valid, 1
  bal.c2      r1, f_encap_vlan
  phvwr.!c2   p.ethernet_etherType, r6

  add         r7, k.l3_metadata_payload_length, 22
  bcf         [c1],  f_insert_ipv4_header
  add         r6, r0, 0x402f
	b.!c1       f_insert_ipv6_header
  add         r6, r0, 0x2f40

.align
encap_gre:
  phvwr       p.ethernet_dstAddr, d.u.encap_vxlan_d.mac_da
  phvwr       p.ethernet_srcAddr, d.u.encap_vxlan_d.mac_sa

  phvwr       p.{gre_C...gre_ver}, 0x0000
  phvwr       p.gre_proto, k.ethernet_etherType
  phvwr       p.gre_valid, 1

  seq         c1, d.u.encap_vxlan_d.ip_type, 0
  cmov        r6, c1, ETHERTYPE_IPV4, ETHERTYPE_IPV6
  seq         c2, d.u.encap_vxlan_d.vlan_valid, 1
  bal.c2      r1, f_encap_vlan
  phvwr.!c2   p.ethernet_etherType, r6

  add         r7, k.l3_metadata_payload_length, 4
  bcf         [c1],  f_insert_ipv4_header
  add         r6, r0, 0x402f
	b.!c1       f_insert_ipv6_header
  add         r6, r0, 0x2f40

.align
encap_erspan:
  phvwr       p.inner_ethernet_dstAddr[47:32], k.ethernet_dstAddr_sbit0_ebit15
  phvwr       p.{inner_ethernet_dstAddr[31:0]...inner_ethernet_etherType}, k.{ethernet_dstAddr_sbit16_ebit47...ethernet_etherType}
  phvwr       p.ethernet_dstAddr, d.u.encap_vxlan_d.mac_da
  phvwr       p.ethernet_srcAddr, d.u.encap_vxlan_d.mac_sa

  phvwri      p.{gre_C...gre_proto}, GRE_PROTO_ERSPAN_T3
  phvwr       p.{erspan_t3_header_version,erspan_t3_header_vlan}, 0x2000
  phvwr       p.erspan_t3_header_priority, 0
  phvwr       p.erspan_t3_header_span_id, k.capri_intrinsic_tm_span_session
  phvwr       p.erspan_t3_header_timestamp, r6
  phvwr       p.{erspan_t3_header_sgt,erspan_t3_header_ft_d_other}, 0

  phvwr       p.inner_ethernet_valid, 1
  phvwr       p.gre_valid, 1
  phvwr       p.erspan_t3_header_valid, 1

  seq         c1, d.u.encap_vxlan_d.ip_type, 0
  cmov        r6, c1, ETHERTYPE_IPV4, ETHERTYPE_IPV6
  seq         c2, d.u.encap_vxlan_d.vlan_valid, 1
  bal.c2      r1, f_encap_vlan
  phvwr.!c2   p.ethernet_etherType, r6

  add         r7, k.l3_metadata_payload_length, 30
  bcf         [c1],  f_insert_ipv4_header
  add         r6, r0, 0x402f
	b.!c1       f_insert_ipv6_header
  add         r6, r0, 0x2f40

.align
encap_ip:
  seq         c1, d.u.encap_vxlan_d.ip_type, 0
  cmov        r6, c1, ETHERTYPE_IPV4, ETHERTYPE_IPV6
  seq         c2, d.u.encap_vxlan_d.vlan_valid, 1
  bal.c2      r1, f_encap_vlan
  phvwr.!c2   p.ethernet_etherType, r6

  add         r7, r0, k.l3_metadata_payload_length
  bcf         [c1],  f_insert_ipv4_header
  add         r6, 0x4000, k.tunnel_metadata_inner_ip_proto, 8
	b.!c1       f_insert_ipv6_header
  add         r6, 0x0040, k.tunnel_metadata_inner_ip_proto, 8

.align
encap_mpls:
  seq         c1, d.u.encap_mpls_d.eompls, TRUE
  phvwr.c1    p.inner_ethernet_dstAddr[47:32], k.ethernet_dstAddr_sbit0_ebit15
  phvwr.c1    p.{inner_ethernet_dstAddr[31:0]...inner_ethernet_etherType}, k.{ethernet_dstAddr_sbit16_ebit47...ethernet_etherType}
  phvwr       p.ethernet_dstAddr, d.u.encap_vxlan_d.mac_da
  phvwr       p.ethernet_srcAddr, d.u.encap_vxlan_d.mac_sa
  add         r1, r0, d.u.encap_mpls_d.num_labels
  .brbegin
  br          r1[1:0]
  phvwr       p.ethernet_etherType, ETHERTYPE_MPLS
  .brcase 0
  nop.e
  nop
  .brcase 1
  phvwr.e     p.{mpls_0_label...mpls_0_ttl}, d.{u.encap_mpls_d.label0...u.encap_mpls_d.ttl0}
  phvwr       p.mpls_0_valid, 0x1
  .brcase 2
  phvwr.e     p.{mpls_0_label...mpls_1_ttl}, d.{u.encap_mpls_d.label0...u.encap_mpls_d.ttl1}
  phvwr       p.{mpls_1_valid,mpls_0_valid}, 0x3
  .brcase 3
  phvwr.e     p.{mpls_0_label...mpls_2_ttl}, d.{u.encap_mpls_d.label0...u.encap_mpls_d.ttl2}
  phvwr       p.{mpls_2_valid,mpls_1_valid,mpls_0_valid}, 0x7
  .brend

.align
encap_vlan:
  phvwr       p.vlan_tag_etherType, k.ethernet_etherType
  phvwr       p.vlan_tag_vid, k.rewrite_metadata_tunnel_vnid
  phvwr       p.vlan_tag_valid, 1
  seq         c7, k.qos_metadata_cos_en, 1
  phvwr.c7    p.vlan_tag_pcp, k.qos_metadata_cos
  phvwr.e     p.vlan_tag_dei, 0
  phvwr       p.ethernet_etherType, ETHERTYPE_VLAN

.align
encap_ipv4_ipsec_tunnel_esp:
  nop.e
  nop

.align
encap_vlan_ipv4_ipsec_tunnel_esp:
  nop.e
  nop

.align
encap_ipv6_ipsec_tunnel_esp:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
encap_vlan_ipv6_ipsec_tunnel_esp:
  nop.e
  nop

f_encap_vlan:
  phvwr       p.vlan_tag_etherType, r6
  phvwr       p.vlan_tag_vid, d.u.encap_vxlan_d.vlan_id
  phvwr       p.vlan_tag_valid, 1
  seq         c7, k.qos_metadata_cos_en, 1
  phvwr.c7    p.vlan_tag_pcp, k.qos_metadata_cos
  phvwr       p.vlan_tag_dei, 0
  jr          r1
  phvwr       p.ethernet_etherType, ETHERTYPE_VLAN

f_insert_ipv4_header:
  phvwr       p.{ipv4_version,ipv4_ihl,ipv4_diffserv}, 0x4500
  phvwr       p.{ipv4_identification...ipv4_fragOffset}, 0
  phvwr       p.{ipv4_ttl...ipv4_protocol}, r6
  phvwr       p.{ipv4_srcAddr,ipv4_dstAddr}, d.{u.encap_vxlan_d.ip_sa,u.encap_vxlan_d.ip_da}
	add         r7, r7, 20
  phvwr.e     p.ipv4_totalLen, r7
  phvwr       p.ipv4_valid, 1

f_insert_ipv6_header:
  phvwri      p.{ipv6_version...ipv6_trafficClass}, 0x60000000
  phvwr       p.{ipv6_nextHdr,ipv6_hopLimit}, r6
  phvwr       p.{ipv6_srcAddr,ipv6_dstAddr}, d.{u.encap_vxlan_d.ip_sa,u.encap_vxlan_d.ip_da}
  phvwr.e     p.ipv6_payloadLen, r7
  phvwr       p.ipv6_valid, 1
