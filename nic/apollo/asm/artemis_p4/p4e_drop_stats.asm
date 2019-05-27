#include "artemis.h"
#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_p4e_drop_stats_k.h"

struct p4e_drop_stats_k_ k;
struct p4e_drop_stats_d  d;
struct phv_ p;

%%

p4e_drop_stats:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
p4e_drop_stats_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
