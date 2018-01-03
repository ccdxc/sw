#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"
#include "../../include/capri_common.h"

struct drop_stats_k k;
struct drop_stats_d d;
struct phv_         p;

%%

nop:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
drop_stats:
  seq         c1, d.u.drop_stats_d.mirror_en, 1
  phvwr.c1    p.capri_intrinsic_tm_span_session, d.u.drop_stats_d.mirror_session_id
  nop.e
  nop
#if 0
  seq         c1, d.u.drop_stats_d.mirror_en, 1
  add         r7, d.u.drop_stats_d.drop_pkts, 1
  bgti        r7, 0xF, drop_stats_overflow
  phvwr.c1    p.capri_intrinsic_tm_span_session, d.u.drop_stats_d.mirror_session_id
  tblwr.e     d.u.drop_stats_d.drop_pkts, r7[3:0]
  nop
#endif

drop_stats_overflow:
  add         r5, r5, d.u.drop_stats_d.stats_idx, 3
  addi        r6, r0, CAPRI_MEM_SEM_ATOMIC_ADD_START
  add         r6, r6, r5[26:0]
  or          r7, r7, r5[31:27], 58
  memwr.dx.e  r6, r7
  tblwr       d.u.drop_stats_d.drop_pkts, r0

/*
 * stats allocation in the atomic add region:
 * 8B drop packet count
 * total per index = 8B
 */
