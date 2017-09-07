#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct nacl_k k;
struct nacl_d d;
struct phv_   p;

%%

nop:
  nop.e
  nop

nacl_permit:
  seq         c2, d.u.nacl_permit_d.force_flow_hit, 1
  phvwr.c2    p.control_metadata_flow_miss, 0
  phvwr.c2    p.control_metadata_flow_miss_ingress, 0
  seq         c2, d.u.nacl_permit_d.log_en, 1
  phvwr.c2    p.capri_intrinsic_tm_cpu, 1
  seq         c2, d.u.nacl_permit_d.qid_en, 1
  phvwr.c2    p.control_metadata_qid, d.u.nacl_permit_d.qid
  seq         c2, d.u.nacl_permit_d.ingress_mirror_en, 1
  phvwr.c2    p.capri_intrinsic_tm_span_session, d.u.nacl_permit_d.ingress_mirror_session_id
  seq         c2, d.u.nacl_permit_d.egress_mirror_en, 1
  phvwr.c2    p.control_metadata_egress_mirror_session_id, d.u.nacl_permit_d.egress_mirror_session_id
  seq         c2, d.u.nacl_permit_d.rewrite_en, 1
  phvwr.c2    p.rewrite_metadata_rewrite_index, d.u.nacl_permit_d.rewrite_index
  seq         c2, d.u.nacl_permit_d.tunnel_rewrite_en, 1
  phvwr.c2    p.rewrite_metadata_tunnel_rewrite_index, d.u.nacl_permit_d.tunnel_rewrite_index
  seq         c2, d.u.nacl_permit_d.dst_lport_en, 1
  phvwr.c2    p.control_metadata_dst_lport, d.u.nacl_permit_d.dst_lport
  seq         c2, d.u.nacl_permit_d.egress_policer_en, 1
  phvwr.e     p.copp_metadata_policer_index, d.u.nacl_permit_d.policer_index
  phvwr.c2    p.policer_metadata_egress_policer_index, d.u.nacl_permit_d.egress_policer_index

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
nacl_deny:
  phvwr.e     p.control_metadata_drop_reason[DROP_NACL], 1
  phvwr       p.capri_intrinsic_drop, 1
