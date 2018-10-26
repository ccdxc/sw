#include "apollo.h"
#include "EGRESS_p.h"

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
