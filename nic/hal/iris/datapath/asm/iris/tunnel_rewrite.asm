#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_tunnel_rewrite_k.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"

struct tunnel_rewrite_k_ k;
struct tunnel_rewrite_d  d;
struct phv_              p;

%%

nop:
  nop.e
  nop

.align
encap_vxlan:
  // r5 : k.capri_p4_intrinsic_packet_len
  or          r5, r0, k.capri_p4_intrinsic_packet_len
  add         r7, r5, 16

  // ethernet headers
  phvwrpair   p.inner_ethernet_dstAddr, k.ethernet_dstAddr, \
                p.{inner_ethernet_srcAddr,inner_ethernet_etherType}, \
                k.{ethernet_srcAddr,ethernet_etherType}
  phvwrpair   p.ethernet_dstAddr, d.u.encap_vxlan_d.mac_da, \
                p.ethernet_srcAddr, d.u.encap_vxlan_d.mac_sa

  // vxlan header (flags, vni)
  add         r1, r0, 0x08, 56
  or          r1, r1, k.rewrite_metadata_tunnel_vnid, 8

  // udp header (srcPort, dstPort, len, checksum)
  or          r2, r0, k.rewrite_metadata_entropy_hash, 48
  or          r2, r2, 0xC000, 48
  or          r2, r2, UDP_PORT_VXLAN, 32
  or          r2, r2, r7, 16
  phvwr       p.{vxlan_flags...vxlan_reserved2}, r1
  phvwr       p.{udp_srcPort...udp_checksum}, r2

  // set inner_ethernet_valid, vxlan_valid and udp_valid
  // 10 0001 0001
  phvwri      p.{inner_ethernet_valid, \
                 erspan_t3_opt_valid, \
                 erspan_t3_valid, \
                 nvgre_valid, \
                 gre_valid, \
                 vxlan_valid, \
                 genv_valid, \
                 vxlan_gpe_valid, \
                 mpls_0_valid, \
                 udp_valid}, 0x211

  // vlan header
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
  seq         c1, k.vlan_tag_valid, FALSE
  add         r1, k.capri_p4_intrinsic_packet_len, 4
  phvwr.c1    p.vlan_tag_valid, 1
  phvwr.c1    p.vlan_tag_etherType, k.ethernet_etherType
  phvwr.c1    p.ethernet_etherType, ETHERTYPE_VLAN
  phvwr.c1    p.capri_p4_intrinsic_packet_len, r1
  phvwr       p.{vlan_tag_dei, vlan_tag_vid}, k.rewrite_metadata_tunnel_vnid[11:0]
  seq.e       c7, k.qos_metadata_cos_en, 1
  phvwr.c7    p.vlan_tag_pcp, k.qos_metadata_cos

.align
encap_mpls_udp:
  or          r5, r0, k.capri_p4_intrinsic_packet_len
  add         r7, r5, -2
  // ethernet header
  seq         c1, d.u.encap_mpls_udp_d.mac_sa, r0
  phvwr.!c1   p.ethernet_srcAddr, d.u.encap_mpls_udp_d.mac_sa
  phvwr       p.ethernet_dstAddr, d.u.encap_mpls_udp_d.mac_da
  // udp header
  or          r1, r0, k.rewrite_metadata_entropy_hash, 48
  or          r1, r1, 0xC000, 48
  or          r1, r1, UDP_PORT_MPLS, 32
  or          r1, r1, r7, 16
  phvwr       p.{udp_srcPort...udp_checksum}, r1
  // mpls header
  or          r1, 0x140, k.rewrite_metadata_tunnel_vnid, 12
  phvwr       p.{mpls_0_label,mpls_0_exp,mpls_0_bos,mpls_0_ttl}, r1
  // set header valid bits
  phvwrmi     p.{mpls_0_valid, udp_valid}, 0x3, 0x3
  // vlan header
  seq         c1, d.u.encap_vxlan_d.ip_type, IP_HEADER_TYPE_IPV4
  cmov        r6, c1, ETHERTYPE_IPV4, ETHERTYPE_IPV6
  seq         c2, d.u.encap_vxlan_d.vlan_valid, 1
  bal.c2      r1, f_encap_vlan
  phvwr.!c2   p.ethernet_etherType, r6
  // update capri_p4_intrinsic_packet_len
  add         r1, r5, 32
  add.c2      r1, r1, 4
  phvwr       p.capri_p4_intrinsic_packet_len, r1
  // ip header
  sne         c1, k.rewrite_metadata_tunnel_ip, r0
  tblwr.c1.l  d.u.encap_mpls_udp_d.ip_da, k.rewrite_metadata_tunnel_ip
  b           f_insert_ipv4_header
  add         r6, r0, 0x4011

.align
encap_erspan:
  phvwrpair   p.inner_ethernet_dstAddr, k.ethernet_dstAddr, \
                p.{inner_ethernet_srcAddr,inner_ethernet_etherType}, \
                k.{ethernet_srcAddr,ethernet_etherType}
  phvwrpair   p.ethernet_dstAddr, d.u.encap_vxlan_d.mac_da, \
                  p.ethernet_srcAddr, d.u.encap_vxlan_d.mac_sa

  phvwr       p.{gre_C...gre_ver}, r0
  phvwr       p.gre_proto, GRE_PROTO_ERSPAN_T3
  phvwrpair   p.erspan_t3_version, 0x2, p.erspan_t3_bso, 0
  seq         c1, k.capri_intrinsic_tm_iport, TM_PORT_EGRESS
  phvwr.c1    p.erspan_t3_direction, 1
  phvwr       p.erspan_t3_granularity, 0x3
  phvwrpair   p.{erspan_t3_sgt...erspan_t3_hw_id}, 0, \
                  p.{erspan_t3_granularity,erspan_t3_options}, 0x6

  sne         c3, k.control_metadata_current_time_in_ns, r0
#ifndef CAPRI_IGNORE_TIMESTAMP
  bcf         [!c3], encap_erspan_timestamp_done
  phvwr.c3    p.erspan_t3_options, 1
  phvwr       p.erspan_t3_timestamp, k.control_metadata_current_time_in_ns[31:0]
  phvwr       p.erspan_t3_opt_valid, 1
  or          r1, k.control_metadata_current_time_in_ns[63:32], \
                k.capri_intrinsic_lif, 32
  or          r1, r1, 0x3, 58
  phvwr       p.{erspan_t3_opt_platf_id...erspan_t3_opt_timestamp}, r1
#endif

encap_erspan_timestamp_done:
  phvwr       p.inner_ethernet_valid, 1
  phvwr       p.gre_valid, 1
  phvwr       p.erspan_t3_valid, 1

  seq         c1, d.u.encap_vxlan_d.ip_type, IP_HEADER_TYPE_IPV4
  cmov        r6, c1, ETHERTYPE_IPV4, ETHERTYPE_IPV6
  seq         c2, d.u.encap_vxlan_d.vlan_valid, 1
  bal.c2      r1, f_encap_vlan
  phvwr.!c2   p.ethernet_etherType, r6

  cmov        r5, c3, 8, 0
  add         r5, r5, k.capri_p4_intrinsic_packet_len
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

  or          r2, 0xC000, k.rewrite_metadata_entropy_hash
  phvwr       p.udp_srcPort, r2
  phvwr       p.udp_dstPort, UDP_PORT_VXLAN_GPE
  phvwr       p.udp_checksum, 0
  or          r5, r0, k.capri_p4_intrinsic_packet_len
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

  or          r2, 0xC000, k.rewrite_metadata_entropy_hash
  phvwr       p.udp_srcPort, r2
  phvwr       p.udp_dstPort, UDP_PORT_GENV
  phvwr       p.udp_checksum, 0
  or          r5, r0, k.capri_p4_intrinsic_packet_len
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

  or          r5, r0, k.capri_p4_intrinsic_packet_len
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

  or          r5, r0, k.capri_p4_intrinsic_packet_len
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

  or          r5, r0, k.capri_p4_intrinsic_packet_len
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
