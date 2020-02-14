#include "ingress.h"
#include "INGRESS_p.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nw.h"

struct nacl_k k;
struct nacl_d d;
struct phv_   p;

%%

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
nop:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
nacl_permit:
  seq           c2, d.u.nacl_permit_d.discard_drop, 1
  phvwr.c2      p.capri_intrinsic_drop, 0
  phvwr.c2      p.control_metadata_drop_reason, 0

  seq           c2, d.u.nacl_permit_d.force_flow_hit, 1
  phvwr.c2      p.control_metadata_flow_miss_ingress, 0
  phvwr.c2      p.control_metadata_i2e_flags[P4_I2E_FLAGS_FLOW_MISS], 0

  seq           c2, d.u.nacl_permit_d.log_en, 1
  phvwr.c2      p.capri_intrinsic_tm_cpu, 1

  seq           c2, d.u.nacl_permit_d.qid_en, 1
  phvwr.c2      p.control_metadata_qid, d.u.nacl_permit_d.qid

  seq           c2, d.u.nacl_permit_d.ingress_mirror_en, 1
  phvwr.c2      p.capri_intrinsic_tm_span_session, \
                    d.u.nacl_permit_d.ingress_mirror_session_id

  seq           c2, d.u.nacl_permit_d.egress_mirror_en, 1
  phvwr.c2      p.control_metadata_egress_mirror_session_id, \
                    d.u.nacl_permit_d.egress_mirror_session_id

  seq           c2, d.u.nacl_permit_d.rewrite_en, 1
  phvwr.c2      p.rewrite_metadata_rewrite_index[11:0], \
                    d.u.nacl_permit_d.rewrite_index
  phvwr.c2      p.rewrite_metadata_flags, d.u.nacl_permit_d.rewrite_flags

  seq           c2, d.u.nacl_permit_d.tunnel_rewrite_en, 1
  phvwr.c2      p.rewrite_metadata_tunnel_rewrite_index[9:0], \
                    d.u.nacl_permit_d.tunnel_rewrite_index
  phvwr.c2      p.rewrite_metadata_tunnel_vnid, \
                    d.u.nacl_permit_d.tunnel_vnid
  phvwr.c2      p.tunnel_metadata_tunnel_originate[0], \
                    d.u.nacl_permit_d.tunnel_originate

  seq           c2, d.u.nacl_permit_d.dst_lport_en, 1
  phvwr.c2      p.control_metadata_dst_lport, d.u.nacl_permit_d.dst_lport
  phvwr.c2      p.capri_intrinsic_tm_cpu, FALSE
  phvwr         p.control_metadata_nacl_stats_idx, d.u.nacl_permit_d.stats_idx
  phvwr         p.control_metadata_nacl_egress_drop, d.u.nacl_permit_d.egress_drop
  phvwr.e       p.copp_metadata_policer_index, d.u.nacl_permit_d.policer_index
  phvwrpair.c2  p.capri_intrinsic_tm_replicate_ptr, 0, \
                p.capri_intrinsic_tm_replicate_en, FALSE

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
nacl_deny:
  seq           c1, d.u.nacl_deny_d.drop_reason_valid, 1
  sle.c1        c1, d.u.nacl_deny_d.drop_reason, DROP_MAX
  phvwr.!c1     p.control_metadata_drop_reason[DROP_NACL], 1
  add           r1, offsetof(p, control_metadata_drop_reason), \
                    d.u.nacl_deny_d.drop_reason
  phvwrp.c1     r1, 0, 1, 1
  phvwr.e       p.control_metadata_nacl_stats_idx, d.u.nacl_deny_d.stats_idx
  phvwr.f       p.capri_intrinsic_drop, 1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
nacl_error:
  nop.e
  nop
