#include "artemis.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_meter_stats_k.h"

struct meter_stats_k_   k;
struct meter_stats_d    d;
struct phv_ p;

%%

meter_stats:
    tbladd.f.e      d.{meter_stats_d.pkt_bytes}.dx, k.rewrite_metadata_meter_len
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
meter_stats_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
