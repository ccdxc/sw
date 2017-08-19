#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct output_mapping_k k;
struct output_mapping_d d;
struct phv_             p;

%%

set_tm_oport:
  seq         c1, d.u.set_tm_oport_d.egress_mirror_en, TRUE
  phvwr.c1    p.capri_intrinsic_tm_span_session, k.control_metadata_egress_mirror_session_id
  add         r7, r0, r0
  seq         c1, d.u.set_tm_oport_d.nports, 0
  mod.!c1     r7, k.rewrite_metadata_entropy_hash, d.u.set_tm_oport_d.nports
  sub         r7, 28, r7, 4
  srlv        r6, d.{u.set_tm_oport_d.egress_port1...u.set_tm_oport_d.egress_port8}, r7
  phvwr       p.capri_intrinsic_tm_oport, r6
  phvwr       p.capri_intrinsic_tm_oq, k.control_metadata_egress_tm_oqueue
  phvwr       p.control_metadata_rdma_enabled, d.u.set_tm_oport_d.rdma_enabled
  phvwr       p.control_metadata_p4plus_app_id, d.u.set_tm_oport_d.p4plus_app_id
  seq         c1, d.u.set_tm_oport_d.vlan_tag_in_skb, TRUE
  nop.!c1.e
  seq         c1, r6[2:0], TM_PORT_DMA
  seq         c2, k.vlan_tag_valid, TRUE
  andcf       c2, [c1]
  phvwr.c2    p.p4_to_p4plus_classic_nic_vlan_pcp, k.vlan_tag_pcp
  phvwr.c2    p.p4_to_p4plus_classic_nic_vlan_dei, k.vlan_tag_dei
  phvwr.c2    p.p4_to_p4plus_classic_nic_vlan_vid, \
                  k.{vlan_tag_vid_sbit0_ebit3, vlan_tag_vid_sbit4_ebit11}
  or          r1, k.p4_to_p4plus_classic_nic_flags, CLASSIC_NIC_FLAGS_VLAN_VALID
  phvwr.c2    p.p4_to_p4plus_classic_nic_flags, r1
  phvwr.c2.e  p.ethernet_etherType, k.vlan_tag_etherType
  phvwr.c2    p.vlan_tag_valid, FALSE
  phvwr.!c1   p.vlan_tag_valid, TRUE
  phvwr.!c1.e p.vlan_tag_etherType, k.ethernet_etherType
  phvwr.!c1   p.ethernet_etherType, 0x8100

.align
redirect_to_cpu:
  seq         c1, d.u.redirect_to_cpu_d.egress_mirror_en, TRUE
  phvwr.c1    p.capri_intrinsic_tm_span_session, k.control_metadata_egress_mirror_session_id
  seq         c1, k.capri_intrinsic_tm_instance_type, TM_INSTANCE_TYPE_CPU
  seq         c2, k.capri_intrinsic_tm_instance_type, TM_INSTANCE_TYPE_NORMAL
  andcf       c1, [c2]
  phvwr       p.control_metadata_cpu_copy, TRUE
  phvwr       p.capri_intrinsic_tm_oport, TM_PORT_DMA
  phvwr.e     p.capri_intrinsic_tm_oq, d.u.redirect_to_cpu_d.tm_oqueue
  phvwr       p.rewrite_metadata_tunnel_rewrite_index, d.u.redirect_to_cpu_d.tunnel_index

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
redirect_to_remote:
  seq         c1, d.u.redirect_to_remote_d.egress_mirror_en, TRUE
  phvwr.c1    p.capri_intrinsic_tm_span_session, k.control_metadata_egress_mirror_session_id
  phvwr       p.capri_intrinsic_tm_oport, d.u.redirect_to_remote_d.tm_oport
  phvwr.e     p.capri_intrinsic_tm_oq, d.u.redirect_to_remote_d.tm_oqueue
  phvwr       p.rewrite_metadata_tunnel_rewrite_index, d.u.redirect_to_remote_d.tunnel_index

.align
nop:
  nop.e
  nop
