#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_output_mapping_k.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nw.h"

struct output_mapping_k_ k;
struct output_mapping_d  d;
struct phv_              p;

%%

output_mapping_drop:
  phvwr       p.capri_intrinsic_lif, 0
  phvwr.e     p.capri_intrinsic_drop, TRUE
  phvwr       p.control_metadata_egress_drop_reason[EGRESS_DROP_OUTPUT_MAPPING], 1

.align
set_tm_oport_enforce_src_lport:
  sne         c1, d.u.set_tm_oport_enforce_src_lport_d.mnic_enforce_src_lport, r0
  sne.c1      c1, d.u.set_tm_oport_enforce_src_lport_d.mnic_enforce_src_lport, k.control_metadata_src_lport
  b.!c1       set_tm_oport
  phvwr.c1    p.capri_intrinsic_lif, d.u.set_tm_oport_enforce_src_lport_d.dst_lif
  phvwr.e     p.capri_intrinsic_drop, TRUE
  phvwr       p.control_metadata_egress_drop_reason[EGRESS_DROP_OUTPUT_MAPPING], 1

.align
set_tm_oport:
  seq         c1, d.u.set_tm_oport_d.nacl_egress_drop_en, TRUE
  seq         c2, k.control_metadata_nacl_egress_drop, TRUE
  bcf         [c1 & c2], output_mapping_drop
  add         r7, r0, r0
  seq         c1, d.u.set_tm_oport_d.nports, 0
  mod.!c1     r7, k.rewrite_metadata_entropy_hash, d.u.set_tm_oport_d.nports

  // mod instruction stalls; instructions below execute till r7 is ready
  add         r1, r0, r0
  seq         c1, k.control_metadata_span_copy, FALSE
  seq.c1      c2, d.u.set_tm_oport_d.egress_mirror_en, TRUE
  or.c2       r1, r0, k.control_metadata_egress_mirror_session_id
  seq.c1      c3, d.u.set_tm_oport_d.mirror_en, TRUE
  or.c3       r1, r1, d.u.set_tm_oport_d.mirror_session_id
  phvwr       p.capri_intrinsic_tm_span_session, r1

  phvwrpair   p.capri_intrinsic_lif, d.u.set_tm_oport_d.dst_lif, \
                p.capri_intrinsic_tm_oq, k.control_metadata_dest_tm_oq[4:0]
  phvwr       p.control_metadata_rdma_enabled, d.u.set_tm_oport_d.rdma_enabled
  phvwr       p.control_metadata_p4plus_app_id, d.u.set_tm_oport_d.p4plus_app_id

  sne         c1, d.u.set_tm_oport_d.access_vlan_id, 0
  seq.c1      c1, k.vlan_tag_vid, \
                d.u.set_tm_oport_d.access_vlan_id
  bcf         [!c1], set_tm_oport_common
  sub         r7, 28, r7, 2
  // access vlan processing
  sub         r1, k.capri_p4_intrinsic_packet_len, 4
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

output_mapping_drop1:
  phvwr       p.capri_intrinsic_lif, 0
  phvwr.e     p.capri_intrinsic_drop, TRUE
  phvwr       p.control_metadata_egress_drop_reason[EGRESS_DROP_OUTPUT_MAPPING], 1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
output_mapping_error:
  nop.e
  nop
