#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/iris/include/defines.h"
#include "../../include/capri_common.h"
#include "nw.h"

struct egress_drop_stats_k k;
struct egress_drop_stats_d d;
struct phv_         p;

%%

egress_drop_stats:
  K_DBG_WR(0x160)
  DBG_WR(0x168, k.control_metadata_egress_drop_reason)
  seq         c1, k.control_metadata_mirror_on_drop_en, TRUE
  seq         c2, d.egress_drop_stats_d.mirror_en, TRUE
  setcf       c2, [!c1 & c2]
  phvwr.c1    p.capri_intrinsic_tm_span_session, \
                d.egress_drop_stats_d.mirror_session_id
  phvwr.c2    p.capri_intrinsic_tm_span_session, \
                d.egress_drop_stats_d.mirror_session_id
  phvwr.e     p.capri_intrinsic_tm_oport, TM_PORT_DMA
  nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
egress_drop_stats_error:
  nop.e
  nop
