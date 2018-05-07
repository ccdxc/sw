#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/iris/include/defines.h"
#include "../../include/capri_common.h"
#include "nw.h"

struct drop_stats_k k;
struct drop_stats_d d;
struct phv_         p;

%%

drop_stats:
  K_DBG_WR(0xa0)
  DBG_WR(0xa8, k.control_metadata_drop_reason)
  seq         c1, k.control_metadata_mirror_on_drop_en, TRUE
  seq         c2, d.drop_stats_d.mirror_en, TRUE
  setcf       c2, [!c1 & c2]
  phvwr.c1    p.capri_intrinsic_tm_span_session, \
                k.control_metadata_mirror_on_drop_session_id
  phvwr.c2    p.capri_intrinsic_tm_span_session, \
                d.drop_stats_d.mirror_session_id
  phvwr.e     p.capri_intrinsic_tm_oport, TM_PORT_EGRESS
  nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
drop_stats_error:
  nop.e
  nop
