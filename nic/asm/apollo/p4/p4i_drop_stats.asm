#include "apollo.h"
#include "INGRESS_p.h"

struct phv_ p;

%%

p4i_drop_stats:
    nop.e
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
p4i_drop_stats_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
