#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct output_mapping_k k;
struct output_mapping_d d;
struct phv_             p;

%%

nop:
  nop.e
  nop

.align
set_tm_oport:
  add         r7, r0, r0
  seq         c1, d.u.set_tm_oport_d.nports, 0
  mod.!c1     r7, k.rewrite_metadata_entropy_hash, d.u.set_tm_oport_d.nports

  // mod instruction stalls; instructions below execute till r7 is ready
  seq         c1, d.u.set_tm_oport_d.egress_mirror_en, TRUE
  sne         c2, k.capri_intrinsic_tm_instance_type, TM_INSTANCE_TYPE_SPAN
  andcf       c1, [c2]
  phvwr.c1    p.capri_intrinsic_tm_span_session, k.control_metadata_egress_mirror_session_id
  phvwr       p.capri_intrinsic_tm_oq, k.control_metadata_egress_tm_oqueue
  phvwr       p.capri_intrinsic_lif, d.u.set_tm_oport_d.dst_lif
  phvwr       p.control_metadata_rdma_enabled, d.u.set_tm_oport_d.rdma_enabled
  phvwr       p.control_metadata_p4plus_app_id, d.u.set_tm_oport_d.p4plus_app_id

  sub         r7, 28, r7, 2
  srlv        r6, d.{u.set_tm_oport_d.egress_port1...u.set_tm_oport_d.egress_port8}, r7
  phvwr       p.capri_intrinsic_tm_oport, r6
  phvwr       p.control_metadata_vlan_strip, d.u.set_tm_oport_d.vlan_strip
  seq.e       c1, d.u.set_tm_oport_d.encap_vlan_id_valid, TRUE
  phvwr.c1    p.rewrite_metadata_tunnel_vnid, d.u.set_tm_oport_d.encap_vlan_id

.align
redirect_to_cpu:
  phvwr       p.capri_intrinsic_lif, d.u.redirect_to_cpu_d.dst_lif
  phvwr       p.control_metadata_cpu_copy, TRUE
  seq         c1, d.u.redirect_to_cpu_d.egress_mirror_en, TRUE
  phvwr.c1    p.capri_intrinsic_tm_span_session, k.control_metadata_egress_mirror_session_id
  phvwr       p.capri_intrinsic_tm_oport, TM_PORT_DMA
  phvwr.e     p.capri_intrinsic_tm_oq, d.u.redirect_to_cpu_d.tm_oqueue
  phvwr       p.control_metadata_p4plus_app_id, P4PLUS_APPTYPE_CPU

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
redirect_to_remote:
  seq         c1, d.u.redirect_to_remote_d.egress_mirror_en, TRUE
  phvwr.c1    p.capri_intrinsic_tm_span_session, k.control_metadata_egress_mirror_session_id
  phvwr       p.capri_intrinsic_tm_oport, d.u.redirect_to_remote_d.tm_oport
  phvwr.e     p.capri_intrinsic_tm_oq, d.u.redirect_to_remote_d.tm_oqueue
  phvwr       p.rewrite_metadata_tunnel_rewrite_index, d.u.redirect_to_remote_d.tunnel_index
