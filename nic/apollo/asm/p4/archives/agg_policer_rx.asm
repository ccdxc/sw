#include "apollo.h"
#include "EGRESS_p.h"

struct phv_ p;

%%

execute_agg_policer:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
agg_policer_rx_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
