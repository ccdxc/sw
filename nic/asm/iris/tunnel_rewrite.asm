#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/iris/include/defines.h"

struct tunnel_rewrite_k k;
struct tunnel_rewrite_d d;
struct phv_             p;

%%

nop:
  nop.e
  nop

.align
encap_vxlan:
  // r5 : k.capri_p4_intrinsic_packet_len
  or          r5, k.capri_p4_intrinsic_packet_len_sbit6_ebit13, \
                  k.capri_p4_intrinsic_packet_len_sbit0_ebit5, 8
  phvwr       p.{inner_ethernet_dstAddr...inner_ethernet_etherType}, \
                  k.{ethernet_dstAddr...ethernet_etherType}
  phvwrpair   p.ethernet_dstAddr, d.u.encap_vxlan_d.mac_da, \
                p.ethernet_srcAddr, d.u.encap_vxlan_d.mac_sa

  add         r7, r5, 16

  // vxlan header (flags, vni)
  add         r1, r0, 0x08, 56
  or          r1, r1, k.rewrite_metadata_tunnel_vnid, 8

  // udp header (srcPort, dstPort, len)
  add         r2, r0, k.rewrite_metadata_entropy_hash, 48
  or          r2, r2, UDP_PORT_VXLAN, 32
  or          r2, r2, r7, 16
  phvwr       p.{vxlan_flags...vxlan_reserved2}, r1
  phvwr       p.{udp_srcPort...udp_checksum}, r2

  // set inner_ethernet_valid, vxlan_valid and udp_valid
  .assert(offsetof(p, inner_ethernet_valid) - offsetof(p, vxlan_valid) == 4)
  .assert(offsetof(p, vxlan_valid) - offsetof(p, udp_valid) == 3)
  phvwrmi     p.{inner_ethernet_valid...udp_valid}, 0xFF, 0x89

  seq         c1, d.u.encap_vxlan_d.ip_type, IP_HEADER_TYPE_IPV4
  cmov        r6, c1, ETHERTYPE_IPV4, ETHERTYPE_IPV6
  seq         c2, d.u.encap_vxlan_d.vlan_valid, 1
  bal.c2      r1, f_encap_vlan
  phvwr.!c2   p.ethernet_etherType, r6

  // update capri_p4_intrinsic_packet_len
  cmov        r1, c1, 50, 70
  add.c2      r1, r1, 4
  add         r1, r1, r5
  phvwr       p.capri_p4_intrinsic_packet_len, r1

  bcf         [c1],  f_insert_ipv4_header
  add         r6, r0, 0x4011
#ifdef PHASE2
  b.!c1       f_insert_ipv6_header
  add         r6, r0, 0x1140
#else /* PHASE2 */
  nop.e
  nop
#endif /* PHASE2 */

.align
encap_vlan:
  or          r5, k.capri_p4_intrinsic_packet_len_sbit6_ebit13, \
                  k.capri_p4_intrinsic_packet_len_sbit0_ebit5, 8
  add         r1, r5, 4
  phvwr       p.capri_p4_intrinsic_packet_len, r1
  phvwr       p.vlan_tag_etherType, k.ethernet_etherType
  phvwr       p.{vlan_tag_dei, vlan_tag_vid}, k.rewrite_metadata_tunnel_vnid[11:0]
  seq         c7, k.qos_metadata_cos_en, 1
  phvwr.c7    p.vlan_tag_pcp, k.qos_metadata_cos
  phvwr.e     p.vlan_tag_valid, 1
  phvwr       p.ethernet_etherType, ETHERTYPE_VLAN

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
encap_erspan:
  phvwr       p.{inner_ethernet_dstAddr...inner_ethernet_etherType}, \
                  k.{ethernet_dstAddr...ethernet_etherType}
  phvwr       p.ethernet_dstAddr, d.u.encap_vxlan_d.mac_da
  phvwr       p.ethernet_srcAddr, d.u.encap_vxlan_d.mac_sa

  phvwri      p.{gre_C...gre_proto}, GRE_PROTO_ERSPAN_T3
  phvwr       p.{erspan_t3_version,erspan_t3_vlan}, 0x2000
  phvwr       p.erspan_t3_priority, 0
  phvwr       p.erspan_t3_span_id, k.capri_intrinsic_tm_span_session
#ifndef CAPRI_IGNORE_TIMESTAMP
  phvwr       p.erspan_t3_timestamp, r6
#endif
  phvwr       p.{erspan_t3_sgt,erspan_t3_ft_d_other}, 0

  phvwr       p.inner_ethernet_valid, 1
  phvwr       p.gre_valid, 1
  phvwr       p.erspan_t3_valid, 1

  seq         c1, d.u.encap_vxlan_d.ip_type, IP_HEADER_TYPE_IPV4
  cmov        r6, c1, ETHERTYPE_IPV4, ETHERTYPE_IPV6
  seq         c2, d.u.encap_vxlan_d.vlan_valid, 1
  bal.c2      r1, f_encap_vlan
  phvwr.!c2   p.ethernet_etherType, r6

  or          r5, k.capri_p4_intrinsic_packet_len_sbit6_ebit13, \
                  k.capri_p4_intrinsic_packet_len_sbit0_ebit5, 8
  add         r7, r5, 16

  // update capri_p4_intrinsic_packet_len
  cmov        r1, c1, 50, 70
  add.c2      r1, r1, 4
  add         r1, r1, r5
  phvwr       p.capri_p4_intrinsic_packet_len, r1

  bcf         [c1],  f_insert_ipv4_header
  add         r6, r0, 0x402f
#ifdef PHASE2
  b.!c1       f_insert_ipv6_header
  add         r6, r0, 0x2f40
#else /* PHASE2 */
  nop.e
  nop
#endif /* PHASE2 */

// r6 :  etherType (input)
f_encap_vlan:
  .assert(offsetof(p, ethernet_etherType) - offsetof(p, vlan_tag_etherType) == 32)
  phvwr       p.vlan_tag_valid, 1
  add         r3, r6, d.u.encap_vxlan_d.vlan_id, 16
  add         r4, r0, ETHERTYPE_VLAN
  phvwrpair   p.ethernet_etherType, r4, p.{vlan_tag_vid,vlan_tag_etherType}, r3
  seq         c7, k.qos_metadata_cos_en, 1
  jr          r1
  phvwr.c7    p.vlan_tag_pcp, k.qos_metadata_cos

// r6 : ttl, protocol (input)
// r7 : payload length (input)
f_insert_ipv4_header:
  add         r3, r7, 20
  or          r3, r3, 0x4500, 16
  phvwr       p.{ipv4_version...ipv4_totalLen}, r3
  phvwrpair   p.{ipv4_identification...ipv4_fragOffset}, 0, \
                p.{ipv4_ttl...ipv4_protocol}, r6
  phvwrpair   p.ipv4_srcAddr, d.u.encap_vxlan_d.ip_sa, \
                p.ipv4_dstAddr, d.u.encap_vxlan_d.ip_da
  phvwr.e     p.ipv4_valid, 1
  phvwr.f     p.control_metadata_checksum_ctl[CHECKSUM_CTL_IP_CHECKSUM], TRUE

#ifdef PHASE2
// r6 : protocol, ttl (input)
f_insert_ipv6_header:
  phvwri      p.{ipv6_version...ipv6_trafficClass}, 0x60000000
  phvwr       p.{ipv6_nextHdr,ipv6_hopLimit}, r6
  phvwr       p.{ipv6_srcAddr,ipv6_dstAddr}, d.{u.encap_vxlan_d.ip_sa,u.encap_vxlan_d.ip_da}
  phvwr.e     p.ipv6_payloadLen, r7
  phvwr.f     p.ipv6_valid, 1

.align
encap_vxlan_gpe:
  phvwr       p.{inner_ethernet_dstAddr...inner_ethernet_etherType}, \
                  k.{ethernet_dstAddr...ethernet_etherType}
  phvwr       p.ethernet_dstAddr, d.u.encap_vxlan_d.mac_da
  phvwr       p.ethernet_srcAddr, d.u.encap_vxlan_d.mac_sa

  phvwr       p.udp_srcPort, k.rewrite_metadata_entropy_hash
  phvwr       p.udp_dstPort, UDP_PORT_VXLAN_GPE
  phvwr       p.udp_checksum, 0
  or          r5, k.capri_p4_intrinsic_packet_len_sbit6_ebit13, \
                  k.capri_p4_intrinsic_packet_len_sbit0_ebit5, 8
  add         r7, r5, 16
  phvwr       p.udp_len, r7

  phvwri      p.{vxlan_gpe_flags,vxlan_gpe_reserved}, 0x90000000
  phvwr       p.vxlan_gpe_vni, k.rewrite_metadata_tunnel_vnid
  phvwr       p.vxlan_gpe_reserved2, 0

  phvwr       p.inner_ethernet_valid, 1
  phvwr       p.udp_valid, 1
  phvwr       p.vxlan_gpe_valid, 1

  seq         c1, d.u.encap_vxlan_d.ip_type, IP_HEADER_TYPE_IPV4
  cmov        r6, c1, ETHERTYPE_IPV4, ETHERTYPE_IPV6
  seq         c2, d.u.encap_vxlan_d.vlan_valid, 1
  bal.c2      r1, f_encap_vlan
  phvwr.!c2   p.ethernet_etherType, r6

  // update capri_p4_intrinsic_packet_len
  cmov        r1, c1, 50, 70
  add.c2      r1, r1, 4
  add         r1, r1, r5
  phvwr       p.capri_p4_intrinsic_packet_len, r1

  bcf         [c1],  f_insert_ipv4_header
  add         r6, r0, 0x4011
  b.!c1       f_insert_ipv6_header
  add         r6, r0, 0x1140

.align
encap_genv:
  phvwr       p.{inner_ethernet_dstAddr...inner_ethernet_etherType}, \
                  k.{ethernet_dstAddr...ethernet_etherType}
  phvwr       p.ethernet_dstAddr, d.u.encap_vxlan_d.mac_da
  phvwr       p.ethernet_srcAddr, d.u.encap_vxlan_d.mac_sa

  phvwr       p.udp_srcPort, k.rewrite_metadata_entropy_hash
  phvwr       p.udp_dstPort, UDP_PORT_GENV
  phvwr       p.udp_checksum, 0
  or          r5, k.capri_p4_intrinsic_packet_len_sbit6_ebit13, \
                  k.capri_p4_intrinsic_packet_len_sbit0_ebit5, 8
  add         r7, r5, 16
  phvwr       p.udp_len, r7

  phvwri      p.{genv_ver...genv_protoType}, ETHERTYPE_ETHERNET
  phvwr       p.genv_vni, k.rewrite_metadata_tunnel_vnid
  phvwr       p.genv_reserved2, 0

  phvwr       p.inner_ethernet_valid, 1
  phvwr       p.udp_valid, 1
  phvwr       p.genv_valid, 1

  seq         c1, d.u.encap_vxlan_d.ip_type, IP_HEADER_TYPE_IPV4
  cmov        r6, c1, ETHERTYPE_IPV4, ETHERTYPE_IPV6
  seq         c2, d.u.encap_vxlan_d.vlan_valid, 1
  bal.c2      r1, f_encap_vlan
  phvwr.!c2   p.ethernet_etherType, r6

  // update capri_p4_intrinsic_packet_len
  cmov        r1, c1, 50, 70
  add.c2      r1, r1, 4
  add         r1, r1, r5
  phvwr       p.capri_p4_intrinsic_packet_len, r1

  bcf         [c1],  f_insert_ipv4_header
  add         r6, r0, 0x4011
  b.!c1       f_insert_ipv6_header
  add         r6, r0, 0x1140

.align
encap_nvgre:
  phvwr       p.{inner_ethernet_dstAddr...inner_ethernet_etherType}, \
                  k.{ethernet_dstAddr...ethernet_etherType}
  phvwr       p.ethernet_dstAddr, d.u.encap_vxlan_d.mac_da
  phvwr       p.ethernet_srcAddr, d.u.encap_vxlan_d.mac_sa

  phvwri      p.{gre_C...gre_proto}, GRE_PROTO_NVGRE
  phvwr       p.nvgre_tni, k.rewrite_metadata_tunnel_vnid
  phvwr       p.nvgre_flow_id, k.rewrite_metadata_entropy_hash

  phvwr       p.inner_ethernet_valid, 1
  phvwr       p.gre_valid, 1
  phvwr       p.nvgre_valid, 1

  seq         c1, d.u.encap_vxlan_d.ip_type, IP_HEADER_TYPE_IPV4
  cmov        r6, c1, ETHERTYPE_IPV4, ETHERTYPE_IPV6
  seq         c2, d.u.encap_vxlan_d.vlan_valid, 1
  bal.c2      r1, f_encap_vlan
  phvwr.!c2   p.ethernet_etherType, r6

  or          r5, k.capri_p4_intrinsic_packet_len_sbit6_ebit13, \
                  k.capri_p4_intrinsic_packet_len_sbit0_ebit5, 8
  add         r7, r5, 8

  // update capri_p4_intrinsic_packet_len
  cmov        r1, c1, 42, 62
  add.c2      r1, r1, 4
  add         r1, r1, r5
  phvwr       p.capri_p4_intrinsic_packet_len, r1

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

  seq         c1, d.u.encap_vxlan_d.ip_type, IP_HEADER_TYPE_IPV4
  cmov        r6, c1, ETHERTYPE_IPV4, ETHERTYPE_IPV6
  seq         c2, d.u.encap_vxlan_d.vlan_valid, 1
  bal.c2      r1, f_encap_vlan
  phvwr.!c2   p.ethernet_etherType, r6

  or          r5, k.capri_p4_intrinsic_packet_len_sbit6_ebit13, \
                  k.capri_p4_intrinsic_packet_len_sbit0_ebit5, 8
  add         r7, r5, 4

  // update capri_p4_intrinsic_packet_len
  cmov        r1, c1, 38, 58
  add.c2      r1, r1, 4
  add         r1, r1, r5
  phvwr       p.capri_p4_intrinsic_packet_len, r1

  bcf         [c1],  f_insert_ipv4_header
  add         r6, r0, 0x402f
    b.!c1       f_insert_ipv6_header
  add         r6, r0, 0x2f40

.align
encap_ip:
  seq         c1, d.u.encap_vxlan_d.ip_type, IP_HEADER_TYPE_IPV4
  cmov        r6, c1, ETHERTYPE_IPV4, ETHERTYPE_IPV6
  seq         c2, d.u.encap_vxlan_d.vlan_valid, 1
  bal.c2      r1, f_encap_vlan
  phvwr.!c2   p.ethernet_etherType, r6

  or          r5, k.capri_p4_intrinsic_packet_len_sbit6_ebit13, \
                  k.capri_p4_intrinsic_packet_len_sbit0_ebit5, 8
  sub         r7, r5, 14

  // update capri_p4_intrinsic_packet_len
  cmov        r1, c1, 20, 40
  add.c2      r1, r1, 4
  add         r1, r1, r5
  phvwr       p.capri_p4_intrinsic_packet_len, r1

  bcf         [c1],  f_insert_ipv4_header
  add         r6, 0x4000, k.tunnel_metadata_inner_ip_proto, 8
  b.!c1       f_insert_ipv6_header
  add         r6, 0x0040, k.tunnel_metadata_inner_ip_proto, 8

.align
encap_mpls:
  seq         c1, d.u.encap_mpls_d.eompls, TRUE
  phvwr.c1    p.{inner_ethernet_dstAddr...inner_ethernet_etherType}, \
                  k.{ethernet_dstAddr...ethernet_etherType}
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
#endif /* PHASE2 */

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tunnel_rewrite_error:
  nop.e
  nop
