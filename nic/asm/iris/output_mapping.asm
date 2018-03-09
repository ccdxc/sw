#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/iris/include/defines.h"
#include "nw.h"

struct output_mapping_k k;
struct output_mapping_d d;
struct phv_             p;

%%

output_mapping_drop:
  K_DBG_WR(0x100)
  DBG_WR(0x108, k.control_metadata_dst_lport)
  phvwr.e       p.capri_intrinsic_drop, TRUE
  nop

.align
set_tm_oport:
  K_DBG_WR(0x100)
  DBG_WR(0x10a, 0x10a)
  add         r7, r0, r0
  seq         c1, d.u.set_tm_oport_d.nports, 0
  mod.!c1     r7, k.rewrite_metadata_entropy_hash, d.u.set_tm_oport_d.nports

  // mod instruction stalls; instructions below execute till r7 is ready
  seq         c1, d.u.set_tm_oport_d.egress_mirror_en, TRUE
  seq.c1      c1, k.control_metadata_span_copy, FALSE 
  phvwr.c1    p.capri_intrinsic_tm_span_session, k.control_metadata_egress_mirror_session_id
  phvwrpair   p.capri_intrinsic_lif, d.u.set_tm_oport_d.dst_lif, \
                p.capri_intrinsic_tm_oq, k.control_metadata_dest_tm_oq[4:0]
  phvwr       p.control_metadata_rdma_enabled, d.u.set_tm_oport_d.rdma_enabled
  phvwr       p.control_metadata_p4plus_app_id, d.u.set_tm_oport_d.p4plus_app_id

  sne         c1, d.u.set_tm_oport_d.access_vlan_id, 0
  seq.c1      c1, k.{vlan_tag_vid_sbit0_ebit3,vlan_tag_vid_sbit4_ebit11}, \
                d.u.set_tm_oport_d.access_vlan_id
  bcf         [!c1], set_tm_oport_common
  sub         r7, 28, r7, 2
  // access vlan processing
  sub         r1, k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                     capri_p4_intrinsic_packet_len_sbit6_ebit13}, 4
  phvwr       p.vlan_tag_valid, FALSE
  phvwr       p.ethernet_etherType, k.vlan_tag_etherType
  phvwr       p.capri_p4_intrinsic_packet_len, r1

set_tm_oport_common:
  srlv        r6, d.{u.set_tm_oport_d.egress_port1...u.set_tm_oport_d.egress_port8}, r7
  phvwr       p.capri_intrinsic_tm_oport, r6
  DBG_WR(0x10b, r6)
  phvwr       p.control_metadata_vlan_strip, d.u.set_tm_oport_d.vlan_strip
  seq.e       c1, d.u.set_tm_oport_d.encap_vlan_id_valid, TRUE
  phvwr.c1    p.rewrite_metadata_tunnel_vnid, d.u.set_tm_oport_d.encap_vlan_id

.align
redirect_to_cpu:
  seq         c1, k.control_metadata_cpu_copy, TRUE
  phvwr.c1    p.capri_intrinsic_tm_oq, d.u.redirect_to_cpu_d.cpu_copy_tm_oq
  phvwr.!c1   p.capri_intrinsic_tm_oq, d.u.redirect_to_cpu_d.control_tm_oq
  phvwr       p.control_metadata_to_cpu, TRUE
  seq         c1, d.u.redirect_to_cpu_d.egress_mirror_en, TRUE
  phvwr.c1    p.capri_intrinsic_tm_span_session, k.control_metadata_egress_mirror_session_id
  phvwrpair.e p.capri_intrinsic_tm_oport, TM_PORT_DMA, \
              p.capri_intrinsic_lif, d.u.redirect_to_cpu_d.dst_lif
  phvwr       p.control_metadata_p4plus_app_id, P4PLUS_APPTYPE_CPU

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
redirect_to_remote:
  seq         c1, d.u.redirect_to_remote_d.egress_mirror_en, TRUE
  phvwr.c1    p.capri_intrinsic_tm_span_session, k.control_metadata_egress_mirror_session_id
  phvwr       p.capri_intrinsic_tm_oport, d.u.redirect_to_remote_d.tm_oport
  phvwr.e     p.capri_intrinsic_tm_oq, d.u.redirect_to_remote_d.tm_oq
  phvwr       p.rewrite_metadata_tunnel_rewrite_index, d.u.redirect_to_remote_d.tunnel_index

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
output_mapping_error:
  nop.e
  nop
