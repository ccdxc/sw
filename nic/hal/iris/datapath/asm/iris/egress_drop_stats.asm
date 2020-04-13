#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_egress_drop_stats_k.h"
#include "asic/cmn/asic_common.hpp"
#include "nw.h"

struct egress_drop_stats_k_ k;
struct egress_drop_stats_d  d;
struct phv_                 p;

%%

egress_drop_stats:
    seq         c1, k.control_metadata_mirror_on_drop_en, TRUE
    seq         c2, d.egress_drop_stats_d.mirror_en, TRUE
    setcf       c2, [!c1 & c2]
    phvwr.c1    p.capri_intrinsic_tm_span_session, \
                    d.egress_drop_stats_d.mirror_session_id
    phvwr.e     p.capri_intrinsic_tm_oport, TM_PORT_DMA
    phvwr.c2    p.capri_intrinsic_tm_span_session, \
                    d.egress_drop_stats_d.mirror_session_id

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
egress_drop_stats_error:
    phvwr       p.capri_intrinsic_drop, TRUE
    addi        r6, r0, ASIC_MEM_SEM_ATOMIC_ADD_START
    add         r5, r5, k.capri_intrinsic_error_bits, 3
    add         r6, r6, r5[26:0]
    or.e        r7,  1, r5[31:27], 58
    memwr.dx    r6, r7
